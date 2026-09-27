#pragma once

#ifndef REQUEST_CONTROLLER_H
#define REQUEST_CONTROLLER_H

#include <QObject>
#include <optional>

#include "ProductRepository.h"
#include "PurchaseMethod.h"

class AppServices;
class QComboBox;
class QWidget;

/*!
 * \brief Customer request use cases (checkout, order, credit, rental, test drive).
 *
 * Each flow shows a FormDialog, validates the input and persists the request
 * through RequestRepository. Multi-step submissions run in a single transaction.
 */
class RequestController final : public QObject
{
    Q_OBJECT
public:
    explicit RequestController(AppServices& services, QObject* parent = nullptr);

    /// Checkout of a concrete car: purchase / credit / rental, optional insurance, trim choice.
    bool checkout(QWidget* parent, const ProductInfo& product);
    /// Order of a car that is not in stock.
    bool order(QWidget* parent, const ProductInfo& product);
    /// Service tile entry point; \a preselected is used when opened from a product page.
    bool request(QWidget* parent, PurchaseMethod method, const std::optional<ProductInfo>& preselected = {});

signals:
    /// Emitted after any request was stored (notification badges should refresh).
    void requestSubmitted();
    /// The user chose "purchase" on the profile page: show the catalogue.
    void catalogRequested();

private:
    bool credit(QWidget* parent, const std::optional<ProductInfo>& preselected);
    bool rental(QWidget* parent, const std::optional<ProductInfo>& preselected);
    bool testDrive(QWidget* parent, const std::optional<ProductInfo>& preselected);

    QComboBox* createCarCombo(const std::optional<ProductInfo>& preselected) const;
    QComboBox* createOptionCombo(const QString& category) const;
    void notifySuccess(QWidget* parent, const QString& message);

    AppServices& m_services;
};

#endif // REQUEST_CONTROLLER_H
