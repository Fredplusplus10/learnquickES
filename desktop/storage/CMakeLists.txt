# Local persistence: the learner's SQLite database (Qt SQL), with versioned schema migrations.
add_library(lq_storage STATIC
    include/lq/certificates.h
    include/lq/database.h
    include/lq/learner.h
    include/lq/profile.h
    include/lq/settings.h
    src/certificates.cpp
    src/database.cpp
    src/learner.cpp
    src/profile.cpp
    src/settings.cpp
)
target_include_directories(lq_storage PUBLIC include)
target_link_libraries(lq_storage PUBLIC lq_core Qt6::Core Qt6::Sql)
lq_set_compile_options(lq_storage)
