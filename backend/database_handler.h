#ifndef DATABASE_HANDLERH_H
#define DATABASE_HANDLERH_H

#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QVariant>
#include <QDebug>
#include <utility>

class DatabaseHandler {
public:
    DatabaseHandler();

    template <typename Container, typename Port>
    DatabaseHandler(Container&& host, Port&& port, Container&& db_name, Container&& username, Container&& password)
    {
        db_ = QSqlDatabase::addDatabase("QPSQL");
        db_.setHostName(std::forward<Container>(host));
        db_.setPort(std::forward<Port>(port));
        db_.setDatabaseName(std::forward<Container>(db_name));
        db_.setUserName(std::forward<Container>(username));
        db_.setPassword(std::forward<Container>(password));
    }

    bool Open();
    void Close();
    void UpdateConnection(const QString& host, int port, const QString& db_name, const QString& username, const QString& password);

    void LoadDefault();

    QString GetLastError() const;

    QString GetTableDescription(const QStringView table_name);

    QStringList GetTables() const;

    bool ExecuteQuery(const QStringView string_query);
    QVariant ExecuteSelectQuery(const QStringView string_query) const;

    int GetRowsCount(QStringView table_name) const;

    int GetMaxOrMinValueFromTable(const QString& max_or_min, const QString& column_name, const QString& table_name);

    const QStringList GetForeignKeysForColumn(const QString& table_name, const QString& column);

    QList<QString> GetDistinctColors();

private:
    QSqlDatabase db_;
};

#endif // DATABASE_HANDLERH_H
