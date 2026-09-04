#include "SystemData.h"

#include <QFile>

namespace {
QString SqlFileName(const SqlQueryId queryId)
{
    switch (queryId) {
    case SqlQueryId::SelectAllProducts: return QStringLiteral("products/select_all.sql");
    case SqlQueryId::SelectProductsByName: return QStringLiteral("products/select_by_name.sql");
    case SqlQueryId::SelectAdminByUsername: return QStringLiteral("auth/select_admin_by_username.sql");
    case SqlQueryId::SelectClientByEmail: return QStringLiteral("auth/select_client_by_email.sql");
    case SqlQueryId::SelectClientByEmailOrPhone: return QStringLiteral("auth/select_client_by_email_or_phone.sql");
    case SqlQueryId::InsertClient: return QStringLiteral("auth/insert_client.sql");
    }
    return {};
}
}

QString SystemData::Sql(const SqlQueryId queryId)
{
    QFile file(QStringLiteral(SYSTEM_DATA_DIRECTORY) + QStringLiteral("/resources/sql/")
               + SqlFileName(queryId));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    return QString::fromUtf8(file.readAll()).trimmed();
}

QString SystemData::DefaultDatabasePath()
{
    return QStringLiteral(SYSTEM_DATA_DIRECTORY) + QStringLiteral("/sysdb/system_data.sqlite");
}
