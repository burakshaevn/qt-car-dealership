#pragma once

#include <QString>

enum class SqlQueryId {
    SelectAllProducts,
    SelectProductsByName,
    SelectAdminByUsername,
    SelectClientByEmail,
    SelectClientByEmailOrPhone,
    InsertClient,
};

/**
 * Access to database assets bundled with the application.
 *
 * SQL belongs to version-controlled .sql files, rather than to controllers or
 * repositories. Query identifiers describe application use-cases; they never
 * expose an SQL text or a resource path to application code.
 */
class SystemData final
{
public:
    [[nodiscard]] static QString Sql(SqlQueryId queryId);
    [[nodiscard]] static QString DefaultDatabasePath();
};
