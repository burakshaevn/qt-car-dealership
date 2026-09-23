#pragma once

#ifndef ADMIN_CONTROLLER_H
#define ADMIN_CONTROLLER_H

#include <QHash>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QScopedPointer>

#include "AdminRepository.h"

class AdminPage;
class AdminTableModel;
class AppServices;

/*!
 * \brief Admin panel use cases: browse tables/views, CRUD, approve/reject requests.
 *
 * The list of tables, their captions and workflow statuses are data
 * (sys_admin_tables); nothing is hard-coded here.
 */
class AdminController final : public QObject
{
    Q_OBJECT
public:
    AdminController(AppServices& services, AdminPage* page, QObject* parent = nullptr);
    ~AdminController() override;

    [[nodiscard]] QList<AdminTableInfo> tables() const { return m_tables; }
    void open(const QString& tableName);

private:
    void reload();
    void setStatus(bool approve);
    void addRecord();
    void editRecord();
    void deleteRecord();
    [[nodiscard]] int selectedRow(const QString& action);
    [[nodiscard]] QStringList primaryKeyFields() const;

    AppServices& m_services;
    QPointer<AdminPage> m_page;
    QScopedPointer<AdminTableModel> m_model;
    QHash<QString, QString> m_labels; ///< column captions of the open table
    QList<AdminTableInfo> m_tables;
};

#endif // ADMIN_CONTROLLER_H
