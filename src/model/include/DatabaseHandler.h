#pragma once

#ifndef DATABASE_HANDLER_H
#define DATABASE_HANDLER_H

#include <QList>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QVariant>
#include <QVariantMap>

#include "SqlQueries.h"

/*!
 * \brief Owns the SQLite connection and executes named, parameterised statements.
 *
 * - Statements are resolved by logical name (see SqlQueries.h) from resources;
 *   no SQL text lives in C++ code.
 * - Parameters are always bound (`:name` placeholders), never interpolated.
 * - Each handler uses its own named connection, so several handlers
 *   (e.g. application + tests) can coexist.
 */
class DatabaseHandler final
{
public:
    explicit DatabaseHandler(QString connectionName = QStringLiteral("dealership"));
    ~DatabaseHandler();

    DatabaseHandler(const DatabaseHandler&) = delete;
    DatabaseHandler& operator=(const DatabaseHandler&) = delete;

    /// Opens (creating if needed) an SQLite file and brings its schema up to date.
    bool open(const QString& databasePath);

    /// Opens the default database: $CAR_DEALERSHIP_DB or <AppDataLocation>/dealership.sqlite.
    bool openDefault();

    void close();
    [[nodiscard]] bool isOpen() const;

    [[nodiscard]] static QString defaultDatabasePath();
    [[nodiscard]] QString databasePath() const;
    [[nodiscard]] QString connectionName() const;
    [[nodiscard]] QSqlDatabase database() const;

    /// Runs a named statement and returns the active query (check isActive()).
    QSqlQuery select(SqlQuery::Name name, const QVariantMap& params = {}) const;

    /// Runs a named statement and returns every row as a column -> value map.
    [[nodiscard]] QList<QVariantMap> rows(SqlQuery::Name name, const QVariantMap& params = {}) const;

    /// Runs a named statement and returns the first column of the first row.
    [[nodiscard]] QVariant scalar(SqlQuery::Name name,
                                  const QVariantMap& params = {},
                                  const QVariant& fallback = {}) const;

    /// Runs a named DML statement. On failure \a userMessage receives a localized explanation.
    bool execute(SqlQuery::Name name, const QVariantMap& params = {}, QString* userMessage = nullptr);

    [[nodiscard]] QVariant lastInsertId() const;

    bool transaction();
    bool commit();
    bool rollback();

    /// Converts a driver error into a message suitable for the user (texts are stored in sys_strings).
    [[nodiscard]] QString userMessage(const QSqlError& error) const;
    [[nodiscard]] QSqlError lastError() const;

    /// Value from sys_strings.
    [[nodiscard]] QString string(const QString& category,
                                 const QString& key,
                                 const QString& fallback = {}) const;

private:
    bool prepareAndExec(QSqlQuery& query, SqlQuery::Name name, const QVariantMap& params) const;
    bool runBootstrap(const QString& name);

    QString m_connectionName;
    mutable QSqlError m_lastError;
    QVariant m_lastInsertId;
};

#endif // DATABASE_HANDLER_H
