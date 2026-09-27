#pragma once

#ifndef NOTIFICATIONS_CONTROLLER_H
#define NOTIFICATIONS_CONTROLLER_H

#include <QHash>
#include <QObject>
#include <QPointer>

#include "RequestRepository.h"

class AppServices;
class NotificationsPage;
struct NotificationItem;

/*!
 * \brief Feeds NotificationsPage from RequestRepository and exports contracts.
 */
class NotificationsController final : public QObject
{
    Q_OBJECT
public:
    NotificationsController(AppServices& services, NotificationsPage* page, QObject* parent = nullptr);

    void refresh();
    [[nodiscard]] int unreadCount() const;

signals:
    void unreadCountChanged(int count);

private:
    [[nodiscard]] NotificationItem present(const Notification& notification) const;
    void exportContract(const Notification& notification);
    void loadStatusLabels();

    AppServices& m_services;
    QPointer<NotificationsPage> m_page;
    QHash<QString, QString> m_statusKeys; ///< localized status value -> key (approved, rejected, ...)
};

#endif // NOTIFICATIONS_CONTROLLER_H
