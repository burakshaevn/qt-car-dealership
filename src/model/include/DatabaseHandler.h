#ifndef DATABASE_HANDLER_H
#define DATABASE_HANDLER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <optional>

#include "SystemData.h"

class DatabaseHandler {
public:
    DatabaseHandler();

    bool open();
    void close();

    /// Подключение к файлу SQLite
    void updateConnection(const QString& databasePath);

    /// Подключается к БД по умолчанию (путь берётся из окружения или дефолтный)
    void loadDefault();

    QString getLastError() const;

    /// Возвращает строку из sys_strings по категории и ключу
    QString getString(const QString& category, const QString& key,
                      const QString& fallback = QString()) const;
    /// Возвращает значение настройки из sys_settings
    QString getSetting(const QString& key, const QString& fallback = QString()) const;

    QString getTableDescription(QStringView tableName) const;
    QStringList getTables() const;

    bool executeQuery(QStringView stringQuery);
    bool executeQueryWithUserMessage(QStringView stringQuery, QString& errorMessage);
    QVariant executeSelectQuery(QStringView stringQuery) const;
    QSqlQuery executeNamedSelect(SqlQueryId queryId,
                                 const QVariantMap& bindings = {}) const;
    bool executeNamedQuery(SqlQueryId queryId,
                           const QVariantMap& bindings = {},
                           QString* errorMessage = nullptr);

    std::optional<int> tryGetCarTypeId(QStringView typeName) const;
    bool isKnownColor(QStringView color) const;
    QStringList getCarTypeNames() const;
    QString getDefaultCatalogColor() const;

    int getColumnsCount(QStringView tableName) const;
    int getMaxOrMinValueFromTable(const QString& maxOrMin,
                                  const QString& columnName,
                                  const QString& tableName);
    const QStringList getForeignKeysForColumn(const QString& tableName,
                                              const QString& columnName);
    QList<QString> getDistinctColors();

private:
    QSqlDatabase m_database;
};

#endif // DATABASE_HANDLER_H
