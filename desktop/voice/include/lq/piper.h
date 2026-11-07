#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

// A Piper neural voice (VITS exported to ONNX) run on the CPU with onnxruntime: text in, 16-bit mono PCM out.
// The text goes through lq::phonemizeSentences, so pass it through lq::speechText first.
namespace lq {

class PiperVoice
{
public:
    // model is the .onnx file; its settings are in the .onnx.json beside it. Null, with the reason in *error,
    // when either cannot be read.
    static std::unique_ptr<PiperVoice> load(const std::filesystem::path &model, std::string *error = nullptr);
    ~PiperVoice();
    PiperVoice(const PiperVoice &) = delete;
    PiperVoice &operator=(const PiperVoice &) = delete;

    int sampleRate() const;
    // lengthScale above 1 speaks slower. Each sentence is brought to full volume, with 0.2 s between
    // sentences, as Piper does. Empty when nothing in the text can be spoken or inference fails.
    std::vector<std::int16_t> synthesize(std::string_view text, float lengthScale = 1.0f) const;

    // Piper's input for one sentence: ^ _ p₁ _ p₂ … pₙ _ $. Phonemes the voice does not know are left out.
    std::vector<std::int64_t> phonemeIds(std::u32string_view phonemes) const;
    bool knows(char32_t phoneme) const;

private:
    PiperVoice();
    struct Impl;
    std::unique_ptr<Impl> d;
};

} // namespace lq
