#include "AdminTableController.h"

#include <QStackedWidget>

AdminTableController::AdminTableController(QObject* parent)
    : QObject(parent)
{
}

void AdminTableController::setDependencies(const QSharedPointer<DatabaseHandler>& database)
{
    m_database = database;
}

void AdminTableController::ensureView(QWidget* owner)
{
    if (!m_tableView) {
        m_tableView.reset(new AdminTableWidget(m_database, owner));
        connect(m_tableView.get(),
                &AdminTableWidget::logout,
                this,
                &AdminTableController::logoutRequested);
    }

    if (!m_isInitialized) {
        m_tableView->buildAdminTables();
        m_isInitialized = true;
    }
}

void AdminTableController::show(QStackedWidget* stackedWidget, QWidget* owner)
{
    if (!stackedWidget || !m_database) {
        return;
    }

    ensureView(owner);

    if (stackedWidget->indexOf(m_tableView.get()) == -1) {
        stackedWidget->addWidget(m_tableView.get());
    }

    stackedWidget->setCurrentWidget(m_tableView.get());
}

void AdminTableController::reset()
{
    m_tableView.reset();
    m_isInitialized = false;
}
