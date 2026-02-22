#include "PurchaseRequestStrategy.h"

#include <QDate>
#include <QDateTime>
#include <QDialog>
#include <QMessageBox>
#include <QComboBox>
#include <QPushButton>
#include <QSqlError>
#include <QSqlQuery>
#include <QTime>

#include "AppServices.h"
#include "ThemeStyleProvider.h"
#include "ui_PurchaseRequestForm.h"
#include "ui_RentalRequestForm.h"
#include "ui_TestDriveRequestForm.h"
#include "ui_CreditRequestForm.h"

namespace {
const QString kPendingStatus = QStringLiteral(u"\u043D\u0435 \u043E\u0431\u0440\u0430\u0431\u043E\u0442\u0430\u043D\u043E");

bool FillCarCombo(QComboBox* combo)
{
    QSqlQuery carQuery("SELECT MIN(id) AS id, name FROM cars GROUP BY name ORDER BY name");
    if (!carQuery.isActive()) {
        return false;
    }

    while (carQuery.next()) {
        combo->addItem(carQuery.value("name").toString(), carQuery.value("id").toInt());
    }
    return combo->count() > 0;
}

bool EnsureAuthorized(QWidget* parent, AppServices* services)
{
    if (!services || !services->GetUserSession() || !services->GetUserSession()->IsAuthorized()) {
        QMessageBox::warning(parent, "Error", "Please sign in to submit a request.");
        return false;
    }
    return true;
}

void ApplySettingsLikeStyle(QDialog& dialog)
{
    ApplyThemeStyle(&dialog, "DialogForm");
}


template <typename UiType>
void InitDialogButtons(UiType& ui)
{
    ui.labelHeader->setProperty("type", "header");
    ui.cancelButton->setProperty("type", "secondary");
    ui.submitButton->setProperty("type", "primary");
}

class StandardPurchaseStrategy final : public PurchaseRequestStrategy
{
public:
    bool Execute(QWidget* parent, AppServices* services) override
    {
        if (!EnsureAuthorized(parent, services)) {
            return false;
        }

        QDialog dialog(parent);
        Ui::PurchaseRequestForm ui;
        ui.setupUi(&dialog);
        ApplySettingsLikeStyle(dialog);
        InitDialogButtons(ui);

        if (!FillCarCombo(ui.carCombo)) {
            QMessageBox::warning(parent, "Error", "Failed to load car list.");
            return false;
        }

        QObject::connect(ui.cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
        QObject::connect(ui.submitButton, &QPushButton::clicked, &dialog, &QDialog::accept);

        if (dialog.exec() != QDialog::Accepted) {
            return false;
        }

        QSqlQuery query;
        query.prepare(
            "INSERT INTO purchase_requests (client_id, car_id, status) "
            "VALUES (:client_id, :car_id, :status)");
        query.bindValue(":client_id", services->GetUserSession()->GetId());
        query.bindValue(":car_id", ui.carCombo->currentData().toInt());
        query.bindValue(":status", kPendingStatus);

        if (!query.exec()) {
            QMessageBox::critical(parent, "Error", "Failed to submit request: " + query.lastError().text());
            return false;
        }

        QMessageBox::information(parent, "Success", "Purchase request submitted.");
        return true;
    }
};

class RentalPurchaseStrategy final : public PurchaseRequestStrategy
{
public:
    bool Execute(QWidget* parent, AppServices* services) override
    {
        if (!EnsureAuthorized(parent, services)) {
            return false;
        }

        QDialog dialog(parent);
        Ui::RentalRequestForm ui;
        ui.setupUi(&dialog);
        ApplySettingsLikeStyle(dialog);
        InitDialogButtons(ui);

        ui.daysSpin->setRange(1, 30);
        ui.daysSpin->setSuffix(" days");
        ui.daysSpin->setValue(3);
        ui.calendar->setMinimumDate(QDate::currentDate());

        if (!FillCarCombo(ui.carCombo)) {
            QMessageBox::warning(parent, "Error", "Failed to load car list.");
            return false;
        }

        QObject::connect(ui.cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
        QObject::connect(ui.submitButton, &QPushButton::clicked, &dialog, &QDialog::accept);

        if (dialog.exec() != QDialog::Accepted) {
            return false;
        }

        QSqlQuery query;
        query.prepare(
            "INSERT INTO rental_requests (client_id, car_id, start_date, rental_days, status) "
            "VALUES (:client_id, :car_id, :start_date, :rental_days, :status)");
        query.bindValue(":client_id", services->GetUserSession()->GetId());
        query.bindValue(":car_id", ui.carCombo->currentData().toInt());
        query.bindValue(":start_date", ui.calendar->selectedDate());
        query.bindValue(":rental_days", ui.daysSpin->value());
        query.bindValue(":status", kPendingStatus);

        if (!query.exec()) {
            QMessageBox::critical(parent, "Error", "Failed to submit request: " + query.lastError().text());
            return false;
        }

        QMessageBox::information(parent, "Success", "Rental request submitted.");
        return true;
    }
};

class TestDrivePurchaseStrategy final : public PurchaseRequestStrategy
{
public:
    bool Execute(QWidget* parent, AppServices* services) override
    {
        if (!EnsureAuthorized(parent, services)) {
            return false;
        }

        QDialog dialog(parent);
        Ui::TestDriveRequestForm ui;
        ui.setupUi(&dialog);
        ApplySettingsLikeStyle(dialog);
        InitDialogButtons(ui);

        ui.calendar->setMinimumDate(QDate::currentDate());
        ui.timeEdit->setDisplayFormat("HH:mm");
        ui.timeEdit->setTime(QTime(10, 0));

        if (!FillCarCombo(ui.carCombo)) {
            QMessageBox::warning(parent, "Error", "Failed to load car list.");
            return false;
        }

        QObject::connect(ui.cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
        QObject::connect(ui.submitButton, &QPushButton::clicked, &dialog, &QDialog::accept);

        if (dialog.exec() != QDialog::Accepted) {
            return false;
        }

        const QDateTime scheduled = QDateTime(ui.calendar->selectedDate(), ui.timeEdit->time());
        QSqlQuery query;
        query.prepare(
            "INSERT INTO test_drives (client_id, car_id, scheduled_date, status) "
            "VALUES (:client_id, :car_id, :scheduled_date, :status)");
        query.bindValue(":client_id", services->GetUserSession()->GetId());
        query.bindValue(":car_id", ui.carCombo->currentData().toInt());
        query.bindValue(":scheduled_date", scheduled);
        query.bindValue(":status", kPendingStatus);

        if (!query.exec()) {
            QMessageBox::critical(parent, "Error", "Failed to submit request: " + query.lastError().text());
            return false;
        }

        QMessageBox::information(parent, "Success", "Test drive request submitted.");
        return true;
    }
};

class CreditPurchaseStrategy final : public PurchaseRequestStrategy
{
public:
    bool Execute(QWidget* parent, AppServices* services) override
    {
        if (!EnsureAuthorized(parent, services)) {
            return false;
        }

        QDialog dialog(parent);
        Ui::CreditRequestForm ui;
        ui.setupUi(&dialog);
        ApplySettingsLikeStyle(dialog);
        InitDialogButtons(ui);

        ui.amountSpin->setRange(100000, 10000000);
        ui.amountSpin->setSingleStep(50000);
        ui.amountSpin->setValue(1000000);
        ui.amountSpin->setSuffix(" rub.");

        ui.termSpin->setRange(6, 120);
        ui.termSpin->setValue(36);
        ui.termSpin->setSuffix(" months");

        if (!FillCarCombo(ui.carCombo)) {
            QMessageBox::warning(parent, "Error", "Failed to load car list.");
            return false;
        }

        QObject::connect(ui.cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
        QObject::connect(ui.submitButton, &QPushButton::clicked, &dialog, &QDialog::accept);

        if (dialog.exec() != QDialog::Accepted) {
            return false;
        }

        QSqlQuery query;
        query.prepare(
            "INSERT INTO loan_requests (client_id, car_id, loan_amount, loan_term_months, status) "
            "VALUES (:client_id, :car_id, :loan_amount, :loan_term_months, :status)");
        query.bindValue(":client_id", services->GetUserSession()->GetId());
        query.bindValue(":car_id", ui.carCombo->currentData().toInt());
        query.bindValue(":loan_amount", ui.amountSpin->value());
        query.bindValue(":loan_term_months", ui.termSpin->value());
        query.bindValue(":status", kPendingStatus);

        if (!query.exec()) {
            QMessageBox::critical(parent, "Error", "Failed to submit request: " + query.lastError().text());
            return false;
        }

        QMessageBox::information(parent, "Success", "Credit request submitted.");
        return true;
    }
};
} // namespace

std::unique_ptr<PurchaseRequestStrategy> CreatePurchaseRequestStrategy(PurchaseMethod method)
{
    switch (method) {
    case PurchaseMethod::Standart:
        return std::make_unique<StandardPurchaseStrategy>();
    case PurchaseMethod::Rental:
        return std::make_unique<RentalPurchaseStrategy>();
    case PurchaseMethod::TestDrive:
        return std::make_unique<TestDrivePurchaseStrategy>();
    case PurchaseMethod::Credit:
        return std::make_unique<CreditPurchaseStrategy>();
    case PurchaseMethod::Unknown:
    default:
        return nullptr;
    }
}
