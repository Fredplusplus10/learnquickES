cmake_minimum_required(VERSION 3.22)

# The oldest macOS the app opens on; it must come before project(). Without it the executable takes the build
# machine's version (macOS 26 on GitHub's runners) and refuses to open on anything older. Qt 6.12's own
# frameworks need 14.4, so the app cannot go lower than that.
set(CMAKE_OSX_DEPLOYMENT_TARGET "14.4" CACHE STRING "Oldest macOS LearnQuick runs on")

project(LearnQuick
    VERSION 0.4.5
    DESCRIPTION "LearnQuick native desktop app"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Multimedia Qml Quick Sql Svg TextToSpeech)
qt_standard_project_setup(REQUIRES 6.8)

option(LQ_BUILD_TESTS "Build the unit tests (downloads Catch2 and nlohmann/json)" ON)
# The bundled neural voice: onnxruntime and Supertonic 2, about 290 MB downloaded at configure time. Without it
# the app speaks with the operating system's voices only.
if(WIN32 OR APPLE)
    set(lq_neural_voice_default ON)
else()
    set(lq_neural_voice_default OFF)
endif()
option(LQ_NEURAL_VOICE "Bundle the neural Spanish voice (downloads onnxruntime and the voice)" ${lq_neural_voice_default})

# Strict warnings on our own targets; MSVC is also told the sources are UTF-8.
function(lq_set_compile_options target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /utf-8)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

