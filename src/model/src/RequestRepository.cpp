#include "RequestRepository.h"

#include "DatabaseHandler.h"

namespace Q = SqlQuery;

namespace {
// Parameter values understood by notifications/select_for_client.sql.
QString filterKey(const NotificationFilter filter)
{
    switch (filter) {
    case NotificationFilter::Unread: return QStringLiteral("unread");
    case NotificationFilter::Approved: return QStringLiteral("approved");
    case NotificationFilter::LastWeek: return QStringLiteral("week");
    case NotificationFilter::LastMonth: return QStringLiteral("month");
    case NotificationFilter::All: break;
    }
    return QStringLiteral("all");
}

// SQLite stores timestamps as ISO text; Qt accepts both "T" and space separators.
QDateTime parseDateTime(const QVariant& value)
{
    QDateTime result = QDateTime::fromString(value.toString(), Qt::ISODate);
    if (!result.isValid()) {
        result = value.toDateTime();
    }
    return result;
}
} // namespace

RequestRepository::RequestRepository(QSharedPointer<DatabaseHandler> database)
    : m_database(std::move(database))
{}

bool RequestRepository::createPurchase(const int clientId, const int carId, QString* error)
{
    return m_database->execute(Q::Requests::kInsertPurchase, {{"client_id", clientId}, {"car_id", carId}}, error);
}

bool RequestRepository::createOrder(const int clientId,
                                    const QString& carName,
                                    const QString& color,
                                    const QString& trim,
                                    QString* error)
{
    return m_database->execute(Q::Requests::kInsertOrder,
                               {{"client_id", clientId},
                                {"car_name", carName},
                                {"color", color},
                                {"trim", trim}},
                               error);
}

bool RequestRepository::createLoan(const int clientId,
                                   const int carId,
                                   const qint64 amount,
                                   const int termMonths,
                                   QString* error)
{
    return m_database->execute(Q::Requests::kInsertLoan,
                               {{"client_id", clientId},
                                {"car_id", carId},
                                {"loan_amount", amount},
                                {"loan_term_months", termMonths}},
                               error);
}

bool RequestRepository::createInsurance(const int clientId,
                                        const int carId,
                                        const QString& insuranceType,
                                        QString* error)
{
    return m_database->execute(Q::Requests::kInsertInsurance,
                               {{"client_id", clientId},
                                {"car_id", carId},
                                {"insurance_type", insuranceType}},
                               error);
}

bool RequestRepository::createRental(const int clientId,
                                     const int carId,
                                     const int days,
                                     const QDate& startDate,
                                     QString* error)
{
    return m_database->execute(Q::Requests::kInsertRental,
                               {{"client_id", clientId},
                                {"car_id", carId},
                                {"rental_days", days},
                                {"start_date", startDate.toString(Qt::ISODate)}},
                               error);
}

bool RequestRepository::createTestDrive(const int clientId,
                                        const int carId,
                                        const QDateTime& when,
                                        QString* error)
{
    return m_database->execute(Q::Requests::kInsertTestDrive,
                               {{"client_id", clientId},
                                {"car_id", carId},
                                {"scheduled_date", when.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))}},
                               error);
}

bool RequestRepository::updateStatus(const RequestRef& ref, const QString& status, QString* error)
{
    return m_database->execute(Q::Requests::kUpdateStatus,
                               {{"source_table", ref.SourceTable}, {"id", ref.Id}, {"status", status}},
                               error);
}

QList<Notification> RequestRepository::notifications(const int clientId, const NotificationFilter filter) const
{
    QList<Notification> result;
    const auto kRows = m_database->rows(Q::Notifications::kSelectForClient,
                                        {{"client_id", clientId}, {"filter", filterKey(filter)}});
    result.reserve(kRows.size());
    for (const auto& row : kRows) {
        Notification n;
        n.Ref = {row.value("source_table").toString(), row.value("id").toInt()};
        n.Type = row.value("request_type").toString();
        n.CarId = row.value("car_id").toInt();
        n.Status = row.value("status").toString();
        n.AdditionalInfo = row.value("additional_info").toString();
        n.Date = parseDateTime(row.value("date_info"));
        n.IsRead = row.value("notification_shown").toBool();
        n.IsApproved = row.value("is_approved").toBool();
        result.append(n);
    }
    return result;
}

int RequestRepository::unreadCount(const int clientId) const
{
    return m_database->scalar(Q::Notifications::kCountUnread, {{"client_id", clientId}}, 0).toInt();
}

bool RequestRepository::markAllRead(const int clientId)
{
    return m_database->execute(Q::Notifications::kMarkAllShown, {{"client_id", clientId}});
}

std::optional<ContractDetails> RequestRepository::contractDetails(const RequestRef& ref) const
{
    const auto kRows = m_database->rows(Q::Contracts::kSelectRequestDetails,
                                        {{"source_table", ref.SourceTable}, {"id", ref.Id}});
    if (kRows.isEmpty()) {
        return std::nullopt;
    }
    const QVariantMap& row = kRows.first();
    ContractDetails details;
    details.Type = row.value("request_type").toString();
    details.CarName = row.value("car_name").toString();
    details.CarColor = row.value("car_color").toString();
    details.CarPrice = row.value("car_price").toLongLong();
    details.Trim = row.value("trim").toString();
    details.LoanTermMonths = row.value("loan_term_months").toInt();
    return details;
}
