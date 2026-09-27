#pragma once

#ifndef REQUEST_REPOSITORY_H
#define REQUEST_REPOSITORY_H

#include <QDate>
#include <QDateTime>
#include <QList>
#include <QSharedPointer>
#include <QString>
#include <optional>

class DatabaseHandler;

/// Identifies a request regardless of which table stores it.
struct RequestRef
{
    QString SourceTable;
    int Id = 0;
};

struct Notification
{
    RequestRef Ref;
    QString Type;          ///< service, insurance, loan, test_drive, rental, purchase, order
    int CarId = 0;
    QString Status;
    QString AdditionalInfo;
    QDateTime Date;
    bool IsRead = false;
    bool IsApproved = false;
};

enum class NotificationFilter { All, Unread, Approved, LastWeek, LastMonth };

/// Data required to render a contract for an approved request.
struct ContractDetails
{
    QString Type;
    QString CarName;
    QString CarColor;
    qint64 CarPrice = 0;
    QString Trim;
    int LoanTermMonths = 0;
};

/*!
 * \brief Customer requests: creation, status workflow and notifications.
 */
class RequestRepository final
{
public:
    explicit RequestRepository(QSharedPointer<DatabaseHandler> database);

    bool createPurchase(int clientId, int carId, QString* error = nullptr);
    bool createOrder(int clientId, const QString& carName, const QString& color,
                     const QString& trim, QString* error = nullptr);
    bool createLoan(int clientId, int carId, qint64 amount, int termMonths, QString* error = nullptr);
    bool createInsurance(int clientId, int carId, const QString& insuranceType, QString* error = nullptr);
    bool createRental(int clientId, int carId, int days, const QDate& startDate, QString* error = nullptr);
    bool createTestDrive(int clientId, int carId, const QDateTime& when, QString* error = nullptr);

    bool updateStatus(const RequestRef& ref, const QString& status, QString* error = nullptr);

    [[nodiscard]] QList<Notification> notifications(int clientId, NotificationFilter filter) const;
    [[nodiscard]] int unreadCount(int clientId) const;
    bool markAllRead(int clientId);

    [[nodiscard]] std::optional<ContractDetails> contractDetails(const RequestRef& ref) const;

private:
    QSharedPointer<DatabaseHandler> m_database;
};

#endif // REQUEST_REPOSITORY_H
