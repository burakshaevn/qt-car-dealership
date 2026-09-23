#include "ReferenceDataRepository.h"

#include "DatabaseHandler.h"

namespace Q = SqlQuery;

ReferenceDataRepository::ReferenceDataRepository(QSharedPointer<DatabaseHandler> database)
    : m_database(std::move(database))
{}

QList<CarType> ReferenceDataRepository::carTypes() const
{
    QList<CarType> result;
    if (!m_database) {
        return result;
    }
    const auto kRows = m_database->rows(Q::Reference::kSelectCarTypes);
    for (const auto& row : kRows) {
        result.append({row.value("id").toInt(), row.value("name").toString()});
    }
    return result;
}

QString ReferenceDataRepository::defaultCatalogColor() const
{
    return m_database ? m_database->scalar(Q::Reference::kSelectDefaultCatalogColor).toString() : QString();
}

QList<ChoiceOption> ReferenceDataRepository::options(const QString& category) const
{
    QList<ChoiceOption> result;
    if (!m_database) {
        return result;
    }
    const auto kRows = m_database->rows(Q::System::kSelectOptions, {{"category", category}});
    for (const auto& row : kRows) {
        result.append({row.value("value"), row.value("label").toString()});
    }
    return result;
}

QString ReferenceDataRepository::defaultTrim() const
{
    return m_database ? m_database->string(QStringLiteral("defaults"), QStringLiteral("trim")) : QString();
}
