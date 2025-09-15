#pragma once

#ifndef TABLE_MANAGER_H
#define TABLE_MANAGER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QSqlTableModel>
#include <QHeaderView>
#include <QInputDialog>
#include <QSqlField>
#include <QTableWidget>
#include <QStandardItemModel>
#include <QScrollArea>
#include <QMouseEvent>

#include "database_handler.h"
#include "domain.h"
#include "user.h"

/*!
 * \class Table
 * \brief Класс для управления и отображения таблиц базы данных с административными функциями
 * \details Предоставляет интерфейс для просмотра, добавления, редактирования и удаления записей
 *          в различных таблицах базы данных, а также обработки заявок (подтверждение/отклонение)
 */
class Table : public QWidget {
    Q_OBJECT

public:
    /*!
     * \brief Конструктор класса Table
     * \param db_manager Умный указатель на обработчик базы данных
     * \param user Указатель на объект пользователя для проверки прав доступа
     * \param parent Родительский виджет (опционально)
     */
    explicit Table(QSharedPointer<DatabaseHandler> db_manager, const User* user, QWidget* parent = nullptr);

    /*!
     * \brief Строит административный интерфейс таблиц
     * \details Инициализирует все элементы управления: селектор таблиц, кнопки действий,
     *          плавающее меню и соединяет сигналы со слотами
     */
    void BuildAdminTables();

    /*!
     * \brief Загружает данные выбранной таблицы в интерфейс
     * \details Выполняет SQL-запрос для получения данных таблицы и отображает их в QTableView.
     *          Автоматически определяет тип таблицы и настраивает соответствующий интерфейс
     */
    void LoadTable();

    /*!
     * \brief Добавляет новую запись в текущую таблицу
     * \details Открывает диалоговое окно для ввода данных новой записи и выполняет INSERT запрос
     */
    void AddRecord();

    /*!
     * \brief Удаляет выбранную запись из текущей таблицы
     * \details Запрашивает подтверждение и выполняет DELETE запрос с проверкой внешних ключей
     */
    void DeleteRecord();

    /*!
     * \brief Редактирует существующую запись в текущей таблице
     * \details Открывает диалоговое окно для изменения данных записи и выполняет UPDATE запрос
     */
    void EditRecord();

    /*!
     * \brief Получает имя столбца первичного ключа для указанной таблицы
     * \param table_name Имя таблицы для анализа
     * \return Имя столбца первичного ключа или пустую строку если не найден
     */
    QString GetPrimaryKeyColumnName(const QString& table_name);

    /*!
     * \brief Запрашивает подтверждение удаления записи
     * \param table_name Имя таблицы содержащей запись
     * \param primary_key_column Имя столбца первичного ключа
     * \param id Значение первичного ключа удаляемой записи
     * \return true если пользователь подтвердил удаление, иначе false
     */
    bool GetConfirmation(const QString& table_name, const QString& primary_key_column, int id);

    /*!
     * \brief Фильтр событий для реализации перетаскивания плавающего меню
     * \param obj Объект вызвавший событие
     * \param event Событие для обработки
     * \return true если событие обработано, иначе false
     */
    bool eventFilter(QObject* obj, QEvent* event);

private slots:
    /*!
     * \brief Подтверждает выбранную заявку
     * \details Устанавливает статус "подтверждено" или "одобрено" для выбранной заявки
     */
    void ApproveRequest();

    /*!
     * \brief Отклоняет выбранную заявку
     * \details Устанавливает статус "отменено" или "отклонено" для выбранной заявки
     */
    void RejectRequest();

signals:
    /*!
     * \brief Сигнал выхода из системы
     * \details Испускается при нажатии кнопки выхода
     */
    void Logout();

protected:
    QSharedPointer<DatabaseHandler> m_database_handler;    ///< Умный указатель на обработчик БД

    Tables m_current_table;                                ///< Текущая выбранная таблица из enum Tables
    QTableView* m_data_table;                              ///< Виджет для отображения данных таблицы
    QLabel* m_description_table;                           ///< Метка для описания текущей таблицы

    QComboBox* m_table_selector;                           ///< Выпадающий список для выбора таблицы

    QScopedPointer<QWidget> m_floating_menu;               ///< Плавающее меню с кнопками действий

    /*!
     * \brief Обновляет статус заявки в базе данных
     * \param table_name - Имя таблицы заявок
     * \param status - Новый статус заявки
     * \param request_id ID заявки для обновления
     */
    void UpdateRequestStatus(const QString& table_name, const QString& status, const int request_id);

    /*!
     * \brief Проверяет является ли таблица таблицей заявок
     * \param table_name - Имя таблицы для проверки
     * \return true - если таблица содержит заявки, иначе - false
     */
    bool IsRequestTable(const QString& table_name) const;

    /*!
     * \brief Показывает или скрывает кнопки обработки заявок
     * \param show true - показать кнопки, false - скрыть
     */
    void ShowRequestButtons(bool show);
};

#endif // TABLE_MANAGER_H
