#include "AppServices.h"

AppServices::AppServices() = default;
AppServices::~AppServices() = default;

bool AppServices::initialize(const QString& databasePath)
{
    m_database.reset(new DatabaseHandler);
    const bool kOpened = databasePath.isEmpty() ? m_database->openDefault() : m_database->open(databasePath);
    if (!kOpened) {
        m_error = m_database->lastError().text();
        if (m_error.isEmpty()) {
            m_error = QObject::tr("Не удалось открыть базу данных: %1")
                          .arg(databasePath.isEmpty() ? DatabaseHandler::defaultDatabasePath() : databasePath);
        }
        return false;
    }

    m_clients.reset(new ClientRepository(m_database));
    m_reference.reset(new ReferenceDataRepository(m_database));
    m_requests.reset(new RequestRepository(m_database));
    m_products.reset(new ProductRepository(m_database));
    m_admin.reset(new AdminRepository(m_database));
    m_error.clear();
    return true;
}

bool AppServices::isReady() const
{
    return m_database && m_database->isOpen();
}

QString AppServices::errorString() const
{
    return m_error;
}
