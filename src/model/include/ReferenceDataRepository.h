#pragma once

#ifndef REFERENCE_DATA_REPOSITORY_H
#define REFERENCE_DATA_REPOSITORY_H

#include <QList>
#include <QSharedPointer>
#include <QString>
#include <QVariant>

class DatabaseHandler;

struct CarType
{
    int Id = 0;
    QString Name;
};

/// A value/label pair for combo boxes (loan terms, insurance kinds, ...).
struct ChoiceOption
{
    QVariant Value;
    QString Label;
};

/*!
 * \brief Dictionaries and configuration stored in the database
 *        (car types, form choices, default values).
 */
class ReferenceDataRepository final
{
public:
    /// Categories of sys_options.
    struct OptionCategory {
        static inline const QString kLoanTermMonths = QStringLiteral("loan_term_months");
        static inline const QString kRentalDays = QStringLiteral("rental_days");
        static inline const QString kInsuranceType = QStringLiteral("insurance_type");
    };

    explicit ReferenceDataRepository(QSharedPointer<DatabaseHandler> database);

    [[nodiscard]] QList<CarType> carTypes() const;
    [[nodiscard]] QString defaultCatalogColor() const;
    [[nodiscard]] QList<ChoiceOption> options(const QString& category) const;
    [[nodiscard]] QString defaultTrim() const;

private:
    QSharedPointer<DatabaseHandler> m_database;
};

#endif // REFERENCE_DATA_REPOSITORY_H
