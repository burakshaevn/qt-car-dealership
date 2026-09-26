#pragma once

#include <QString>
#include <QStringList>

/*!
 * \brief Splits an SQLite script into individual statements.
 *
 * QSqlQuery executes exactly one statement at a time, so migration scripts
 * have to be split before execution. The splitter understands:
 *  - string literals and quoted identifiers ('…', "…", `…`, […]);
 *  - line (--) and block comments;
 *  - CREATE TRIGGER bodies (BEGIN … END), including nested CASE … END.
 */
namespace SqlScript {

[[nodiscard]] QStringList splitStatements(const QString& script);

} // namespace SqlScript
