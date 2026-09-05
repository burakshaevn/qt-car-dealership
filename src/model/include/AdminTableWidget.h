#pragma once

#ifndef ADMINTABLEWIDGET_H
#define ADMINTABLEWIDGET_H

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
#include <QScrollArea>
#include <QMouseEvent>

#include "AdminTableModel.h"
#include "DatabaseHandler.h"
#include "PriceFormatter.h"

/*!
 * \class AdminTableWidget
 * \brief Класс для управления и отображения таблиц базы данных с административными функциями
 * \details Предоставляет интерфейс для просмотра, добавления, редактирования и удаления записей
 *          в различных таблицах базы данных, а также обработки заявок (подтверждение/отклонение)
 */
class AdminTableWidget : public QWidget {
    Q_OBJECT

public:
    /*!
     * \brief Конструктор класса AdminTableWidget
     * \param db_manager Умный указатель на обработчик базы данных
     * \param parent Родительский виджет (опционально)
     */
    explicit AdminTableWidget(QSharedPointer<DatabaseHandler> dbManager, QWidget* parent = nullptr);

    /*!
     * \brief Строит административный интерфейс таблиц
     * \details Инициализирует все элементы управления: селектор таблиц, кнопки действий,
     *          плавающее меню и соединяет сигналы со слотами
     */
    void buildAdminTables();

    /*!
     * \brief Загружает данные выбранной таблицы в интерфейс
     * \details Выполняет SQL-запрос для получения данных таблицы и отображает их в QTableView.
     *          Автоматически определяет тип таблицы и настраивает соответствующий интерфейс
     */
    void loadTable();

    /*!
     * \brief Добавляет новую запись в текущую таблицу
     * \details Открывает диалоговое окно для ввода данных новой записи и выполняет INSERT запрос
     */
    void addRecord();

    /*!
     * \brief Удаляет выбранную запись из текущей таблицы
     * \details Запрашивает подтверждение и выполняет DELETE запрос с проверкой внешних ключей
     */
    void deleteRecord();

    /*!
     * \brief Редактирует существующую запись в текущей таблице
     * \details Открывает диалоговое окно для изменения данных записи и выполняет UPDATE запрос
     */
    void editRecord();

    /*!
     * \brief Получает имя столбца первичного ключа для указанной таблицы
     * \param table_name Имя таблицы для анализа
     * \return Имя столбца первичного ключа или пустую строку если не найден
     */
    QString getPrimaryKeyColumnName(const QString& tableName);

    /*!
     * \brief Запрашивает подтверждение удаления записи
     * \param table_name Имя таблицы содержащей запись
     * \param primary_key_column Имя столбца первичного ключа
     * \param id Значение первичного ключа удаляемой записи
     * \return true если пользователь подтвердил удаление, иначе false
     */
    bool getConfirmation(const QString& tableName, const QString& primaryKeyColumn, int id);

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
    void approveRequest();

    /*!
     * \brief Отклоняет выбранную заявку
     * \details Устанавливает статус "отменено" или "отклонено" для выбранной заявки
     */
    void rejectRequest();

signals:
    /*!
     * \brief Сигнал выхода из системы
     * \details Испускается при нажатии кнопки выхода
     */
    void logout();

protected:
    QSharedPointer<DatabaseHandler> m_databaseHandler; ///< Умный указатель на обработчик БД
    QScopedPointer<AdminTableModel> m_tableModel;      ///< Модель данных админской таблицы

    QTableView* m_dataTable;    ///< Виджет для отображения данных таблицы
    QLabel* m_descriptionTable; ///< Метка для описания текущей таблицы

    QComboBox* m_tableSelector; ///< Выпадающий список для выбора таблицы

    QScopedPointer<QWidget> m_floatingMenu; ///< Плавающее меню с кнопками действий

    /*!
     * \brief Обновляет статус заявки в базе данных
     * \param table_name - Имя таблицы заявок
     * \param status - Новый статус заявки
     * \param request_id ID заявки для обновления
     */
    void updateRequestStatus(const QString& tableName, const QString& status, const int kRequestId);

    /*!
     * \brief Проверяет является ли таблица таблицей заявок
     * \param table_name - Имя таблицы для проверки
     * \return true - если таблица содержит заявки, иначе - false
     */
    bool isRequestTable(const QString& tableName) const;

    /*!
     * \brief Показывает или скрывает кнопки обработки заявок
     * \param show true - показать кнопки, false - скрыть
     */
    void showRequestButtons(bool show);
};

#endif // ADMINTABLEWIDGET_H
