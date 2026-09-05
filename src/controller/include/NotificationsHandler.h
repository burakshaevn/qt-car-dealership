#ifndef NOTIFICATIONSHANDLER_H
#define NOTIFICATIONSHANDLER_H

#include <QDialog>
#include <QVBoxLayout>
#include <QString>
#include <QStringView>
#include <QResizeEvent>
#include <QVariant>

#include "DatabaseHandler.h"

namespace Ui {
class notifications;
}

class NotificationsHandler : public QDialog
{
    Q_OBJECT

public:
    explicit NotificationsHandler(QSharedPointer<DatabaseHandler> databaseHandler, QWidget *parent = nullptr);
    ~NotificationsHandler();

    /*!
     * \brief Получает и отображает уведомления
     * \param user_id Для какого пользователя выполняется действие
     */
    void loadAndShowNotifications(const int kUserId);

    /*!
     * \brief Возвращает новые уведомления, если они есть
     * \param user_id Для какого пользователя выполняется поиск новых уведомлений
     * \return
     */
    QVariant getNewNotifications(const int kUserId);

    /*!
     * \brief Отмечает все уведомления как прочитанные
     * \param user_id Для какого пользователя выполняется действие
     */
    void markNotificationsAsReaded(const int kUserId);

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

    /*!
     * \brief onFilterChanged - обработчик изменения фильтра
     */
    void onFilterChanged();

    /*!
     * \brief onMarkAllReadClicked - обработчик кнопки "Прочитать все"
     */
    void onMarkAllReadClicked();

private:
    Ui::notifications* m_ui;

    QVBoxLayout* m_notificationsLayout;

    QWeakPointer<DatabaseHandler> m_databaseHandler;       ///< Предоставляет интерфейс для работы с БД

    bool m_isSortedAscending;                             ///< Направление для сортировки по дате
    int m_currentUserId;                                  ///< ID текущего пользователя
    QString m_currentFilter;                               ///< Текущий фильтр

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
    void sortNotifications(const bool kAscending);
};

#endif // NOTIFICATIONSHANDLER_H
