#pragma once

#ifndef CONTRACT_TEMPLATES_H
#define CONTRACT_TEMPLATES_H

#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QObject>

namespace ContractTemplates {

QString getLoanContractHtml(const QString& currentDate, const QString& carName, const QString& carColor, 
                          const QString& carPrice, const QString& loanAmount, const QString& loanTerm);

QString getRentalContractHtml(const QString& currentDate, const QString& carName, const QString& carColor, 
                            const QString& rentalDays, const QString& startDate);

QString getInsuranceContractHtml(const QString& currentDate, const QString& carName, const QString& carColor, 
                                const QString& insuranceType);

QString getServiceContractHtml(const QString& currentDate, const QString& carName, const QString& carColor, const QString& serviceType, const QString& scheduledDate);

} // namespace ContractTemplates

#endif // CONTRACT_TEMPLATES_H 