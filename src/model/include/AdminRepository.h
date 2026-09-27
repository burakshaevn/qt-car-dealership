#pragma once

#ifndef ADMIN_REPOSITORY_H
#define ADMIN_REPOSITORY_H

#include <QHash>
#include <QList>
#include <QSharedPointer>
#include <QString>
#include <QStringList>

class DatabaseHandler;

/// Admin-visible table description (from sys_admin_tables).
struct AdminTableInfo
{
    QString TableName;     ///< Physical table (for editing)
    QString ViewName;      ///< Table or view used for display
    QString DisplayName;
    QString Description;
    bool IsRequest = false;
    QString ApproveStatus;
    QString RejectStatus;

    [[nodiscard]] bool isEditable() const { return ViewName == TableName; }
};

class AdminRepository final
{
public:
    explicit AdminRepository(QSharedPointer<DatabaseHandler> database);

    [[nodiscard]] QList<AdminTableInfo> tables() const;
    [[nodiscard]] qint64 salesTotal() const;
    /// "table.column" entries that reference \a tableName.
    [[nodiscard]] QStringList referencingTables(const QString& tableName) const;
    /// Human readable captions for the columns of \a tableName (column name → label).
    [[nodiscard]] QHash<QString, QString> columnLabels(const QString& tableName) const;

private:
    QSharedPointer<DatabaseHandler> m_database;
};

#endif // ADMIN_REPOSITORY_H
