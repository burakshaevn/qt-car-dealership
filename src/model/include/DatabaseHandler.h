#ifndef DATABASE_HANDLER_H
#define DATABASE_HANDLER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVariant>
#include <optional>

class DatabaseHandler {
public:
    DatabaseHandler();

    bool Open();
    void Close();

    /// Подключение к файлу SQLite
    void UpdateConnection(const QString& database_path);

    /// Подключается к БД по умолчанию (путь берётся из окружения или дефолтный)
    void LoadDefault();

    /// Создаёт системные таблицы (sys_strings, sys_settings) и наполняет их
    void EnsureSystemSchema();
    /// Создаёт прикладные таблицы и триггеры
    void EnsureInventorySchema();

    QString GetLastError() const;

    /// Возвращает строку из sys_strings по категории и ключу
    QString GetString(const QString& category, const QString& key,
                      const QString& fallback = QString()) const;
    /// Возвращает значение настройки из sys_settings
    QString GetSetting(const QString& key, const QString& fallback = QString()) const;

    QString GetTableDescription(QStringView table_name) const;
    QStringList GetTables() const;

    bool ExecuteQuery(QStringView string_query);
    bool ExecuteQueryWithUserMessage(QStringView string_query, QString& error_message);
    QVariant ExecuteSelectQuery(QStringView string_query) const;

    std::optional<int> TryGetCarTypeId(QStringView type_name) const;
    bool IsKnownColor(QStringView color) const;
    QStringList GetCarTypeNames() const;
    QString GetDefaultCatalogColor() const;

    int GetColumnsCount(QStringView table_name) const;
    int GetMaxOrMinValueFromTable(const QString& max_or_min,
                                  const QString& column_name,
                                  const QString& table_name);
    const QStringList GetForeignKeysForColumn(const QString& table_name,
                                              const QString& column_name);
    QList<QString> GetDistinctColors();

private:
    bool TableExists(const QString& table) const;
    bool ColumnExists(const QString& table, const QString& column) const;

    QSqlDatabase m_database;
};

#endif // DATABASE_HANDLER_H