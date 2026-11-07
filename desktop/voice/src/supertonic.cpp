#include "lq/supertonic.h"

#include "ort.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <map>
#include <random>
#include <thread>
#include <type_traits>
#include <utility>

// The inference follows Supertone's own Python package (supertonic 1.3.1, core.py and pipeline.py): the
// duration predictor sets the length, the text encoder embeds the text, the vector estimator turns noise
// into speech latents in a fixed number of steps, and the vocoder makes the waveform. The noise is drawn as
// numpy draws it, so the same text and seed give the same samples here and there.
namespace lq {
namespace {

using ort::fail;
using ort::utf8;

// Supertone's default is 1.05. At 1.0 Whisper missed as few words or fewer on 36 course lines; asking for a
// slower pace than that only lengthens the pauses and blurs the words, so the app stretches the latents instead.
// A quicker one (a stretch below 1, for a voice slower than the pace asked) is asked of the model.
constexpr float kSpeed = 1.0f;
constexpr int kSteps = 8;  // Supertone's default; fewer is faster and slurs more
// How far the latents are stretched; any slower pace stretches the sound for the rest. On 36 course lines at
// 1.6 times (UTMOS naturalness, 1 to 5): latents to 1.25 and the sound 1.28 times gave F3 3.64 and M4 3.49;
// the latents alone, 3.51 and 3.35; the sound alone, 3.50 and 3.50. Up to 1.25 the latents alone are best
// (at 1.2, F3 3.95 and M4 3.78 against 3.87 and 3.73 for the sound alone).
constexpr float kMaxLatentStretch = 1.25f;
constexpr std::size_t kMaxPart = 300;  // characters per part, as Supertone's pipeline
// The noise seed. One fixed seed makes a phrase sound the same every time it is played, and the seed changes how
// clearly it comes out. Whisper transcribed 120 course words made with seeds 0 to 7 in F3 and M4: this one got
// the most right (220 of 240; the worst seed, 197), and it was every voice's until 0.4.5.
constexpr std::uint32_t kSeed = 1;

// The voices whose words came out clearer with another seed: seeds 0 to 3 on 40 course lines (Whisper's missed
// words, out of 167), then checked on 36 other lines. F1: 8 with seed 3, 15 with seed 1; M4: 9 with seed 0, 16;
// M5: 13 with seed 3, 21. F3, M1 and M2 missed the fewest with seed 1.
std::uint32_t seedFor(std::string_view style)
{
    static constexpr std::pair<std::string_view, std::uint32_t> seeds[] = {{"F1", 3}, {"M4", 0}, {"M5", 3}};
    for (const auto &[name, seed] : seeds) {
        if (name == style)
            return seed;
    }
    return kSeed;
}

// Every phrase, in every voice, is brought to the same loudness: its speech (the 10 ms frames above 2 % of its
// loudest) to -18 dBFS, with no peak above 1 dB below full scale. Bringing each phrase's peak to the same level
// instead left F3 about 5.5 dB louder than M2 on 36 course lines, and phrases of one voice 5 to 7 dB apart.
constexpr float kLoudness = 0.126f;  // -18 dBFS
constexpr float kPeak = 0.89f;       // -1 dBFS

std::u32string decode(std::string_view text)
{
    std::u32string out;
    out.reserve(text.size());
    for (std::size_t i = 0; i < text.size();) {
        const auto byte = static_cast<unsigned char>(text[i]);
        const std::size_t length = byte < 0x80             ? 1
                                   : (byte >> 5) == 0x6    ? 2
                                   : (byte >> 4) == 0xE    ? 3
                                   : (byte >> 3) == 0x1E   ? 4
                                                           : 0;
        if (length == 0 || i + length > text.size()) {
            ++i;  // not UTF-8: left out
            continue;
        }
        char32_t c = length == 1 ? byte : byte & (0x7Fu >> length);
        for (std::size_t k = 1; k < length; ++k)
            c = (c << 6) | (static_cast<unsigned char>(text[i + k]) & 0x3Fu);
        out += c;
        i += length;
    }
    return out;
}

// Python's \s for str patterns.
bool isSpace(char32_t c)
{
    return (c >= 0x09 && c <= 0x0D) || (c >= 0x1C && c <= 0x20) || c == 0x85 || c == 0xA0 || c == 0x1680
           || (c >= 0x2000 && c <= 0x200A) || c == 0x2028 || c == 0x2029 || c == 0x202F || c == 0x205F || c == 0x3000;
}

bool isEmoji(char32_t c)
{
    return (c >= 0x1F300 && c <= 0x1FAFF) || (c >= 0x2600 && c <= 0x27BF) || (c >= 0x1F1E6 && c <= 0x1F1FF);
}

// Unicode NFKD for what Spanish text holds: accented Latin letters become the letter and a combining mark
// (á → a + U+0301), and compatibility characters their plain form (no-break space → space, … → ...).
void appendNfkd(std::u32string &out, char32_t c)
{
    struct Split {
        char base;
        char32_t mark;
    };
    // U+00C0…U+00DF; the lower-case letters are 0x20 above, with the same marks.
    static constexpr std::array<Split, 32> latin1{{
        {'A', 0x300}, {'A', 0x301}, {'A', 0x302}, {'A', 0x303}, {'A', 0x308}, {'A', 0x30A}, {0, 0},       {'C', 0x327},
        {'E', 0x300}, {'E', 0x301}, {'E', 0x302}, {'E', 0x308}, {'I', 0x300}, {'I', 0x301}, {'I', 0x302}, {'I', 0x308},
        {0, 0},       {'N', 0x303}, {'O', 0x300}, {'O', 0x301}, {'O', 0x302}, {'O', 0x303}, {'O', 0x308}, {0, 0},
        {0, 0},       {'U', 0x300}, {'U', 0x301}, {'U', 0x302}, {'U', 0x308}, {'Y', 0x301}, {0, 0},       {0, 0},
    }};
    if (c >= 0xC0 && c <= 0xFF) {
        const bool lower = c >= 0xE0;
        const Split split = c == 0xFF ? Split{'Y', 0x308} : latin1[(c - 0xC0) % 32];
        if (split.base != 0) {
            out += static_cast<char32_t>(split.base + (lower ? 0x20 : 0));
            out += split.mark;
            return;
        }
        out += c;
        return;
    }
    switch (c) {
    case 0xA0: out += U' '; return;
    case 0xA8: out += U" ̈"; return;
    case 0xAA: out += U'a'; return;
    case 0xAF: out += U" ̄"; return;
    case 0xB2: out += U'2'; return;
    case 0xB3: out += U'3'; return;
    case 0xB4: out += U" ́"; return;
    case 0xB5: out += U'μ'; return;
    case 0xB8: out += U" ̧"; return;
    case 0xB9: out += U'1'; return;
    case 0xBA: out += U'o'; return;
    case 0x2011: out += U'‐'; return;
    case 0x2024: out += U'.'; return;
    case 0x2025: out += U".."; return;
    case 0x2026: out += U"..."; return;
    case 0x202F:
    case 0x205F:
    case 0x3000: out += U' '; return;
    default: break;
    }
    if (c >= 0x2000 && c <= 0x200A) {
        out += U' ';
        return;
    }
    out += c;
}

// Supertone's text preprocessing (UnicodeProcessor._preprocess_text), wrapped as <es>…</es>. "@" is read
// as "arroba" rather than Supertone's English "at".
std::u32string prepare(std::u32string_view text)
{
    std::u32string s;
    for (const char32_t c : text) {
        if (!isEmoji(c))
            appendNfkd(s, c);
    }
    std::u32string symbols;
    for (const char32_t c : s) {
        switch (c) {
        case 0x2013:
        case 0x2011:
        case 0x2014: symbols += U'-'; break;
        case 0xAF:
        case U'_':
        case U'[':
        case U']':
        case U'|':
        case U'/':
        case U'#':
        case 0x2192:
        case 0x2190: symbols += U' '; break;
        case 0x201C:
        case 0x201D: symbols += U'"'; break;
        case 0x2018:
        case 0x2019:
        case 0xB4:
        case U'`': symbols += U'\''; break;
        case 0x2665:
        case 0x2606:
        case 0x2661:
        case 0xA9:
        case U'\\': break;
        case U'@': symbols += U" arroba "; break;
        default: symbols += c;
        }
    }
    // No space before punctuation, one mark at a time as Supertone does.
    for (const char32_t mark : {U',', U'.', U'!', U'?', U';', U':', U'\''}) {
        std::u32string next;
        for (std::size_t i = 0; i < symbols.size(); ++i) {
            if (symbols[i] == U' ' && i + 1 < symbols.size() && symbols[i + 1] == mark)
                continue;
            next += symbols[i];
        }
        symbols = std::move(next);
    }
    // One quote where several repeat, and single spaces.
    std::u32string out;
    for (const char32_t c : symbols) {
        const bool quote = c == U'"' || c == U'\'' || c == U'`';
        if (quote && !out.empty() && out.back() == c)
            continue;
        const char32_t ch = isSpace(c) ? U' ' : c;
        if (ch == U' ' && (out.empty() || out.back() == U' '))
            continue;
        out += ch;
    }
    while (!out.empty() && out.back() == U' ')
        out.pop_back();
    static constexpr std::u32string_view endings = U".!?;:,'\")]}…。」』】〉》›»";
    if (out.empty() || endings.find(out.back()) == std::u32string_view::npos)
        out += U'.';
    return U"<es>" + out + U"</es>";
}

// Parts of at most kMaxPart characters, split between sentences, as Supertone's chunk_text.
std::vector<std::u32string> parts(std::u32string_view text)
{
    std::vector<std::u32string> sentences;
    std::u32string sentence;
    for (std::size_t i = 0; i < text.size(); ++i) {
        sentence += text[i];
        const bool end = text[i] == U'.' || text[i] == U'!' || text[i] == U'?';
        if (end && i + 1 < text.size() && isSpace(text[i + 1])) {
            sentences.push_back(std::move(sentence));
            sentence.clear();
        }
    }
    sentences.push_back(std::move(sentence));

    std::vector<std::u32string> out;
    std::u32string current;
    for (std::u32string &s : sentences) {
        std::size_t start = 0;
        std::size_t end = s.size();
        while (start < end && isSpace(s[start]))
            ++start;
        while (end > start && isSpace(s[end - 1]))
            --end;
        if (start == end)
            continue;
        const std::u32string_view trimmed = std::u32string_view(s).substr(start, end - start);
        if (current.size() + trimmed.size() + 1 <= kMaxPart) {
            if (!current.empty())
                current += U' ';
            current += trimmed;
        } else {
            if (!current.empty())
                out.push_back(std::move(current));
            current = trimmed;
        }
    }
    if (!current.empty())
        out.push_back(std::move(current));
    return out;
}

// numpy's legacy normal draws (np.random.seed, then np.random.randn): MT19937, 53-bit doubles from two
// draws, and the polar method, keeping the second value of each pair.
class NumpyNormal
{
public:
    explicit NumpyNormal(std::uint32_t seed)
        : m_mt(seed)
    {
    }

