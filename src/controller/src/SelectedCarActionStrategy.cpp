#include "SelectedCarActionStrategy.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDate>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlError>
#include <QSqlQuery>
#include <QVBoxLayout>

#include "AppServices.h"
#include "domain.h"

namespace {

class CheckoutSelectedCarStrategy final : public SelectedCarActionStrategy
{
public:
    bool Execute(QWidget* parent, AppServices* services, const ProductInfo& product) override
    {
        if (!services || !services->GetUserSession() || !services->GetUserSession()->IsAuthorized()) {
            QMessageBox::warning(parent, "Ошибка", "Авторизуйтесь для оформления заявки.");
            return false;
        }

        if (product.name_.isEmpty()) {
            QMessageBox::warning(parent, "Ошибка", "Не выбран автомобиль для оформления заявки.");
            return false;
        }

        QDialog dialog(parent);
        dialog.setWindowTitle("Оформление заявки на покупку");
        dialog.setMinimumSize(500, 400);
        dialog.resize(500, 500);
        dialog.setStyleSheet(
            "QDialog { background: #ffffff; }"
            "QLabel { color: #1d1b20; font: 11pt 'JetBrains Mono'; }"
            "QLabel#carInfo {"
            "  background: #f6f8fb; border: 1px solid #d9e1ea; border-radius: 10px;"
            "  padding: 12px; font: 700 11pt 'JetBrains Mono';"
            "}"
            "QLabel#sectionTitle { font: 700 12pt 'JetBrains Mono'; margin-top: 4px; }"
            "QCheckBox { font: 10.5pt 'JetBrains Mono'; spacing: 8px; }"
            "QComboBox {"
            "  padding: 7px 10px; border: 1px solid #d9e1ea; border-radius: 8px;"
            "  background: #fafbfd; min-height: 28px; font: 10.5pt 'JetBrains Mono';"
            "}"
            "QComboBox:focus { border: 1px solid #2196F3; }"
            "QPushButton {"
            "  padding: 9px 14px; border-radius: 8px; font: 600 10.5pt 'JetBrains Mono'; min-width: 120px;"
            "}"
            "QPushButton[type='primary'] { background: #2196F3; color: white; border: none; }"
            "QPushButton[type='primary']:hover { background: #1976D2; }"
            "QPushButton[type='secondary'] { background: #fafbfd; color: #1d1b20; border: 1px solid #d9e1ea; }"
            "QPushButton[type='secondary']:hover { background: #eef3f8; }");

        QVBoxLayout* layout = new QVBoxLayout(&dialog);
        layout->setSpacing(15);
        layout->setContentsMargins(30, 30, 30, 30);

        QLabel* carInfoLabel = new QLabel(QString("Автомобиль: %1\nЦвет: %2\nЦена: %3 руб.")
                                              .arg(product.name_)
                                              .arg(product.color_)
                                              .arg(FormatPrice(product.price_)),
                                          &dialog);
        carInfoLabel->setObjectName("carInfo");
        layout->addWidget(carInfoLabel);

        QLabel* optionsLabel = new QLabel("Выберите вариант оформления:", &dialog);
        optionsLabel->setObjectName("sectionTitle");
        layout->addWidget(optionsLabel);

        QCheckBox* buyCheckBox = new QCheckBox("Купить", &dialog);
        layout->addWidget(buyCheckBox);

        QCheckBox* loanCheckBox = new QCheckBox("Оформить в кредит", &dialog);
        layout->addWidget(loanCheckBox);

        QComboBox* loanTermCombo = new QComboBox(&dialog);
        loanTermCombo->addItems({"12 месяцев", "24 месяца", "36 месяцев", "48 месяцев", "60 месяцев"});
        loanTermCombo->hide();
        layout->addWidget(loanTermCombo);

        QCheckBox* insuranceCheckBox = new QCheckBox("Добавить страховку", &dialog);
        layout->addWidget(insuranceCheckBox);

        QComboBox* insuranceTypeCombo = new QComboBox(&dialog);
        insuranceTypeCombo->addItems({"ОСАГО", "КАСКО", "Комплекс"});
        insuranceTypeCombo->hide();
        layout->addWidget(insuranceTypeCombo);

        QLabel* trimLabel = new QLabel("Комплектация (если доступно):", &dialog);
        QComboBox* trimCombo = new QComboBox(&dialog);
        {
            QSqlQuery q;
            q.prepare("SELECT DISTINCT trim FROM cars WHERE name = :name AND trim IS NOT NULL AND trim <> ''");
            q.bindValue(":name", product.name_);
            if (q.exec()) {
                while (q.next()) {
                    trimCombo->addItem(q.value(0).toString());
                }
            }
            if (!product.trim_.isEmpty()) {
                const int idx = trimCombo->findText(product.trim_);
                if (idx >= 0) {
                    trimCombo->setCurrentIndex(idx);
                }
            }
        }

        if (trimCombo->count() > 0) {
            layout->addWidget(trimLabel);
            layout->addWidget(trimCombo);
        } else {
            trimLabel->hide();
            trimCombo->hide();
        }

        QCheckBox* rentalCheckBox = new QCheckBox("Взять в аренду", &dialog);
        layout->addWidget(rentalCheckBox);

        QComboBox* rentalTermCombo = new QComboBox(&dialog);
        rentalTermCombo->addItems({"1 месяц", "3 месяца", "6 месяцев", "12 месяцев"});
        rentalTermCombo->hide();
        layout->addWidget(rentalTermCombo);

        QObject::connect(loanCheckBox, &QCheckBox::toggled, loanTermCombo, &QComboBox::setVisible);
        QObject::connect(insuranceCheckBox, &QCheckBox::toggled, insuranceTypeCombo, &QComboBox::setVisible);
        QObject::connect(rentalCheckBox, &QCheckBox::toggled, rentalTermCombo, &QComboBox::setVisible);

        QObject::connect(buyCheckBox, &QCheckBox::toggled, [loanCheckBox, rentalCheckBox](bool checked) {
            if (checked) {
                loanCheckBox->setChecked(false);
                rentalCheckBox->setChecked(false);
            }
        });
        QObject::connect(loanCheckBox, &QCheckBox::toggled, [buyCheckBox, rentalCheckBox](bool checked) {
            if (checked) {
                buyCheckBox->setChecked(false);
                rentalCheckBox->setChecked(false);
            }
        });
        QObject::connect(rentalCheckBox, &QCheckBox::toggled, [buyCheckBox, loanCheckBox](bool checked) {
            if (checked) {
                buyCheckBox->setChecked(false);
                loanCheckBox->setChecked(false);
            }
        });

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        buttonLayout->setSpacing(15);
        QPushButton* okButton = new QPushButton("Подтвердить", &dialog);
        QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
        buttonLayout->addWidget(cancelButton);
        buttonLayout->addWidget(okButton);
        layout->addLayout(buttonLayout);

        bool accepted = false;
        QObject::connect(okButton, &QPushButton::clicked, [&]() {
            if (!buyCheckBox->isChecked() && !loanCheckBox->isChecked() && !insuranceCheckBox->isChecked() && !rentalCheckBox->isChecked()) {
                QMessageBox::warning(&dialog, "Ошибка", "Выберите вариант оформления.");
                return;
            }
            accepted = true;
            dialog.accept();
        });
        QObject::connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

        if (dialog.exec() != QDialog::Accepted || !accepted) {
            return false;
        }

        QSqlQuery query;
        const QString selectedTrim = trimCombo->isVisible() ? trimCombo->currentText() : QString();
        int targetCarId = product.id_;
        int targetStock = product.stock_qty_;

        if (trimCombo->isVisible() && !selectedTrim.isEmpty() && selectedTrim != product.trim_) {
            QSqlQuery find;
            find.prepare("SELECT id, stock_qty FROM cars WHERE name = :name AND trim = :trim AND color = :color ORDER BY stock_qty DESC LIMIT 1");
            find.bindValue(":name", product.name_);
            find.bindValue(":trim", selectedTrim);
            find.bindValue(":color", product.color_);
            if (find.exec() && find.next()) {
                targetCarId = find.value(0).toInt();
                targetStock = find.value(1).toInt();
            } else {
                find.finish();
                find.prepare("SELECT id, stock_qty FROM cars WHERE name = :name AND trim = :trim ORDER BY stock_qty DESC LIMIT 1");
                find.bindValue(":name", product.name_);
                find.bindValue(":trim", selectedTrim);
                if (find.exec() && find.next()) {
                    targetCarId = find.value(0).toInt();
                    targetStock = find.value(1).toInt();
                } else {
                    targetStock = 0;
                }
            }
        }

        if (buyCheckBox->isChecked()) {
            if (targetStock > 0) {
                QSqlQuery purchaseQuery;
                purchaseQuery.prepare("INSERT INTO purchase_requests (client_id, car_id, status) VALUES (:client_id, :car_id, 'не обработано')");
                purchaseQuery.bindValue(":client_id", services->GetUserSession()->GetId());
                purchaseQuery.bindValue(":car_id", targetCarId);
                if (!purchaseQuery.exec()) {
                    QMessageBox::critical(parent, "Ошибка", "Не удалось создать заявку на покупку: " + purchaseQuery.lastError().text());
                    return false;
                }
            } else {
                QSqlQuery ins;
                ins.prepare("INSERT INTO order_requests (client_id, car_name, color, trim, status) VALUES (:client_id, :car_name, :color, :trim, 'не обработано')");
                ins.bindValue(":client_id", services->GetUserSession()->GetId());
                ins.bindValue(":car_name", product.name_);
                ins.bindValue(":color", product.color_);
                ins.bindValue(":trim", selectedTrim.isEmpty() ? product.trim_ : selectedTrim);
                if (!ins.exec()) {
                    QMessageBox::critical(parent, "Ошибка", "Не удалось создать заявку на заказ: " + ins.lastError().text());
                    return false;
                }
            }
        }

        if (loanCheckBox->isChecked()) {
            if (targetStock <= 0) {
                QMessageBox::warning(parent, "Автомобиль недоступен", "Выбранный автомобиль недоступен для оформления кредита.");
                return false;
            }

            const int months = loanTermCombo->currentText().split(" ")[0].toInt();
            const QString queryStr = QString(
                "INSERT INTO loan_requests (client_id, car_id, loan_amount, loan_term_months, status) "
                "VALUES (%1, %2, %3, %4, 'не обработано');")
                                         .arg(services->GetUserSession()->GetId())
                                         .arg(targetCarId)
                                         .arg(product.price_)
                                         .arg(months);

            if (!query.exec(queryStr)) {
                QMessageBox::critical(parent, "Ошибка", "Не удалось оформить заявку на кредит: " + query.lastError().text());
                return false;
            }
        }

        if (insuranceCheckBox->isChecked()) {
            const QString queryStr = QString(
                "INSERT INTO insurance_requests (client_id, car_id, insurance_type, status) "
                "VALUES (%1, %2, '%3', 'не обработано');")
                                         .arg(services->GetUserSession()->GetId())
                                         .arg(targetCarId)
                                         .arg(insuranceTypeCombo->currentText());

            if (!query.exec(queryStr)) {
                QMessageBox::critical(parent, "Ошибка", "Не удалось оформить заявку на страхование: " + query.lastError().text());
                return false;
            }
        }

        if (rentalCheckBox->isChecked()) {
            if (targetStock <= 0) {
                QMessageBox::warning(parent, "Автомобиль недоступен", "Выбранный автомобиль недоступен для аренды.");
                return false;
            }

            const QString queryStr = QString(
                "INSERT INTO rental_requests (client_id, car_id, rental_days, start_date, status) "
                "VALUES (%1, %2, %3, '%4', 'не обработано');")
                                         .arg(services->GetUserSession()->GetId())
                                         .arg(targetCarId)
                                         .arg(rentalTermCombo->currentText().split(" ")[0].toInt())
                                         .arg(QDate::currentDate().toString("yyyy-MM-dd"));

            QString errorMessage;
            if (!services->GetDatabase()->ExecuteQueryWithUserMessage(queryStr, errorMessage)) {
                QMessageBox::warning(parent, "Ошибка", errorMessage);
                return false;
            }
        }

        QMessageBox::information(parent, "Успех", "Заявка успешно оформлена!");
        return true;
    }
};

class OrderSelectedCarStrategy final : public SelectedCarActionStrategy
{
public:
    bool Execute(QWidget* parent, AppServices* services, const ProductInfo& product) override
    {
        if (!services || !services->GetUserSession() || !services->GetUserSession()->IsAuthorized()) {
            QMessageBox::warning(parent, "Ошибка", "Авторизуйтесь для оформления заказа.");
            return false;
        }

        if (product.name_.isEmpty()) {
            QMessageBox::warning(parent, "Ошибка", "Не выбран автомобиль для заказа.");
            return false;
        }

        QDialog dialog(parent);
        dialog.setWindowTitle("Заказ автомобиля");
        dialog.setFixedSize(500, 400);

        QVBoxLayout* layout = new QVBoxLayout(&dialog);
        layout->setSpacing(15);
        layout->setContentsMargins(30, 30, 30, 30);

        QLabel* carInfoLabel = new QLabel(QString("Автомобиль: %1\nЦвет: %2\nЦена: %3 руб.")
                                              .arg(product.name_)
                                              .arg(product.color_)
                                              .arg(FormatPrice(product.price_)),
                                          &dialog);
        layout->addWidget(carInfoLabel);

        QLabel* trimLabel = new QLabel("Выберите комплектацию:", &dialog);
        layout->addWidget(trimLabel);

        QComboBox* trimCombo = new QComboBox(&dialog);
        QSqlQuery query;
        query.prepare("SELECT DISTINCT trim FROM cars WHERE name = :name AND trim IS NOT NULL AND trim <> ''");
        query.bindValue(":name", product.name_);
        if (query.exec()) {
            while (query.next()) {
                trimCombo->addItem(query.value(0).toString());
            }
        }

        if (trimCombo->count() == 0) {
            trimCombo->addItem(product.trim_.isEmpty() ? QString("Стандартная") : product.trim_);
        } else if (!product.trim_.isEmpty()) {
            const int idx = trimCombo->findText(product.trim_);
            if (idx >= 0) {
                trimCombo->setCurrentIndex(idx);
            }
        }
        layout->addWidget(trimCombo);

        QLabel* stockLabel = new QLabel("", &dialog);
        layout->addWidget(stockLabel);

        auto updateStockInfo = [=](const QString& trim) {
            QSqlQuery stockQuery;
            stockQuery.prepare("SELECT stock_qty FROM cars WHERE name = :name AND trim = :trim AND color = :color");
            stockQuery.bindValue(":name", product.name_);
            stockQuery.bindValue(":trim", trim);
            stockQuery.bindValue(":color", product.color_);
            if (stockQuery.exec() && stockQuery.next()) {
                const int stock = stockQuery.value(0).toInt();
                stockLabel->setText(stock > 0 ? QString("В наличии: %1 шт.").arg(stock) : QString("Нет в наличии - будет создана заявка на заказ"));
            } else {
                stockLabel->setText("Нет в наличии - будет создана заявка на заказ");
            }
        };
        QObject::connect(trimCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [=]() { updateStockInfo(trimCombo->currentText()); });
        updateStockInfo(trimCombo->currentText());

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        QPushButton* okButton = new QPushButton("Заказать", &dialog);
        QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
        buttonLayout->addWidget(cancelButton);
        buttonLayout->addWidget(okButton);
        layout->addLayout(buttonLayout);

        bool accepted = false;
        QObject::connect(okButton, &QPushButton::clicked, [&]() {
            accepted = true;
            dialog.accept();
        });
        QObject::connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

        if (dialog.exec() != QDialog::Accepted || !accepted) {
            return false;
        }

        const QString selectedTrim = trimCombo->currentText();
        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT id, stock_qty FROM cars WHERE name = :name AND trim = :trim AND color = :color");
        checkQuery.bindValue(":name", product.name_);
        checkQuery.bindValue(":trim", selectedTrim);
        checkQuery.bindValue(":color", product.color_);

        int targetCarId = product.id_;
        int targetStock = 0;
        if (checkQuery.exec() && checkQuery.next()) {
            targetCarId = checkQuery.value(0).toInt();
            targetStock = checkQuery.value(1).toInt();
        }

        if (targetStock > 0) {
            QSqlQuery purchaseQuery;
            purchaseQuery.prepare("INSERT INTO purchase_requests (client_id, car_id, status) VALUES (:client_id, :car_id, 'не обработано')");
            purchaseQuery.bindValue(":client_id", services->GetUserSession()->GetId());
            purchaseQuery.bindValue(":car_id", targetCarId);
            if (!purchaseQuery.exec()) {
                QMessageBox::critical(parent, "Ошибка", "Не удалось создать заявку на покупку: " + purchaseQuery.lastError().text());
                return false;
            }
        } else {
            QSqlQuery orderQuery;
            orderQuery.prepare("INSERT INTO order_requests (client_id, car_name, color, trim, status) VALUES (:client_id, :car_name, :color, :trim, 'не обработано')");
            orderQuery.bindValue(":client_id", services->GetUserSession()->GetId());
            orderQuery.bindValue(":car_name", product.name_);
            orderQuery.bindValue(":color", product.color_);
            orderQuery.bindValue(":trim", selectedTrim);
            if (!orderQuery.exec()) {
                QMessageBox::critical(parent, "Ошибка", "Не удалось создать заявку на заказ: " + orderQuery.lastError().text());
                return false;
            }
        }

        return true;
    }
};

} // namespace

std::unique_ptr<SelectedCarActionStrategy> CreateSelectedCarActionStrategy(SelectedCarAction action)
{
    switch (action) {
    case SelectedCarAction::Checkout:
        return std::make_unique<CheckoutSelectedCarStrategy>();
    case SelectedCarAction::Order:
        return std::make_unique<OrderSelectedCarStrategy>();
    default:
        return nullptr;
    }
}
