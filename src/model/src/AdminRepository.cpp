#include "AdminRepository.h"

#include "DatabaseHandler.h"

namespace Q = SqlQuery;

AdminRepository::AdminRepository(QSharedPointer<DatabaseHandler> database)
    : m_database(std::move(database))
{}

QList<AdminTableInfo> AdminRepository::tables() const
{
    QList<AdminTableInfo> result;
    if (!m_database) {
        return result;
    }
    const auto kRows = m_database->rows(Q::Admin::kSelectTables);
    for (const auto& row : kRows) {
        AdminTableInfo info;
        info.TableName = row.value("table_name").toString();
        info.ViewName = row.value("view_name").toString();
        info.DisplayName = row.value("display_name").toString();
        info.Description = row.value("description").toString();
        info.IsRequest = row.value("is_request").toBool();
        info.ApproveStatus = row.value("approve_status").toString();
        info.RejectStatus = row.value("reject_status").toString();
        result.append(info);
    }
    return result;
}

qint64 AdminRepository::salesTotal() const
{
    return m_database ? m_database->scalar(Q::Admin::kSelectSalesTotal, {}, 0).toLongLong() : 0;
}

QStringList AdminRepository::referencingTables(const QString& tableName) const
{
    QStringList result;
    if (!m_database) {
        return result;
    }
    const auto kRows = m_database->rows(Q::Admin::kSelectReferencingTables, {{"table_name", tableName}});
    for (const auto& row : kRows) {
        result.append(row.value("table_name").toString() + QLatin1Char('.') + row.value("column_name").toString());
    }
    return result;
}

QHash<QString, QString> AdminRepository::columnLabels(const QString& tableName) const
{
    QHash<QString, QString> result;
    if (!m_database) {
        return result;
    }
    const auto kRows = m_database->rows(Q::Admin::kSelectColumnLabels, {{"table_name", tableName}});
    for (const auto& row : kRows) {
        result.insert(row.value("column_name").toString(), row.value("label").toString());
    }
    return result;
}