    double next()
    {
        if (m_hasSpare) {
            m_hasSpare = false;
            return m_spare;
        }
        double x1 = 0;
        double x2 = 0;
        double r2 = 0;
        do {
            x1 = 2.0 * uniform() - 1.0;
            x2 = 2.0 * uniform() - 1.0;
            r2 = x1 * x1 + x2 * x2;
        } while (r2 >= 1.0 || r2 == 0.0);
        const double f = std::sqrt(-2.0 * std::log(r2) / r2);
        m_spare = f * x1;
        m_hasSpare = true;
        return f * x2;
    }

private:
    double uniform()
    {
        const auto a = static_cast<std::uint32_t>(m_mt()) >> 5;
        const auto b = static_cast<std::uint32_t>(m_mt()) >> 6;
        return (a * 67108864.0 + b) / 9007199254740992.0;
    }

    std::mt19937 m_mt;
    bool m_hasSpare = false;
    double m_spare = 0;
};

// Speech latents made longer in time, before the vocoder turns them into sound, so it speaks slower with the
// pitch kept and nothing smeared: stretching the waveform itself (WSOLA) leaves a metallic echo. Past
// kMaxLatentStretch the latents alone measured less natural, so slower paces finish the stretch on the sound.
// Each step packs `compress` vocoder frames of `dim` values, channel = value × compress + frame; the frames
// are unpacked, interpolated linearly to `factor` times as many, and packed again.
std::vector<float> stretchLatent(const std::vector<float> &latent, int dim, int compress, std::int64_t steps,
                                 float factor, std::int64_t *outSteps)
{
    const std::int64_t frames = steps * compress;
    const auto newSteps = static_cast<std::int64_t>(std::ceil(static_cast<double>(steps) * factor));
    const std::int64_t newFrames = newSteps * compress;
    const auto at = [&](int value, std::int64_t frame) {
        return latent[static_cast<std::size_t>((value * compress + frame % compress) * steps + frame / compress)];
    };
    std::vector<float> out(static_cast<std::size_t>(dim * compress * newSteps));
    const double scale = newFrames > 1 ? static_cast<double>(frames - 1) / static_cast<double>(newFrames - 1) : 0.0;
    for (std::int64_t frame = 0; frame < newFrames; ++frame) {
        const double position = static_cast<double>(frame) * scale;
        const auto low = static_cast<std::int64_t>(position);
        const std::int64_t high = std::min(low + 1, frames - 1);
        const auto weight = static_cast<float>(position - static_cast<double>(low));
        for (int value = 0; value < dim; ++value) {
            out[static_cast<std::size_t>((value * compress + frame % compress) * newSteps + frame / compress)]
                = at(value, low) * (1.0f - weight) + at(value, high) * weight;
        }
    }
    *outSteps = newSteps;
    return out;
}

std::vector<float> flatten(const nlohmann::json &value)
{
    std::vector<float> out;
    const auto walk = [&](const auto &self, const nlohmann::json &v) -> void {
        if (v.is_array()) {
            for (const auto &item : v)
                self(self, item);
        } else {
            out.push_back(v.get<float>());
        }
    };
    walk(walk, value);
    return out;
}

struct Style {
    std::vector<float> ttl;
    std::vector<std::int64_t> ttlShape;
    std::vector<float> dp;
    std::vector<std::int64_t> dpShape;
};

using Pointer = nlohmann::json::json_pointer;

// The waveform made `factor` times longer with the pitch kept (WSOLA): 25 ms Hann windows at half overlap,
// each placed where it best continues the previous one, within 10 ms.
std::vector<float> stretchTime(const std::vector<float> &in, int rate, float factor)
{
    const auto n = static_cast<std::size_t>(0.025 * rate);
    const std::size_t hopOut = n / 2;
    const double hopIn = static_cast<double>(hopOut) / factor;
    const auto tolerance = static_cast<std::ptrdiff_t>(0.010 * rate);
    const auto outLength = static_cast<std::size_t>(static_cast<double>(in.size()) * factor);

    std::vector<float> padded(in.size() + n + 2 * static_cast<std::size_t>(tolerance), 0.0f);
    std::copy(in.begin(), in.end(), padded.begin() + tolerance);
    std::vector<float> window(n);  // Hann
    for (std::size_t i = 0; i < n; ++i) {
        const double phase = 2.0 * 3.14159265358979 * static_cast<double>(i) / static_cast<double>(n - 1);
        window[i] = static_cast<float>(0.5 - 0.5 * std::cos(phase));
    }

    std::vector<float> out(outLength + n, 0.0f);
    std::vector<float> weight(outLength + n, 0.0f);
    const auto correlation = [&](std::size_t a, std::size_t b) {
        double sum = 0;
        for (std::size_t i = 0; i < n; i += 2)  // every other sample is enough to compare shapes
            sum += static_cast<double>(padded[a + i]) * padded[b + i];
        return sum;
    };
    std::size_t previous = 0;
    for (std::size_t k = 0;; ++k) {
        const auto centre
            = static_cast<std::size_t>(static_cast<double>(k) * hopIn) + static_cast<std::size_t>(tolerance);
        if (centre + n + static_cast<std::size_t>(tolerance) > padded.size() || k * hopOut + n > out.size())
            break;
        std::size_t start = centre;
        if (k > 0) {
            // Coarse search every 4 samples, then the 3 on either side of the best.
            const std::size_t target = previous + hopOut;
            double best = -1e300;
            std::ptrdiff_t bestOffset = 0;
            for (std::ptrdiff_t d = -tolerance; d <= tolerance; d += 4) {
                const double c = correlation(static_cast<std::size_t>(static_cast<std::ptrdiff_t>(centre) + d), target);
                if (c > best) {
                    best = c;
                    bestOffset = d;
                }
            }
            const std::ptrdiff_t coarse = bestOffset;
            for (std::ptrdiff_t d = std::max(-tolerance, coarse - 3); d <= std::min(tolerance, coarse + 3); ++d) {
                const double c = correlation(static_cast<std::size_t>(static_cast<std::ptrdiff_t>(centre) + d), target);
                if (c > best) {
                    best = c;
                    bestOffset = d;
                }
            }
            start = static_cast<std::size_t>(static_cast<std::ptrdiff_t>(centre) + bestOffset);
        }
        for (std::size_t i = 0; i < n; ++i) {
            out[k * hopOut + i] += padded[start + i] * window[i];
            weight[k * hopOut + i] += window[i];
        }
        previous = start;
    }
    out.resize(outLength);
    for (std::size_t i = 0; i < outLength; ++i)
        out[i] = weight[i] > 1e-3f ? out[i] / weight[i] : out[i];
    return out;
}

// How many sentences end inside a part: a full stop, question or exclamation mark followed by a space.
std::size_t sentenceEnds(std::u32string_view part)
{
    std::size_t count = 0;
    for (std::size_t i = 0; i + 1 < part.size(); ++i) {
        if ((part[i] == U'.' || part[i] == U'!' || part[i] == U'?') && isSpace(part[i + 1]))
            ++count;
    }
    return count;
}

// The silences the model leaves between sentences drawn out to `pause` seconds, as advised for learners: as
// many of the longest silent runs inside the phrase (10 ms frames under 2 % of its loudest, 50 ms or more) as
// it has gaps between sentences. Speaking each sentence alone, with silence between, made more mistakes: on
// 36 course lines Whisper missed 3.8 % of M2's words instead of 1.5 %, a lone "¿Y tú?" came out garbled.
void lengthenPauses(std::vector<float> &wav, int rate, std::size_t gaps, float pause)
{
    const auto frame = static_cast<std::size_t>(rate / 100);
    const std::size_t frames = wav.size() / frame;
    if (gaps == 0 || frames == 0)
        return;
    std::vector<float> level(frames, 0.0f);
    float loudest = 0.0f;
    for (std::size_t f = 0; f < frames; ++f) {
        for (std::size_t i = f * frame; i < (f + 1) * frame; ++i)
            level[f] = std::max(level[f], std::fabs(wav[i]));
        loudest = std::max(loudest, level[f]);
    }
    const auto sound = [&](std::size_t f) { return level[f] > 0.02f * loudest; };
    std::size_t first = 0;
    while (first < frames && !sound(first))
        ++first;
    std::size_t last = frames;
    while (last > first && !sound(last - 1))
        --last;
    struct Run {
        std::size_t start;
        std::size_t length;
    };
    std::vector<Run> runs;
    for (std::size_t f = first; f < last;) {
        if (sound(f)) {
            ++f;
            continue;
        }
        const std::size_t start = f;
        while (f < last && !sound(f))
            ++f;
        if (f - start >= 5)
            runs.push_back({start, f - start});
    }
    std::stable_sort(runs.begin(), runs.end(), [](const Run &a, const Run &b) { return a.length > b.length; });
    runs.resize(std::min(runs.size(), gaps));
    // From the last, so the earlier ones stay where they were found.
    std::sort(runs.begin(), runs.end(), [](const Run &a, const Run &b) { return a.start > b.start; });
    const auto want = static_cast<std::size_t>(pause * static_cast<float>(rate));
    for (const Run &run : runs) {
        const std::size_t have = run.length * frame;
        if (want > have) {
            const auto middle = static_cast<std::ptrdiff_t>((run.start + run.length / 2) * frame);
            wav.insert(wav.begin() + middle, want - have, 0.0f);
        }
    }
}

// A second-order filter from Robert Bristow-Johnson's audio EQ cookbook, run over the samples in place.
struct Biquad {
    double b0, b1, b2, a1, a2;

