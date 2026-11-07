# The learning engine: plain C++20 with no Qt dependency, so it can be tested and reused anywhere.
add_library(lq_core STATIC
    include/lq/achievements.h
    include/lq/assessment.h
    include/lq/backup.h
    include/lq/calendar.h
    include/lq/coach.h
    include/lq/course.h
    include/lq/date.h
    include/lq/demo.h
    include/lq/exercises.h
    include/lq/progress.h
    include/lq/random.h
    include/lq/speech.h
    include/lq/srs.h
    include/lq/streak.h
    include/lq/text.h
    include/lq/verbs.h
    include/lq/viewing.h
    src/achievements.cpp
    src/assessment.cpp
    src/backup.cpp
    src/calendar.cpp
    src/coach.cpp
    src/course.cpp
    src/date.cpp
    src/demo.cpp
    src/exercises.cpp
    src/progress.cpp
    src/random.cpp
    src/speech.cpp
    src/srs.cpp
    src/streak.cpp
    src/text.cpp
    src/verbs.cpp
    src/viewing.cpp
)
target_include_directories(lq_core PUBLIC include)
target_compile_features(lq_core PUBLIC cxx_std_20)
# Only course.cpp and assessment.cpp parse JSON; the headers never expose it.
target_link_libraries(lq_core PRIVATE nlohmann_json::nlohmann_json)
lq_set_compile_options(lq_core)
