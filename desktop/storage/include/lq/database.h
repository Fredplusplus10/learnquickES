#pragma once

#include <QSqlDatabase>
#include <QString>

namespace lq {

// The learner's local SQLite file: opening it and migrating the schema, nothing else.
class Database
{
public:
    Database();
    ~Database();
    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;

    // Opens (creating if needed) the file and migrates it to the latest schema.
    // Before migrating a file that already has data, it saves a copy beside it (see migrationBackup()); when
    // the copy cannot be made, nothing is migrated and open() fails, so the learner's data stays as it was.
    // Returns false and sets error() on failure.
    bool open(const QString &filePath);
    void close();

    QSqlDatabase connection() const;
    int schemaVersion() const;
    QString error() const { return m_error; }
    // The copy made before the last migration ("learnquick.sqlite3.v3.bak" for data at schema 3), or empty.
    QString migrationBackup() const { return m_migrationBackup; }

    static int latestSchemaVersion();

private:
    bool migrate(const QString &filePath);
    bool backUpBeforeMigrating(const QString &filePath, int version);

    QString m_connectionName;
    QString m_error;
    QString m_migrationBackup;
};

} // namespace lq
