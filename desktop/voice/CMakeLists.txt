# lq_voice: the bundled neural voice. Supertonic 2 (exported to ONNX) run with onnxruntime on the CPU.
# Neither is committed: both are downloaded at configure time and pinned by hash, so a changed upstream file
# fails the configure step instead of shipping something unknown.

# 1.23.2 is the last release with a universal (arm64 + x86_64) macOS build, which the app needs.
set(LQ_ORT_VERSION 1.23.2)
if(WIN32)
    set(archive onnxruntime-win-x64-${LQ_ORT_VERSION}.zip)
    set(archive_sha256 0b38df9af21834e41e73d602d90db5cb06dbd1ca618948b8f1d66d607ac9f3cd)
elseif(APPLE)
    set(archive onnxruntime-osx-universal2-${LQ_ORT_VERSION}.tgz)
    set(archive_sha256 49ae8e3a66ccb18d98ad3fe7f5906b6d7887df8a5edd40f49eb2b14e20885809)
else()
    message(FATAL_ERROR "LQ_NEURAL_VOICE is set up for Windows and macOS only; configure with -DLQ_NEURAL_VOICE=OFF")
endif()
FetchContent_Declare(onnxruntime
    URL https://github.com/microsoft/onnxruntime/releases/download/v${LQ_ORT_VERSION}/${archive}
    URL_HASH SHA256=${archive_sha256}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(onnxruntime)
set(ort_dir "${onnxruntime_SOURCE_DIR}")

add_library(lq_onnxruntime SHARED IMPORTED GLOBAL)
set_target_properties(lq_onnxruntime PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${ort_dir}/include")
if(WIN32)
    set_target_properties(lq_onnxruntime PROPERTIES
        IMPORTED_LOCATION "${ort_dir}/lib/onnxruntime.dll"
        IMPORTED_IMPLIB "${ort_dir}/lib/onnxruntime.lib"
    )
    set(LQ_ORT_RUNTIME "${ort_dir}/lib/onnxruntime.dll" "${ort_dir}/lib/onnxruntime_providers_shared.dll" PARENT_SCOPE)
else()
    set_target_properties(lq_onnxruntime PROPERTIES IMPORTED_LOCATION "${ort_dir}/lib/libonnxruntime.${LQ_ORT_VERSION}.dylib")
    set(LQ_ORT_RUNTIME "${ort_dir}/lib/libonnxruntime.${LQ_ORT_VERSION}.dylib" PARENT_SCOPE)
endif()
set(LQ_ORT_DIR "${ort_dir}" PARENT_SCOPE)

# The voice: Supertonic 2 by Supertone, about 260 MB, 44.1 kHz. One model with ten voices; the app ships the
# four the course uses (F3, M1 M4 M5; see lq::voiceFor) and two more that Settings offers (F1, M2; see
# lq::voiceChoices). Model under the BigScience OpenRAIL-M license, which travels with it (LICENSE). Pinned to a
# commit of Supertone/supertonic-2; everything lands in one folder.
set(voice_url https://huggingface.co/Supertone/supertonic-2/resolve/75e6727618a02f323c720cba9478152d4bc16ca4)
set(voice_dir "${CMAKE_BINARY_DIR}/voices")
set(voice_files)
foreach(entry
        "onnx/duration_predictor.onnx=6d556b3691165c364be91dc0bd894656b5949f5acd2750d8ec2f954010845011"
        "onnx/text_encoder.onnx=dd5f535ed629f7df86071043e15f541ce1b2ab7f1bdbce4c7892b307bca79fa3"
        "onnx/vector_estimator.onnx=105e9d66fd8756876b210a6b4aa03fc393b1eaca3a8dadcc8d9a3bc785c86a35"
        "onnx/vocoder.onnx=19bd51f47a186069c752403518a40f7ea4c647455056d2511f7249691ecddf7c"
        "onnx/tts.json=ee531d9af9b80438a2ed703e22155ee6c83b12595ab22fd3bb6de94c7502fe96"
        "onnx/unicode_indexer.json=b7662a73a0703f43b97c0f2e089f8e8325e26f5d841aca393b5a54c509c92df1"
        "voice_styles/F1.json=6106950ebeb8a5da29ea22075f605db659cd07dbc288a68292543d9129aa250f"
        "voice_styles/F3.json=7eda5bccb4e6eb7f228fa182462d5fcf982d77628234603599027f0734d70c29"
        "voice_styles/M1.json=a04c823cbda6dd1c7de131ec68fea83bbb70d7f29d61623304eb871e3b83b5a1"
        "voice_styles/M2.json=7ddd07bf873a3fd67d09ef4e8293b486beb658158b47e371166198e4c6926072"
        "voice_styles/M4.json=95322725e4d25d9ed4e7dcccbf0f3726b0e9a2471d876b7942373218dbd30174"
        "voice_styles/M5.json=be52f82327da63ff18481ce2dd8060c7df432e0168d748745ef3e21b92d706a5"
        "LICENSE=0d944a9110fed9a9602d60e0423a272903e7bd21ab060490774efc77c2275e9f")
    string(REPLACE "=" ";" parts "${entry}")
    list(GET parts 0 remote)
    list(GET parts 1 sha256)
    get_filename_component(file "${remote}" NAME)
    set(path "${voice_dir}/${file}")
    set(current "")
    if(EXISTS "${path}")
        file(SHA256 "${path}" current)
    endif()
    if(NOT current STREQUAL sha256)
        message(STATUS "Downloading the voice: ${remote}")
        file(DOWNLOAD "${voice_url}/${remote}" "${path}" EXPECTED_HASH SHA256=${sha256} TLS_VERIFY ON)
    endif()
    list(APPEND voice_files "${path}")
endforeach()
list(APPEND voice_files "${CMAKE_CURRENT_SOURCE_DIR}/THIRD_PARTY_NOTICES.md")
set(LQ_VOICE_DIR "${voice_dir}" PARENT_SCOPE)
set(LQ_VOICE_FILES "${voice_files}" PARENT_SCOPE)
# onnxruntime's own license and the notices of what it bundles travel with the app.
set(LQ_ORT_NOTICES "${ort_dir}/LICENSE" "${ort_dir}/ThirdPartyNotices.txt" PARENT_SCOPE)

add_library(lq_voice STATIC
    include/lq/piper.h
    include/lq/supertonic.h
    src/ort.h
    src/piper.cpp
    src/supertonic.cpp
)
target_include_directories(lq_voice PUBLIC include)
target_link_libraries(lq_voice PUBLIC lq_core PRIVATE lq_onnxruntime nlohmann_json::nlohmann_json)
lq_set_compile_options(lq_voice)
