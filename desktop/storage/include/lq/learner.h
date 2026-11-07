#pragma once

#include "lq/profile.h"
#include "lq/progress.h"

#include <QSqlDatabase>
#include <QString>

#include <optional>

namespace lq {

// The learner's progress in SQLite. The plan (length, start, study days) comes from the profile, so it is
// passed in and never stored twice.
std::optional<LearnerState> loadLearner(const QSqlDatabase &db, const Plan &plan, QString *error = nullptr);

// Writes the whole state in one transaction: a few hundred rows at most, so a snapshot is simpler and
// safer than tracking changes, and a crash mid-save leaves the previous snapshot intact.
bool saveLearner(const QSqlDatabase &db, const LearnerState &state, QString *error = nullptr);

// Replaces the profile and the progress together, for a restored backup: if anything fails, neither changes.
bool restoreLearner(const QSqlDatabase &db, const Profile &profile, const LearnerState &state,
                    QString *error = nullptr);

} // namespace lq
