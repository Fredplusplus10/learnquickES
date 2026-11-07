#pragma once

#include "lq/course.h"
#include "lq/progress.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

// The backup file, in the web app's format (app/js/core/store.js): {app: "LearnQuick", exported, state}. A
// backup from either app opens in the other. What only the desktop app keeps (the learner's reason, goals,
// streak emoji, reflections) travels under state.desktop, which the web app keeps untouched in its own saves.
namespace lq {

struct BackupProfile {
    std::string name;
    std::string level;  // "A1", "A2", "A2+"; empty when unknown
    // The desktop app's extras. desktop is false for a backup from the web app, which has none of them.
    bool desktop = false;
    std::string reason;
    std::vector<std::string> goals;
    std::string streakEmoji;
};

struct Backup {
    BackupProfile profile;
    LearnerState state;  // with the plan
};

// exportedAt: an ISO 8601 date and time, as the web app writes it.
std::string writeBackup(const Backup &backup, const Course &course, std::string_view exportedAt);

// nullopt with a reason in `error` for a file that is not a LearnQuick backup. Units and cards of lessons the
// course does not have are left out, and missing parts read as empty, as the web app's migrate() does. The web
// app's own settings (voice, reminder, interface language) are not read.
std::optional<Backup> readBackup(std::string_view text, const Course &course, std::string *error);

} // namespace lq
