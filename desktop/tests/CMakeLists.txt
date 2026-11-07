include(FetchContent)
# Pinned by hash: a changed upstream archive fails the configure step instead of building unknown code.
# nlohmann/json is already available from the top-level CMakeLists.txt.
FetchContent_Declare(Catch2
    URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.16.1.tar.gz
    URL_HASH SHA256=d16bd1b8b2364918bd472cb216394acb93bde6784890df0d91bccfb3f28d777d
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(Catch2)

# The engine, against hand-written cases and against the web app's own output.
add_executable(lq_core_tests
    achievement_tests.cpp
    assessment_tests.cpp
    backup_tests.cpp
    calendar_tests.cpp
    coach_tests.cpp
    course_tests.cpp
    date_tests.cpp
    demo_tests.cpp
    exercise_tests.cpp
    progress_tests.cpp
    speech_tests.cpp
    srs_tests.cpp
    streak_tests.cpp
    test_support.h
    text_tests.cpp
    verbs_tests.cpp
    viewing_tests.cpp
    web_parity_tests.cpp
)
target_link_libraries(lq_core_tests PRIVATE lq_core Catch2::Catch2WithMain nlohmann_json::nlohmann_json)
target_compile_definitions(lq_core_tests PRIVATE
    LQ_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
    LQ_CONTENT_DIR="${PROJECT_SOURCE_DIR}/content"
)
lq_set_compile_options(lq_core_tests)
add_test(NAME core COMMAND lq_core_tests)

# The SQLite layer, on throwaway database files.
add_executable(lq_storage_tests storage_tests.cpp certificate_tests.cpp)
target_link_libraries(lq_storage_tests PRIVATE lq_storage Catch2::Catch2)
target_compile_definitions(lq_storage_tests PRIVATE LQ_ASSETS_DIR="${PROJECT_SOURCE_DIR}/assets")
lq_set_compile_options(lq_storage_tests)
add_test(NAME storage COMMAND lq_storage_tests)

# The view models behind the screens, driven from C++ without QML.
add_executable(lq_app_tests
    app_tests.cpp
    ${PROJECT_SOURCE_DIR}/src/learnercontroller.cpp
    ${PROJECT_SOURCE_DIR}/src/learnercontroller.h
    ${PROJECT_SOURCE_DIR}/src/learnerbackup.cpp
    ${PROJECT_SOURCE_DIR}/src/learnerreports.cpp
    ${PROJECT_SOURCE_DIR}/src/learnerviewing.cpp
    ${PROJECT_SOURCE_DIR}/src/preferencescontroller.cpp
    ${PROJECT_SOURCE_DIR}/src/preferencescontroller.h
    ${PROJECT_SOURCE_DIR}/src/profilecontroller.cpp
    ${PROJECT_SOURCE_DIR}/src/profilecontroller.h
    ${PROJECT_SOURCE_DIR}/src/sessionmodel.cpp
    ${PROJECT_SOURCE_DIR}/src/sessionmodel.h
    ${PROJECT_SOURCE_DIR}/src/todaymodel.cpp
    ${PROJECT_SOURCE_DIR}/src/todaymodel.h
)
target_include_directories(lq_app_tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(lq_app_tests PRIVATE lq_core lq_storage Qt6::Qml Catch2::Catch2)
target_compile_definitions(lq_app_tests PRIVATE
    LQ_CONTENT_DIR="${PROJECT_SOURCE_DIR}/content"
    LQ_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)
lq_set_compile_options(lq_app_tests)
add_test(NAME app COMMAND lq_app_tests)

# The certificates screen's controller: what it shows, and saving and exporting an earned certificate.
add_executable(lq_certificate_tests
    certificate_app_tests.cpp
    ${PROJECT_SOURCE_DIR}/src/certificatecontroller.cpp
    ${PROJECT_SOURCE_DIR}/src/certificatecontroller.h
    ${PROJECT_SOURCE_DIR}/src/profilecontroller.cpp
    ${PROJECT_SOURCE_DIR}/src/profilecontroller.h
)
target_include_directories(lq_certificate_tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(lq_certificate_tests PRIVATE lq_core lq_storage Qt6::Gui Qt6::Qml Qt6::Svg Catch2::Catch2)
target_compile_definitions(lq_certificate_tests PRIVATE LQ_ASSETS_DIR="${PROJECT_SOURCE_DIR}/assets")
lq_set_compile_options(lq_certificate_tests)
add_test(NAME certificates COMMAND lq_certificate_tests)
set_tests_properties(certificates PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# The on-screen keyboard, clicked in a real Qt Quick scene.
find_package(Qt6 REQUIRED COMPONENTS QuickTest Test)
add_executable(lq_keyboard_tests keyboard_tests.cpp)
target_link_libraries(lq_keyboard_tests PRIVATE Qt6::Quick Qt6::QuickTest Qt6::Test Catch2::Catch2)
target_compile_definitions(lq_keyboard_tests PRIVATE LQ_QML_DIR="${PROJECT_SOURCE_DIR}")
lq_set_compile_options(lq_keyboard_tests)
add_test(NAME keyboard COMMAND lq_keyboard_tests)
set_tests_properties(keyboard PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")

# The neural voice, on the downloaded model: it loads, speaks every voice the course uses, and can say every line.
if(LQ_NEURAL_VOICE)
    add_executable(lq_voice_tests voice_tests.cpp)
    target_link_libraries(lq_voice_tests PRIVATE lq_voice Catch2::Catch2WithMain nlohmann_json::nlohmann_json)
    target_compile_definitions(lq_voice_tests PRIVATE
        LQ_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
        LQ_CONTENT_DIR="${PROJECT_SOURCE_DIR}/content"
        LQ_VOICE_DIR="${LQ_VOICE_DIR}"
    )
    lq_set_compile_options(lq_voice_tests)
    if(WIN32)
        # Next to the executable, never through PATH: System32 has an older onnxruntime.dll that would win.
        add_custom_command(TARGET lq_voice_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LQ_ORT_RUNTIME} "$<TARGET_FILE_DIR:lq_voice_tests>"
            VERBATIM
        )
    else()
        set_target_properties(lq_voice_tests PROPERTIES BUILD_RPATH "${LQ_ORT_DIR}/lib")
    endif()
    add_test(NAME voice COMMAND lq_voice_tests)
endif()

if(WIN32)
    set_property(TEST storage app certificates keyboard PROPERTY ENVIRONMENT_MODIFICATION
        "PATH=path_list_prepend:${QT6_INSTALL_PREFIX}/${QT6_INSTALL_BINS}")
endif()