    void run(std::vector<float> &x) const
    {
        double x1 = 0, x2 = 0, y1 = 0, y2 = 0;
        for (float &sample : x) {
            const double in = sample;
            const double out = b0 * in + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
            x2 = x1;
            x1 = in;
            y2 = y1;
            y1 = out;
            sample = static_cast<float>(out);
        }
    }
};

constexpr double kPi = 3.14159265358979323846;

Biquad highPass(double hz, int rate)
{
    const double w = 2 * kPi * hz / rate;
    const double alpha = std::sin(w) / (2 * 0.7071);  // Butterworth
    const double c = std::cos(w);
    const double a0 = 1 + alpha;
    return {(1 + c) / 2 / a0, -(1 + c) / a0, (1 + c) / 2 / a0, -2 * c / a0, (1 - alpha) / a0};
}

Biquad peaking(double hz, double gainDb, double q, int rate)
{
    const double a = std::pow(10.0, gainDb / 40);
    const double w = 2 * kPi * hz / rate;
    const double alpha = std::sin(w) / (2 * q);
    const double c = std::cos(w);
    const double a0 = 1 + alpha / a;
    return {(1 + alpha * a) / a0, -2 * c / a0, (1 - alpha * a) / a0, -2 * c / a0, (1 - alpha / a) / a0};
}

// The sound shaped for learners, as voice-learning presets advise: no rumble below 80 Hz and consonants a little
// more present (+1.5 dB around 3.2 kHz, about 2 to 5 kHz). On 36 course lines this changed neither the words
// Whisper missed nor UTMOS naturalness (M2 3.70 against 3.71, F3 3.98 both). The gentle compressor the presets
// also advise (2.5:1, 18 dB under the loudest moment) cost 0.11 to 0.18 in naturalness in every voice and was
// left out; steady loudness comes from levelling each phrase instead.
void clarify(std::vector<float> &x, int rate)
{
    highPass(80, rate).run(x);
    peaking(3200, 1.5, 0.8, rate).run(x);
}

// The gain that brings a phrase to kLoudness without a peak above kPeak.
float levelGain(const std::vector<float> &x, int rate)
{
    const auto frame = static_cast<std::size_t>(rate / 100);
    float peak = 0.0f;
    for (const float v : x)
        peak = std::max(peak, std::fabs(v));
    if (peak < 1e-6f)
        return 1.0f;
    double sum = 0;
    std::size_t count = 0;
    for (std::size_t start = 0; start + frame <= x.size(); start += frame) {
        float loudest = 0.0f;
        for (std::size_t i = start; i < start + frame; ++i)
            loudest = std::max(loudest, std::fabs(x[i]));
        if (loudest <= 0.02f * peak)
            continue;
        for (std::size_t i = start; i < start + frame; ++i)
            sum += static_cast<double>(x[i]) * x[i];
        count += frame;
    }
    const float speech = count > 0 ? static_cast<float>(std::sqrt(sum / static_cast<double>(count))) : peak;
    return std::min(kLoudness / speech, kPeak / peak);
}

} // namespace

struct SupertonicVoice::Impl {
    Ort::Env env{ORT_LOGGING_LEVEL_WARNING, "learnquick"};
    std::unique_ptr<Ort::Session> durationPredictor;
    std::unique_ptr<Ort::Session> textEncoder;
    std::unique_ptr<Ort::Session> vectorEstimator;
    std::unique_ptr<Ort::Session> vocoder;
    std::vector<std::int64_t> indexer;
    std::map<std::string, Style, std::less<>> styles;
    int sampleRate = 44100;
    int chunkSize = 3072;  // samples per latent frame: base_chunk_size × chunk_compress_factor
    int latentChannels = 144;  // latent_dim × chunk_compress_factor
    int compress = 6;          // chunk_compress_factor: vocoder frames packed in each latent step

