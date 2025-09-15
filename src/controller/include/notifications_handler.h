#ifndef NOTIFICATIONS_HANDLER_H
#define NOTIFICATIONS_HANDLER_H

#include <QDialog>
#include <QVBoxLayout>

#include "database_handler.h"

namespace Ui {
class notifications;
}

class NotificationsHandler : public QDialog
{
    Q_OBJECT

public:
    explicit NotificationsHandler(QSharedPointer<DatabaseHandler> database_handler, QWidget *parent = nullptr);
    ~NotificationsHandler();

    /*!
     * \brief Получает и отображает уведомления
     * \param user_id Для какого пользователя выполняется действие
     */
    void loadAndShowNotifications(const int user_id);

    /*!
     * \brief Возвращает новые уведомления, если они есть
     * \param user_id Для какого пользователя выполняется поиск новых уведомлений
     * \return
     */
    QVariant getNewNotifications(const int user_id);

    /*!
     * \brief Отмечает все уведомления как прочитанные
     * \param user_id Для какого пользователя выполняется действие
     */
    void markNotificationsAsReaded(const int user_id);

    /*!
     * \brief Очищает ScrollArea от всех уведомлений
     */
    void clear();

protected:
    /*!
     * \brief Обработчик события изменения размера окна
     * \param event Собращение изменения размера
     */
    void resizeEvent(QResizeEvent *event) override;

private slots:
    /*!
     * \brief onSortButtonClicked
     */
    void onSortButtonClicked();

private:
    Ui::notifications *ui;

    QVBoxLayout* m_notifications_layout;

    QWeakPointer<DatabaseHandler> m_database_handler;       ///< Предоставляет интерфейс для работы с БД

    bool m_is_sorted_ascending;                             ///< Направление для сортировки по дате

    /*!
     * \brief Добавляет новое уведомление в ScrollArea
     * \param title Заголовок уведомления
     * \param date Дата уведомления
     * \param text Текст уведомления
     */
    void addNotification(const QStringView title, const QStringView date, const QStringView text);

    /*!
     * \brief Генерирует договор на основе уведомления
     * \param type Тип уведомления (service, insurance, loan, rental, test_drive)
     * \param requestId ID заявки в соответствующей таблице
     * \param carId ID автомобиля
     * \param additionalInfo Дополнительная информация (тип услуги, сумма и т.д.)
     * \param dateInfo Информация о дате
     */
    void generateContractFromNotification(const QString& type, int requestId, int carId,
                                          const QString& additionalInfo, const QString& dateInfo);

    /*!
     * \brief Сортирует уведомления по дате
     * \param ascending - направление сортировки
     */
    void sortNotifications(const bool ascending);
};

#endif // NOTIFICATIONS_HANDLER_H
