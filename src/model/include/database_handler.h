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
    /*!
     * \brief Конструктор по умолчанию
     */
    DatabaseHandler();

    /*!
     * \brief Шаблонный конструктор, принимающий универсальную ссылку (forwarding reference). Устанавливает параметры для подключения к БД
     * \param host — имя хоста
     * \param port — порт
     * \param db_name — название базы данных
     * \param username — имя пользователя, который подключается к БД
     * \param password — пароль этого пользователя
     */
    template <typename Type, typename PortType>
    DatabaseHandler(Type&& host, PortType&& port, Type&& db_name, Type&& username, Type&& password)
    {
        db_ = QSqlDatabase::addDatabase("QPSQL");
        db_.setHostName(std::forward<Type>(host));
        db_.setPort(std::forward<PortType>(port));
        db_.setDatabaseName(std::forward<Type>(db_name));
        db_.setUserName(std::forward<Type>(username));
        db_.setPassword(std::forward<Type>(password));
    }

    /*!
     * \brief Открывает соединение с БД
     * \returns true - открыта успешно, false - ошибка при открытии
     */
    bool Open();

    /*!
     * \brief Открывает соединение с БД
     */
    void Close();

    /*!
     * \brief
     * \param host - хост для соединения с БД
     * \param port - порт для соединения с БД
     * \param db_name - название базы данных, к которой подключаемся
     * \param username - имя пользователя, который подключается к БД
     * \param password - пароль пользователя, который подключается к БД
     */
    void UpdateConnection(const QString& host, int port, const QString& db_name, const QString& username, const QString& password);

    /*!
     * \brief Подключается к БД с параметрами (host, port, ...) по умолчанию
     */
    void LoadDefault();

    // Ensures inventory-related schema (stock/trim/order_requests) exists
    void EnsureInventorySchema();

    /*!
     * \brief Возвращает последнюю ошибку, возникшую в БД
     * \returns Текст ошибки в виде строки
     */
    QString GetLastError() const;

    /*!
     * \brief Возвращает описание таблицы
     * \param Название таблицы для которой выполняется запрос
     * \returns Текст описания таблицы
     */
    QString GetTableDescription(const QStringView table_name);

    /*!
     * \brief Возвращает доступные таблицы в БД
     * \returns Список строк, где каждая ячейка - название таблицы
     */
    QStringList GetTables() const;

    /*!
     * \brief Выполняет переданный ей SQL-запрос
     * \param string_query — сам SQL-запрос передаётся в виде ссылки на строку
     * \return true - запрос выполнен успешно, false - запрос не выполнен
     */
    bool ExecuteQuery(const QStringView string_query);

    /*!
     * \brief Выполняет SQL-запрос с красивой обработкой ошибок
     * \param string_query — сам SQL-запрос передаётся в виде ссылки на строку
     * \param error_message — сообщение об ошибке для пользователя
     * \return true - запрос выполнен успешно, false - запрос не выполнен
     */
    bool ExecuteQueryWithUserMessage(const QStringView string_query, QString& error_message);

    /*!
     * \brief Выполняет переданный ей SELECT SQL-запрос
     * \param string_query — сам SQL-запрос передаётся в виде ссылки на строку
     * \return QVariant
     */
    QVariant ExecuteSelectQuery(const QStringView string_query) const;

    /*!
     * \brief Возвращает количество столбцов у таблицы
     * \param Название таблицы для которой выполняется запрос
     * \returns Число строк таблицы
     */
    int GetRowsCount(QStringView table_name) const;

    /*!
     * \brief Ищет мин./макс. значение в таблице
     * \param max_or_min - пояснение, какое значение конкретно нужно найти
     * \param column_name - название столбца, в котором выполняется поиск
     * \param table_name - название таблицы, в которой выполняется операция
     * \returns Мин./макс. число
     */
    int GetMaxOrMinValueFromTable(const QString& max_or_min, const QString& column_name, const QString& table_name);

    /*!
     * \brief Возвращает внешние список внешних ключей для заданного столбца
     * \param table_name — название таблицы, для которой ищем внешние ключи
     * \param column — столбец, для которого нужно выполнить запрос
     * \return Список строк, содержащий имена внешних ключей таблицы
     */
    const QStringList GetForeignKeysForColumn(const QString& table_name, const QString& column);

    /*!
     * \brief Возвращает все существующие цвета у всех автомобилей
     * \returns Список, где в каждой ячейке название цвета
     */
    QList<QString> GetDistinctColors();

private:
    QSqlDatabase db_;   ///< Модуль Qt для работы с БД
};

#endif // DATABASE_HANDLERH_H
