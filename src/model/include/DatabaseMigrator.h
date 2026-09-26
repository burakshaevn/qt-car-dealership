#pragma once

#ifndef DATABASE_MIGRATOR_H
#define DATABASE_MIGRATOR_H

#include <QSqlDatabase>
#include <QString>

/*!
 * \brief Brings an SQLite database to the latest schema version.
 *
 * Migrations are the ordered scripts from :/sql/migrations. Every migration runs
 * inside its own transaction and is recorded in `schema_migrations`, so the
 * process is idempotent: a fresh file gets the full schema and seed data, an
 * existing one receives only the missing steps.
 */
class DatabaseMigrator final
{
public:
    explicit DatabaseMigrator(QSqlDatabase database);

    bool migrate();

    [[nodiscard]] QString errorString() const;
    [[nodiscard]] int currentVersion() const;

private:
    bool execScript(const QString& script);

    QSqlDatabase m_database;
    QString m_error;
};

#endif // DATABASE_MIGRATOR_H
