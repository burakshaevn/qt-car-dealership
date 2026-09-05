#include "NotificationsController.h"

#include "NotificationsHandler.h"
#include "DatabaseHandler.h"

NotificationsController::NotificationsController(QObject* parent)
    : QObject(parent)
{
}

void NotificationsController::setDependencies(const QSharedPointer<DatabaseHandler>& database)
{
    m_database = database;
}

void NotificationsController::showForUser(int userId, QWidget* parent)
{
    if (!m_database) {
        return;
    }
    if (!m_handler) {
        m_handler.reset(new NotificationsHandler(m_database, parent));
    }
    m_handler->loadAndShowNotifications(userId);
    m_handler->exec();
}
