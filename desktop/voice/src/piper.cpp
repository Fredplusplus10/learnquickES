#include "lq/piper.h"

#include "lq/speech.h"
#include "ort.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <map>
#include <thread>

namespace lq {
namespace {

// The single code point a phoneme_id_map key holds; 0 when the key is not exactly one code point.
char32_t singleCodePoint(const std::string &key)
{
    const auto byte = [&](std::size_t i) { return static_cast<unsigned char>(key[i]); };
    if (key.size() == 1 && byte(0) < 0x80)
        return byte(0);
    if (key.size() == 2 && (byte(0) >> 5) == 0x6)
        return static_cast<char32_t>(((byte(0) & 0x1Fu) << 6) | (byte(1) & 0x3Fu));
    if (key.size() == 3 && (byte(0) >> 4) == 0xE)
        return static_cast<char32_t>(((byte(0) & 0x0Fu) << 12) | ((byte(1) & 0x3Fu) << 6) | (byte(2) & 0x3Fu));
    if (key.size() == 4 && (byte(0) >> 3) == 0x1E)
        return static_cast<char32_t>(((byte(0) & 0x07u) << 18) | ((byte(1) & 0x3Fu) << 12) | ((byte(2) & 0x3Fu) << 6)
                                     | (byte(3) & 0x3Fu));
    return 0;
}

using ort::fail;
using ort::utf8;

constexpr float kSentenceSilence = 0.2f;  // seconds, Piper's default

using Pointer = nlohmann::json::json_pointer;

} // namespace

struct PiperVoice::Impl {
    Ort::Env env{ORT_LOGGING_LEVEL_WARNING, "learnquick"};
    std::unique_ptr<Ort::Session> session;
    std::map<char32_t, std::vector<std::int64_t>> ids;
    int sampleRate = 22050;
    float noiseScale = 0.667f;
    float noiseW = 0.8f;
    int speakers = 1;
};

PiperVoice::PiperVoice()
    : d(std::make_unique<Impl>())
{
}

PiperVoice::~PiperVoice() = default;

std::unique_ptr<PiperVoice> PiperVoice::load(const std::filesystem::path &model, std::string *error)
{
    if (!ort::ready(error))
        return nullptr;
    std::filesystem::path configPath = model;
    configPath += ".json";
    std::ifstream in(configPath, std::ios::binary);
    if (!in) {
        fail(error, "cannot read " + utf8(configPath));
        return nullptr;
    }
    const nlohmann::json config = nlohmann::json::parse(in, nullptr, false);
    if (config.is_discarded() || !config.contains("phoneme_id_map")) {
        fail(error, utf8(configPath) + " is not a Piper voice configuration");
        return nullptr;
    }

    std::unique_ptr<PiperVoice> voice(new PiperVoice);
    Impl &v = *voice->d;
    for (const auto &[key, value] : config.at("phoneme_id_map").items()) {
        const char32_t phoneme = singleCodePoint(key);
        if (phoneme != 0 && value.is_array())
            v.ids[phoneme] = value.get<std::vector<std::int64_t>>();
    }
    for (const char32_t required : {U'^', U'_', U'$'}) {
        if (!v.ids.contains(required)) {
            fail(error, utf8(configPath) + " lacks the ^ _ $ phonemes");
            return nullptr;
        }
    }
    v.sampleRate = config.value(Pointer("/audio/sample_rate"), 22050);
    v.noiseScale = config.value(Pointer("/inference/noise_scale"), 0.667f);
    v.noiseW = config.value(Pointer("/inference/noise_w"), 0.8f);
    v.speakers = config.value("num_speakers", 1);

    try {
        Ort::SessionOptions options;
        // A short phrase takes a fraction of a second; more threads than that only compete with the interface.
        const unsigned threads = std::clamp(std::thread::hardware_concurrency() / 2, 1u, 4u);
        options.SetIntraOpNumThreads(static_cast<int>(threads));
        options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        v.session = std::make_unique<Ort::Session>(v.env, model.c_str(), options);
    } catch (const Ort::Exception &e) {
        fail(error, "cannot load " + utf8(model) + ": " + e.what());
        return nullptr;
    }
    return voice;
}

int PiperVoice::sampleRate() const
{
    return d->sampleRate;
}

bool PiperVoice::knows(char32_t phoneme) const
{
    return d->ids.contains(phoneme);
}

std::vector<std::int64_t> PiperVoice::phonemeIds(std::u32string_view phonemes) const
{
    std::vector<std::int64_t> out;
    const auto append = [&](char32_t phoneme) {
        const auto it = d->ids.find(phoneme);
        if (it == d->ids.end())
            return false;
        out.insert(out.end(), it->second.begin(), it->second.end());
        return true;
    };
    append(U'^');
    append(U'_');
    for (const char32_t phoneme : phonemes) {
        if (append(phoneme))
            append(U'_');
    }
    append(U'$');
    return out;
}

std::vector<std::int16_t> PiperVoice::synthesize(std::string_view text, float lengthScale) const
{
    std::vector<std::int16_t> pcm;
    const auto silence = static_cast<std::size_t>(static_cast<float>(d->sampleRate) * kSentenceSilence);
    try {
        Ort::MemoryInfo memory = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        for (const std::u32string &sentence : phonemizeSentences(text)) {
            std::vector<std::int64_t> ids = phonemeIds(sentence);
            if (ids.size() <= 3)  // nothing but ^ _ $
                continue;
            std::array<std::int64_t, 2> idsShape{1, static_cast<std::int64_t>(ids.size())};
            std::int64_t length = static_cast<std::int64_t>(ids.size());
            std::array<std::int64_t, 1> oneShape{1};
            std::array<float, 3> scales{d->noiseScale, lengthScale, d->noiseW};
            std::array<std::int64_t, 1> scalesShape{3};
            std::int64_t speaker = 0;

            std::vector<Ort::Value> inputs;
            inputs.push_back(Ort::Value::CreateTensor<std::int64_t>(memory, ids.data(), ids.size(), idsShape.data(),
                                                                    idsShape.size()));
            inputs.push_back(Ort::Value::CreateTensor<std::int64_t>(memory, &length, 1, oneShape.data(), 1));
            inputs.push_back(Ort::Value::CreateTensor<float>(memory, scales.data(), scales.size(), scalesShape.data(),
                                                             scalesShape.size()));
            std::vector<const char *> names{"input", "input_lengths", "scales"};
            if (d->speakers > 1) {
                inputs.push_back(Ort::Value::CreateTensor<std::int64_t>(memory, &speaker, 1, oneShape.data(), 1));
                names.push_back("sid");
            }
            const char *outputName = "output";
            std::vector<Ort::Value> outputs = d->session->Run(Ort::RunOptions{nullptr}, names.data(), inputs.data(),
                                                              inputs.size(), &outputName, 1);
            const float *audio = outputs.front().GetTensorData<float>();
            const std::size_t count = outputs.front().GetTensorTypeAndShapeInfo().GetElementCount();

            // Full volume for every sentence, as Piper does.
            float peak = 0.01f;
            for (std::size_t i = 0; i < count; ++i)
                peak = std::max(peak, std::fabs(audio[i]));
            const float scale = 32767.0f / peak;
            if (!pcm.empty())
                pcm.insert(pcm.end(), silence, 0);
            pcm.reserve(pcm.size() + count);
            for (std::size_t i = 0; i < count; ++i)
                pcm.push_back(static_cast<std::int16_t>(std::clamp(audio[i] * scale, -32768.0f, 32767.0f)));
        }
    } catch (const Ort::Exception &) {
        return {};
    }
    return pcm;
}

} // namespace lq
