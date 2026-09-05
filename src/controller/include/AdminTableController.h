#pragma once

#ifndef ADMINTABLECONTROLLER_H
#define ADMINTABLECONTROLLER_H

#include "AdminTableWidget.h"
#include <QObject>
#include <QScopedPointer>
#include <QSharedPointer>

class QWidget;
class QStackedWidget;
class DatabaseHandler;

/*!
 * \brief Controller for admin table
 */
class AdminTableController final : public QObject
{
    Q_OBJECT

public:
    explicit AdminTableController(QObject* parent = nullptr);

    /*!
     * \brief Set dependencies
     * \param database Database handler
     */
    void setDependencies(const QSharedPointer<DatabaseHandler>& database);

    /*!
     * \brief Show admin table
     * \param stacked_widget Stacked widget
     * \param owner Owner widget
     */
    void show(QStackedWidget* stackedWidget, QWidget* owner);

    /*!
     * \brief Reset admin table
     */
    void reset();

signals:
    void logoutRequested();

private:
    void ensureView(QWidget* owner);

private:
    QSharedPointer<DatabaseHandler> m_database;
    QScopedPointer<AdminTableWidget> m_tableView;
    bool m_isInitialized = false; ///< Flag to check if the controller is initialized
};

#endif // ADMINTABLECONTROLLER_H
