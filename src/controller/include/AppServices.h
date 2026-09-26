#pragma once

#ifndef APP_SERVICES_H
#define APP_SERVICES_H

#include <QScopedPointer>
#include <QSharedPointer>

#include "AdminRepository.h"
#include "ClientRepository.h"
#include "DatabaseHandler.h"
#include "ProductRepository.h"
#include "ReferenceDataRepository.h"
#include "RequestRepository.h"
#include "UserSession.h"

/*!
 * \brief Composition root: owns the database connection, repositories and the session.
 *
 * Controllers receive this object and never create database objects themselves.
 */
class AppServices final
{
public:
    AppServices();
    ~AppServices();

    AppServices(const AppServices&) = delete;
    AppServices& operator=(const AppServices&) = delete;

    /// Opens \a databasePath (or the default location when empty) and builds repositories.
    bool initialize(const QString& databasePath = {});
    [[nodiscard]] bool isReady() const;
    [[nodiscard]] QString errorString() const;

    [[nodiscard]] QSharedPointer<DatabaseHandler> database() const { return m_database; }
    [[nodiscard]] ClientRepository& clients() { return *m_clients; }
    [[nodiscard]] ReferenceDataRepository& reference() { return *m_reference; }
    [[nodiscard]] RequestRepository& requests() { return *m_requests; }
    [[nodiscard]] ProductRepository& products() { return *m_products; }
    [[nodiscard]] AdminRepository& admin() { return *m_admin; }

    [[nodiscard]] UserSession& session() { return m_session; }
    [[nodiscard]] const UserSession& session() const { return m_session; }

private:
    QSharedPointer<DatabaseHandler> m_database;
    QScopedPointer<ClientRepository> m_clients;
    QScopedPointer<ReferenceDataRepository> m_reference;
    QScopedPointer<RequestRepository> m_requests;
    QScopedPointer<ProductRepository> m_products;
    QScopedPointer<AdminRepository> m_admin;
    UserSession m_session;
    QString m_error;
};

#endif // APP_SERVICES_H