# nlohmann/json reads the course file in the engine. Pinned by hash: a changed upstream archive fails the
# configure step instead of building unknown code.
include(FetchContent)
FetchContent_Declare(nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    URL_HASH SHA256=42f6e95cad6ec532fd372391373363b62a14af6d771056dbfc86160e6dfff7aa
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

add_subdirectory(core)
add_subdirectory(storage)
if(LQ_NEURAL_VOICE)
    add_subdirectory(voice)
endif()

# The target is lowercase so it never clashes with the LearnQuick/ folder of the QML module.
qt_add_executable(learnquick
    src/main.cpp
)

set_target_properties(learnquick PROPERTIES
    OUTPUT_NAME LearnQuick
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(learnquick PRIVATE
    lq_core lq_storage Qt6::Gui Qt6::Multimedia Qt6::Qml Qt6::Quick Qt6::Sql Qt6::Svg Qt6::TextToSpeech)
# The generated QML type registration includes C++ headers by file name.
target_include_directories(learnquick PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_definitions(learnquick PRIVATE LQ_VERSION="${PROJECT_VERSION}")
lq_set_compile_options(learnquick)

# Must be set before qt_add_qml_module, which reads it.
set_source_files_properties(Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(learnquick
    URI LearnQuick
    VERSION 1.0
    QML_FILES
        AssessmentPage.qml
        Breadcrumb.qml
        Card.qml
        CertificatesPage.qml
        CheckpointPage.qml
        Chip.qml
        ChoiceCard.qml
        CoachCard.qml
        CoachPage.qml
        ExerciseSession.qml
        Icon.qml
        ImmersionPage.qml
        InputField.qml
        LessonPage.qml
        LessonsPage.qml
        Main.qml
        MockExamPage.qml
        PageHero.qml
        StreakCard.qml
        CalendarCard.qml
        PracticePage.qml
        PrimaryButton.qml
        ProgressPage.qml
        ProgressRing.qml
        ReflectionCard.qml
        ScrollPage.qml
        SessionSummary.qml
        SettingsPage.qml
        Shell.qml
        SpanishKeyboard.qml
        SpanishWord.qml
        SpeakButton.qml
        StreakEmojiPicker.qml
        StreakIcon.qml
        Theme.qml
        TodayPage.qml
        TrainerPage.qml
        VocabularyPage.qml
        Welcome.qml
    SOURCES
        src/buildinfo.cpp
        src/buildinfo.h
        src/certificatecontroller.cpp
        src/certificatecontroller.h
        src/learnercontroller.cpp
        src/learnercontroller.h
        src/learnerbackup.cpp
        src/learnerreports.cpp
        src/learnerviewing.cpp
        src/preferencescontroller.cpp
        src/preferencescontroller.h
        src/profilecontroller.cpp
        src/profilecontroller.h
        src/qtdate.h
        src/sessionmodel.cpp
        src/sessionmodel.h
        src/speechcontroller.cpp
        src/speechcontroller.h
        src/todaymodel.cpp
        src/todaymodel.h
    RESOURCES
        assets/certificates/badges/level_A1.svg
        assets/certificates/badges/level_A2.svg
        assets/certificates/badges/level_B1.svg
        assets/certificates/certificate_A1.svg
        assets/certificates/certificate_A2.svg
        assets/certificates/certificate_B1.svg
        assets/illustrations/empty-caught-up.svg
        assets/illustrations/empty-immersion.svg
        assets/illustrations/empty-nothing-yet.svg
        assets/illustrations/empty-plan-done.svg
        assets/illustrations/empty-search.svg
        assets/illustrations/spanish-town-banner.svg
        content/assessments.json
        content/course.json
        images/learnquick.png
)

if(APPLE)
    set(LQ_ICNS "${CMAKE_CURRENT_SOURCE_DIR}/platform/macos/LearnQuick.icns")
    set_source_files_properties("${LQ_ICNS}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    target_sources(learnquick PRIVATE "${LQ_ICNS}")
    set_target_properties(learnquick PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/platform/macos/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME LearnQuick
        MACOSX_BUNDLE_GUI_IDENTIFIER com.learnquick.desktop
        MACOSX_BUNDLE_ICON_FILE LearnQuick.icns
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    )
elseif(WIN32)
    target_sources(learnquick PRIVATE platform/windows/LearnQuick.rc)
endif()

if(LQ_NEURAL_VOICE)
    target_link_libraries(learnquick PRIVATE lq_voice)
    target_compile_definitions(learnquick PRIVATE LQ_NEURAL_VOICE)
    # The voice travels next to the executable in the build tree too, so a build runs as the installed app does.
    # Windows looks in the executable's folder before System32, which has an older onnxruntime.dll of its own.
    if(APPLE)
        set_source_files_properties(${LQ_ORT_RUNTIME} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
        set_source_files_properties(${LQ_VOICE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/voices)
        set_source_files_properties(${LQ_ORT_NOTICES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/voices/onnxruntime)
        target_sources(learnquick PRIVATE ${LQ_ORT_RUNTIME} ${LQ_VOICE_FILES} ${LQ_ORT_NOTICES})
        set_target_properties(learnquick PROPERTIES
            BUILD_RPATH "@executable_path/../Frameworks"
            INSTALL_RPATH "@executable_path/../Frameworks"
        )
    else()
        add_custom_command(TARGET learnquick POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LQ_ORT_RUNTIME} "$<TARGET_FILE_DIR:learnquick>"
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:learnquick>/voices/onnxruntime"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LQ_VOICE_FILES} "$<TARGET_FILE_DIR:learnquick>/voices"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LQ_ORT_NOTICES} "$<TARGET_FILE_DIR:learnquick>/voices/onnxruntime"
            VERBATIM
        )
    endif()
endif()

enable_testing()
# Smoke tests: start the app headless on a screen and quit. Any QML warning fails them.
# The welcome flow runs on an empty database in the build tree; every other screen on the sample learner,
# so each one opens with real progress behind it (lesson 8, index 7, is the sample learner's lesson of the day).
add_test(NAME smoke-welcome COMMAND learnquick --smoke-test --data-dir "${CMAKE_CURRENT_BINARY_DIR}/smoke-data")
set(LQ_SMOKE_TESTS smoke-welcome)
set(LQ_SMOKE_ROUTES
    today lessons
    lesson/7/vocab lesson/7/grammar lesson/7/dialogue lesson/7/practice lesson/7/speak lesson/7/done
    practice/review practice/verbs practice/mixed practice/speak practice/cognates practice/falsefriends
    practice/gender practice/past practice/describe
    checkpoint/2 coach settings practice vocab immersion immersion/island progress certificates assessment/A1
    mock/intro mock/reading mock/listening mock/grammar mock/writing mock/speaking mock/result
)
foreach(route IN LISTS LQ_SMOKE_ROUTES)
    string(REPLACE "/" "-" name "smoke-${route}")
    add_test(NAME ${name} COMMAND learnquick --smoke-test --demo --today 2026-11-02 --open ${route})
    list(APPEND LQ_SMOKE_TESTS ${name})
endforeach()
# Night mode binds other colours: the screens with the most of them, again in the dark.
foreach(route IN ITEMS today lesson/7/dialogue lesson/7/practice practice/verbs settings immersion progress certificates)
    string(REPLACE "/" "-" name "smoke-night-${route}")
    add_test(NAME ${name} COMMAND learnquick --smoke-test --demo --today 2026-11-02 --open ${route} --theme dark)
    list(APPEND LQ_SMOKE_TESTS ${name})
endforeach()
set_tests_properties(${LQ_SMOKE_TESTS} PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software"
)
if(WIN32)
    # Lets ctest find the Qt DLLs without adding Qt to the system PATH.
    set_property(TEST ${LQ_SMOKE_TESTS} PROPERTY ENVIRONMENT_MODIFICATION
        "PATH=path_list_prepend:${QT6_INSTALL_PREFIX}/${QT6_INSTALL_BINS}")
endif()

if(LQ_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# `cmake --install` produces a self-contained app: LearnQuick.app on macOS, bin/ with the Qt DLLs on Windows.
include(GNUInstallDirs)
install(TARGETS learnquick
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
# The Visual C++ runtime (vcruntime140, msvcp140...) travels next to the exe, as Microsoft allows, so the app
# starts on a PC that never installed it. Qt's deployment only adds its installer, vc_redist.x64.exe.
if(MSVC)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${CMAKE_INSTALL_BINDIR})
    set(CMAKE_INSTALL_UCRT_LIBRARIES FALSE)
    include(InstallRequiredSystemLibraries)
endif()
if(LQ_NEURAL_VOICE AND WIN32)
    # On macOS the voice is already inside LearnQuick.app.
    install(FILES ${LQ_ORT_RUNTIME} DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES ${LQ_VOICE_FILES} DESTINATION ${CMAKE_INSTALL_BINDIR}/voices)
    install(FILES ${LQ_ORT_NOTICES} DESTINATION ${CMAKE_INSTALL_BINDIR}/voices/onnxruntime)
endif()
# Qt's deployment ships only the windowed platform plugin. The offscreen one travels too, so the smoke test and
# the screenshots run on the installed app with nothing from a Qt installation (see "Fallos conocidos").
if(WIN32)
    # Deployment puts plugins in <prefix>/plugins, next to bin/, and writes a qt.conf that points there.
    install(FILES "$<TARGET_FILE:Qt6::QOffscreenIntegrationPlugin>" DESTINATION plugins/platforms)
elseif(APPLE)
    install(FILES "$<TARGET_FILE:Qt6::QOffscreenIntegrationPlugin>" DESTINATION LearnQuick.app/Contents/PlugIns/platforms)
    # Qt's plugins find their frameworks through @rpath; this one is copied as is, so it is told where they are.
    install(CODE [[
        file(GLOB lq_offscreen "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/LearnQuick.app/Contents/PlugIns/platforms/libqoffscreen*.dylib")
        execute_process(COMMAND install_name_tool -add_rpath @loader_path/../../Frameworks ${lq_offscreen}
                        COMMAND_ERROR_IS_FATAL ANY)
    ]])
endif()
qt_generate_deploy_qml_app_script(
    TARGET learnquick
    OUTPUT_SCRIPT deploy_script
)
install(SCRIPT ${deploy_script})

# LearnQuick's license and the notices and license texts of everything the app ships (Qt, FFmpeg, ONNX Runtime,
# the voice...) travel with it, as the LGPL and the other licenses ask. See THIRD_PARTY_NOTICES.md.
set(LQ_LICENSE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE" "${CMAKE_CURRENT_SOURCE_DIR}/../THIRD_PARTY_NOTICES.md")
if(APPLE)
    set(LQ_LICENSE_DIR LearnQuick.app/Contents/Resources/licenses)
else()
    set(LQ_LICENSE_DIR ${CMAKE_INSTALL_BINDIR}/licenses)
endif()
install(FILES ${LQ_LICENSE_FILES} DESTINATION ${LQ_LICENSE_DIR})
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSES/" DESTINATION ${LQ_LICENSE_DIR}/LICENSES)
