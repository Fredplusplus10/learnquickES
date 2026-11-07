#pragma once

// What the voices share: onnxruntime bound by hand, and error reporting.
// The API is bound by hand in bindApi(), so a wrong onnxruntime library is reported instead of crashing.
#define ORT_API_MANUAL_INIT
#include <onnxruntime_cxx_api.h>

#include <filesystem>
#include <string>

namespace lq::ort {

inline void fail(std::string *error, const std::string &message)
{
    if (error)
        *error = message;
}

// For messages: path::string() throws on Windows when a folder name is outside the ANSI code page.
inline std::string utf8(const std::filesystem::path &path)
{
    const std::u8string text = path.u8string();
    return {reinterpret_cast<const char *>(text.data()), text.size()};
}

// Binds the C++ API to the onnxruntime library that was loaded. Null when that library is older than the
// headers: Windows has its own, older onnxruntime.dll in System32, which is picked when the app's copy is missing.
inline const OrtApi *bindApi()
{
    static const OrtApi *api = [] {
        const OrtApi *found = OrtGetApiBase()->GetApi(ORT_API_VERSION);
        if (found)
            Ort::InitApi(found);
        return found;
    }();
    return api;
}

// False, with the reason in *error, when the app's onnxruntime is missing or too old.
inline bool ready(std::string *error)
{
    if (bindApi())
        return true;
    fail(error, std::string("onnxruntime ") + OrtGetApiBase()->GetVersionString() + " lacks API version "
                    + std::to_string(ORT_API_VERSION) + "; the app's copy is missing");
    return false;
}

} // namespace lq::ort
