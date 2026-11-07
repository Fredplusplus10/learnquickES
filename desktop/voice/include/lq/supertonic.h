#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

// A Supertonic 2 voice (Supertone's flow-matching text-to-speech, exported to ONNX) run on the CPU with
// onnxruntime: text in, 16-bit mono PCM out. It reads text, not phonemes, so pass it through lq::speechText
// first. One model speaks with several voices, each a small style file (F1…F5 female, M1…M5 male).
namespace lq {

class SupertonicVoice
{
public:
    // dir holds the four models, tts.json, unicode_indexer.json and one <style>.json per voice. Null, with
    // the reason in *error, when any of them cannot be read.
    static std::unique_ptr<SupertonicVoice> load(const std::filesystem::path &dir, std::string *error = nullptr);
    ~SupertonicVoice();
    SupertonicVoice(const SupertonicVoice &) = delete;
    SupertonicVoice &operator=(const SupertonicVoice &) = delete;

    int sampleRate() const;
    // The voices found next to the model, sorted ("F3", "M1", "M4"…).
    std::vector<std::string> styles() const;
    bool hasStyle(std::string_view style) const;

    // stretch above 1 speaks slower: the phrase is planned at its natural pace and its speech latents are
    // stretched in time before the vocoder, up to 1.25, so the pitch stays and nothing echoes; a slower pace
    // stretches the sound for the rest. Asking the model itself for a slower pace slurs the sounds; a quicker
    // one (below 1, down to 0.5) is asked of it. The
    // pauses between sentences are drawn out to `pause` seconds, and the sound is shaped for learners. Phrases
    // longer than a few sentences are made in parts. The same text gives the same audio every time. Empty when
    // the style is unknown, nothing in the text can be spoken, or inference fails.
    std::vector<std::int16_t> synthesize(std::string_view text, std::string_view style, float stretch = 1.0f,
                                         float pause = 0.5f) const;

    // The model's input for one part: Supertonic's own text normalization, wrapped as <es>…</es>, as indexes
    // into its alphabet. Characters outside that alphabet are left out.
    std::vector<std::int64_t> textIds(std::string_view text) const;
    // The characters of the normalized text outside the model's alphabet, each once.
    std::u32string unknownCharacters(std::string_view text) const;

private:
    SupertonicVoice();
    struct Impl;
    std::unique_ptr<Impl> d;
};

} // namespace lq
