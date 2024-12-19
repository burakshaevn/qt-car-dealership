#include "database_manager.h"

DatabaseManager::DatabaseManager() = default;

bool DatabaseManager::Open() {
    return db_.open();
}

void DatabaseManager::Close() {
    db_.close();
}

void DatabaseManager::UpdateConnection(const QStringView host, int port, const QStringView db_name, const QStringView username, const QStringView password) {
    db_ = QSqlDatabase::addDatabase("QPSQL");
    db_.setHostName(host.toString());
    db_.setPort(port);
    db_.setDatabaseName(db_name.toString());
    db_.setUserName(username.toString());
    db_.setPassword(password.toString());
}

QString DatabaseManager::GetLastError() const{
    return db_.lastError().text();
}

QVariant DatabaseManager::ExecuteQuery(const QStringView string_query) {
    QSqlQuery query;
    if (!query.exec(string_query.toString())) {
        return query.lastError().text();
    }
    return QString();
}

QVariant DatabaseManager::ExecuteSelectQuery(const QStringView string_query) {
    QSqlQuery query;
    if (!query.exec(string_query.toString())) {
        return query.lastError().text();
    }
    return QVariant::fromValue(query);
}

int DatabaseManager::GetRowsCount(QStringView table_name) const {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM information_schema.columns WHERE table_name = :table_name;");
    query.bindValue(":table_name", table_name.toString());
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return false;
}
