#pragma once

#ifndef NOTIFICATIONS_CONTROLLER_H
#define NOTIFICATIONS_CONTROLLER_H

#include <QObject>
#include <QScopedPointer>
#include <QSharedPointer>

#include "notifications_handler.h"

class DatabaseHandler;
class QWidget;

/*!
 * \brief Класс, предоставляющий сервисы для работы с уведомлениями
 */
class NotificationsController : public QObject
{
    Q_OBJECT
public:
    explicit NotificationsController(QObject* parent = nullptr);

    /*!
     * \brief Устанавливает зависимости
     * \param database - указатель на объект DatabaseHandler
     */
    void SetDependencies(const QSharedPointer<DatabaseHandler>& database);
    /*!
     * \brief Отображает уведомления для пользователя
     * \param userId - ID пользователя
     * \param parent - владелец диалога уведомлений
     */
    void ShowForUser(int userId, QWidget* parent);

private:
    QSharedPointer<DatabaseHandler> database_;
    QScopedPointer<NotificationsHandler> handler_;
};

#endif // NOTIFICATIONS_CONTROLLER_H