    std::vector<std::int64_t> ids(std::u32string_view part) const;
    std::vector<float> speak(const std::vector<std::int64_t> &ids, const Style &style, NumpyNormal &noise,
                             float stretch) const;
};

SupertonicVoice::SupertonicVoice()
    : d(std::make_unique<Impl>())
{
}

SupertonicVoice::~SupertonicVoice() = default;

std::unique_ptr<SupertonicVoice> SupertonicVoice::load(const std::filesystem::path &dir, std::string *error)
{
    if (!ort::ready(error))
        return nullptr;
    const auto readJson = [&](const std::filesystem::path &path, nlohmann::json *out) {
        std::ifstream in(path, std::ios::binary);
        if (!in) {
            fail(error, "cannot read " + utf8(path));
            return false;
        }
        *out = nlohmann::json::parse(in, nullptr, false);
        if (out->is_discarded()) {
            fail(error, utf8(path) + " is not JSON");
            return false;
        }
        return true;
    };

    std::unique_ptr<SupertonicVoice> voice(new SupertonicVoice);
    Impl &v = *voice->d;
    nlohmann::json config;
    nlohmann::json indexer;
    if (!readJson(dir / "tts.json", &config) || !readJson(dir / "unicode_indexer.json", &indexer))
        return nullptr;
    try {
        v.sampleRate = config.at(Pointer("/ae/sample_rate")).get<int>();
        v.compress = config.at(Pointer("/ttl/chunk_compress_factor")).get<int>();
        v.chunkSize = config.at(Pointer("/ae/base_chunk_size")).get<int>() * v.compress;
        v.latentChannels = config.at(Pointer("/ttl/latent_dim")).get<int>() * v.compress;
        v.indexer = indexer.get<std::vector<std::int64_t>>();
    } catch (const nlohmann::json::exception &) {
        fail(error, utf8(dir / "tts.json") + " or unicode_indexer.json is not a Supertonic configuration");
        return nullptr;
    }

    std::error_code ec;
    for (const auto &entry : std::filesystem::directory_iterator(dir, ec)) {
        const std::filesystem::path &path = entry.path();
        const std::string name = utf8(path.stem());
        const bool styleName
            = name.size() == 2 && (name[0] == 'F' || name[0] == 'M') && name[1] >= '1' && name[1] <= '9';
        if (!styleName || path.extension() != ".json")
            continue;
        nlohmann::json json;
        if (!readJson(path, &json))
            return nullptr;
        try {
            Style style{flatten(json.at(Pointer("/style_ttl/data"))),
                        json.at(Pointer("/style_ttl/dims")).get<std::vector<std::int64_t>>(),
                        flatten(json.at(Pointer("/style_dp/data"))),
                        json.at(Pointer("/style_dp/dims")).get<std::vector<std::int64_t>>()};
            v.styles.emplace(name, std::move(style));
        } catch (const nlohmann::json::exception &) {
            fail(error, utf8(path) + " is not a Supertonic voice style");
            return nullptr;
        }
    }
    if (v.styles.empty()) {
        fail(error, "no voice styles in " + utf8(dir));
        return nullptr;
    }

    std::filesystem::path model;
    try {
        Ort::SessionOptions options;
        // A short phrase takes a fraction of a second; more threads than that only compete with the interface.
        const unsigned threads = std::clamp(std::thread::hardware_concurrency() / 2, 1u, 4u);
        options.SetIntraOpNumThreads(static_cast<int>(threads));
        options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        const auto open = [&](const char *file) {
            model = dir / file;
            return std::make_unique<Ort::Session>(v.env, model.c_str(), options);
        };
        v.durationPredictor = open("duration_predictor.onnx");
        v.textEncoder = open("text_encoder.onnx");
        v.vectorEstimator = open("vector_estimator.onnx");
        v.vocoder = open("vocoder.onnx");
    } catch (const Ort::Exception &e) {
        fail(error, "cannot load " + utf8(model) + ": " + e.what());
        return nullptr;
    }
    return voice;
}

int SupertonicVoice::sampleRate() const
{
    return d->sampleRate;
}

std::vector<std::string> SupertonicVoice::styles() const
{
    std::vector<std::string> names;
    for (const auto &[name, style] : d->styles)
        names.push_back(name);
    return names;
}

bool SupertonicVoice::hasStyle(std::string_view style) const
{
    return d->styles.find(style) != d->styles.end();
}

std::vector<std::int64_t> SupertonicVoice::textIds(std::string_view text) const
{
    return d->ids(decode(text));
}

std::u32string SupertonicVoice::unknownCharacters(std::string_view text) const
{
    std::u32string unknown;
    for (const char32_t c : prepare(decode(text))) {
        const bool known = c < d->indexer.size() && d->indexer[c] >= 0;
        if (!known && unknown.find(c) == std::u32string::npos)
            unknown += c;
    }
    return unknown;
}

std::vector<std::int64_t> SupertonicVoice::Impl::ids(std::u32string_view part) const
{
    std::vector<std::int64_t> out;
    for (const char32_t c : prepare(part)) {
        if (c < indexer.size() && indexer[c] >= 0)
            out.push_back(indexer[c]);
    }
    return out;
}

std::vector<float> SupertonicVoice::Impl::speak(const std::vector<std::int64_t> &ids, const Style &style,
                                                NumpyNormal &noise, float stretch) const
{
    Ort::MemoryInfo memory = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    const auto tensor = [&](auto &data, const std::vector<std::int64_t> &shape) {
        using T = std::remove_const_t<std::remove_reference_t<decltype(*data.data())>>;
        return Ort::Value::CreateTensor<T>(memory, const_cast<T *>(data.data()), data.size(), shape.data(),
                                           shape.size());
    };
    const auto length = static_cast<std::int64_t>(ids.size());
    const std::vector<float> textMask(ids.size(), 1.0f);
    const std::vector<std::int64_t> idsShape{1, length};
    const std::vector<std::int64_t> textMaskShape{1, 1, length};

    // How long the phrase lasts, in seconds.
    float duration = 0;
    {
        std::array<Ort::Value, 3> inputs{tensor(ids, idsShape), tensor(style.dp, style.dpShape),
                                         tensor(textMask, textMaskShape)};
        const std::array<const char *, 3> names{"text_ids", "style_dp", "text_mask"};
        const char *output = "duration";
        auto out = durationPredictor->Run(Ort::RunOptions{nullptr}, names.data(), inputs.data(), inputs.size(),
                                          &output, 1);
        duration = out.front().GetTensorData<float>()[0] / kSpeed * std::min(stretch, 1.0f);
    }
    const float samples = duration * static_cast<float>(sampleRate);
    const auto frames = std::max<std::int64_t>(
        1, static_cast<std::int64_t>((samples + static_cast<float>(chunkSize - 1)) / static_cast<float>(chunkSize)));

    std::vector<float> textEmbedding;
    std::vector<std::int64_t> embeddingShape;
    {
        std::array<Ort::Value, 3> inputs{tensor(ids, idsShape), tensor(style.ttl, style.ttlShape),
                                         tensor(textMask, textMaskShape)};
        const std::array<const char *, 3> names{"text_ids", "style_ttl", "text_mask"};
        const char *output = "text_emb";
        auto out = textEncoder->Run(Ort::RunOptions{nullptr}, names.data(), inputs.data(), inputs.size(), &output, 1);
        const float *data = out.front().GetTensorData<float>();
        textEmbedding.assign(data, data + out.front().GetTensorTypeAndShapeInfo().GetElementCount());
        embeddingShape = out.front().GetTensorTypeAndShapeInfo().GetShape();
    }

    std::vector<std::int64_t> latentShape{1, latentChannels, frames};
    std::vector<float> latent(static_cast<std::size_t>(latentChannels * frames));
    for (float &x : latent)
        x = static_cast<float>(noise.next());
    const std::vector<float> latentMask(static_cast<std::size_t>(frames), 1.0f);
    const std::vector<std::int64_t> latentMaskShape{1, 1, frames};
    const std::vector<float> totalStep{static_cast<float>(kSteps)};
    const std::vector<std::int64_t> oneShape{1};
    for (int step = 0; step < kSteps; ++step) {
        const std::vector<float> currentStep{static_cast<float>(step)};
        std::array<Ort::Value, 7> inputs{tensor(latent, latentShape),         tensor(textEmbedding, embeddingShape),
                                         tensor(style.ttl, style.ttlShape),   tensor(latentMask, latentMaskShape),
                                         tensor(textMask, textMaskShape),     tensor(currentStep, oneShape),
                                         tensor(totalStep, oneShape)};
        const std::array<const char *, 7> names{"noisy_latent", "text_emb",     "style_ttl", "latent_mask",
                                                "text_mask",    "current_step", "total_step"};
        const char *output = "denoised_latent";
        auto out = vectorEstimator->Run(Ort::RunOptions{nullptr}, names.data(), inputs.data(), inputs.size(),
                                        &output, 1);
        const float *data = out.front().GetTensorData<float>();
        std::copy(data, data + latent.size(), latent.begin());
    }

    float sampleCount = samples;
    if (stretch > 1.01f) {
        std::int64_t steps = frames;
        latent = stretchLatent(latent, latentChannels / compress, compress, frames, stretch, &steps);
        latentShape = {1, latentChannels, steps};
        sampleCount = samples * stretch;
    }
    Ort::Value input = tensor(latent, latentShape);
    const char *inputName = "latent";
    const char *outputName = "wav_tts";
    auto out = vocoder->Run(Ort::RunOptions{nullptr}, &inputName, &input, 1, &outputName, 1);
    const float *wav = out.front().GetTensorData<float>();
    const std::size_t count = out.front().GetTensorTypeAndShapeInfo().GetElementCount();
    // The vocoder fills whole frames; the phrase ends where the duration says.
    return {wav, wav + std::min(count, static_cast<std::size_t>(sampleCount))};
}

std::vector<std::int16_t> SupertonicVoice::synthesize(std::string_view text, std::string_view styleName,
                                                      float stretch, float pause) const
{
    const auto style = d->styles.find(styleName);
    if (style == d->styles.end())
        return {};
    std::vector<float> audio;
    std::size_t gaps = 0;  // between sentences
    try {
        const float pace = std::max(stretch, 1.0f);
        // Below 1, speak() asks the model itself to be quicker.
        const float latentPace = stretch < 1.0f ? std::max(stretch, 0.5f) : std::min(pace, kMaxLatentStretch);
        const float soundPace = pace / std::max(latentPace, 1.0f);
        NumpyNormal noise(seedFor(styleName));
        for (const std::u32string &part : parts(decode(text))) {
            const bool speakable = std::ranges::any_of(part, [](char32_t c) {
                return (c >= U'0' && c <= U'9') || (c >= U'A' && c <= U'Z') || (c >= U'a' && c <= U'z') || c >= 0xC0;
            });
            if (!speakable)
                continue;
            std::vector<float> wav = d->speak(d->ids(part), style->second, noise, latentPace);
            if (soundPace > 1.01f)
                wav = stretchTime(wav, d->sampleRate, soundPace);
            // Parts follow each other with the silence the model leaves at their ends, drawn out below.
            gaps += sentenceEnds(part) + (audio.empty() ? 0 : 1);
            audio.insert(audio.end(), wav.begin(), wav.end());
        }
    } catch (const Ort::Exception &) {
        return {};
    }
    if (audio.empty())
        return {};

    lengthenPauses(audio, d->sampleRate, gaps, std::max(pause, 0.0f));
    clarify(audio, d->sampleRate);
    const float scale = levelGain(audio, d->sampleRate) * 32767.0f;
    std::vector<std::int16_t> pcm;
    pcm.reserve(audio.size());
    for (const float x : audio)
        pcm.push_back(static_cast<std::int16_t>(std::lround(std::clamp(x * scale, -32768.0f, 32767.0f))));
    return pcm;
}

} // namespace lq
