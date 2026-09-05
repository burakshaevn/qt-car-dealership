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
#include "ThemeStyleProvider.h"
#include "PriceFormatter.h"

namespace {

class CheckoutSelectedCarStrategy final : public SelectedCarActionStrategy
{
public:
    bool execute(QWidget* parent, AppServices* services, const ProductInfo& product) override
    {
        if (!services || !services->getUserSession()
            || !services->getUserSession()->isAuthorized()) {
            QMessageBox::warning(parent, "Ошибка", "Авторизуйтесь для оформления заявки.");
            return false;
        }

        if (product.Name.isEmpty()) {
            QMessageBox::warning(parent, "Ошибка", "Не выбран автомобиль для оформления заявки.");
            return false;
        }

        QDialog dialog(parent);
        dialog.setWindowTitle("Оформление заявки на покупку");
        dialog.setMinimumSize(500, 400);
        dialog.resize(500, 500);
        applyThemeStyle(&dialog, "DialogForm");

        QVBoxLayout* layout = new QVBoxLayout(&dialog);
        layout->setSpacing(15);
        layout->setContentsMargins(30, 30, 30, 30);

        QLabel* carInfoLabel = new QLabel(QString("Автомобиль: %1\nЦвет: %2\nЦена: %3 руб.")
                                              .arg(product.Name)
                                              .arg(product.Color)
                                              .arg(formatPrice(product.Price)),
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
            q.bindValue(":name", product.Name);
            if (q.exec()) {
                while (q.next()) {
                    trimCombo->addItem(q.value(0).toString());
                }
            }
            if (!product.Trim.isEmpty()) {
                const int kIdx = trimCombo->findText(product.Trim);
                if (kIdx >= 0) {
                    trimCombo->setCurrentIndex(kIdx);
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
        okButton->setProperty("type", "primary");
        cancelButton->setProperty("type", "secondary");
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
        const QString kSelectedTrim = trimCombo->isVisible() ? trimCombo->currentText() : QString();
        int targetCarId = product.Id;
        int targetStock = product.StockQty;

        if (trimCombo->isVisible() && !kSelectedTrim.isEmpty() && kSelectedTrim != product.Trim) {
            QSqlQuery find;
            find.prepare("SELECT id, stock_qty FROM cars WHERE name = :name AND trim = :trim AND color = :color ORDER BY stock_qty DESC LIMIT 1");
            find.bindValue(":name", product.Name);
            find.bindValue(":trim", kSelectedTrim);
            find.bindValue(":color", product.Color);
            if (find.exec() && find.next()) {
                targetCarId = find.value(0).toInt();
                targetStock = find.value(1).toInt();
            } else {
                find.finish();
                find.prepare("SELECT id, stock_qty FROM cars WHERE name = :name AND trim = :trim ORDER BY stock_qty DESC LIMIT 1");
                find.bindValue(":name", product.Name);
                find.bindValue(":trim", kSelectedTrim);
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
                purchaseQuery.bindValue(":client_id", services->getUserSession()->getId());
                purchaseQuery.bindValue(":car_id", targetCarId);
                if (!purchaseQuery.exec()) {
                    QMessageBox::critical(parent, "Ошибка", "Не удалось создать заявку на покупку: " + purchaseQuery.lastError().text());
                    return false;
                }
            } else {
                QSqlQuery ins;
                ins.prepare("INSERT INTO order_requests (client_id, car_name, color, trim, status) VALUES (:client_id, :car_name, :color, :trim, 'не обработано')");
                ins.bindValue(":client_id", services->getUserSession()->getId());
                ins.bindValue(":car_name", product.Name);
                ins.bindValue(":color", product.Color);
                ins.bindValue(":trim", kSelectedTrim.isEmpty() ? product.Trim : kSelectedTrim);
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

            const int kMonths = loanTermCombo->currentText().split(" ")[0].toInt();
            const QString kQueryStr = QString("INSERT INTO loan_requests (client_id, car_id, "
                                              "loan_amount, loan_term_months, status) "
                                              "VALUES (%1, %2, %3, %4, 'не обработано');")
                                          .arg(services->getUserSession()->getId())
                                          .arg(targetCarId)
                                          .arg(product.Price)
                                          .arg(kMonths);

            if (!query.exec(kQueryStr)) {
                QMessageBox::critical(parent, "Ошибка", "Не удалось оформить заявку на кредит: " + query.lastError().text());
                return false;
            }
        }

        if (insuranceCheckBox->isChecked()) {
            const QString kQueryStr
                = QString(
                      "INSERT INTO insurance_requests (client_id, car_id, insurance_type, status) "
                      "VALUES (%1, %2, '%3', 'не обработано');")
                      .arg(services->getUserSession()->getId())
                      .arg(targetCarId)
                      .arg(insuranceTypeCombo->currentText());

            if (!query.exec(kQueryStr)) {
                QMessageBox::critical(parent, "Ошибка", "Не удалось оформить заявку на страхование: " + query.lastError().text());
                return false;
            }
        }

        if (rentalCheckBox->isChecked()) {
            if (targetStock <= 0) {
                QMessageBox::warning(parent, "Автомобиль недоступен", "Выбранный автомобиль недоступен для аренды.");
                return false;
            }

            const QString kQueryStr = QString("INSERT INTO rental_requests (client_id, car_id, "
                                              "rental_days, start_date, status) "
                                              "VALUES (%1, %2, %3, '%4', 'не обработано');")
                                          .arg(services->getUserSession()->getId())
                                          .arg(targetCarId)
                                          .arg(rentalTermCombo->currentText().split(" ")[0].toInt())
                                          .arg(QDate::currentDate().toString("yyyy-MM-dd"));

            QString errorMessage;
            if (!services->getDatabase()->executeQueryWithUserMessage(kQueryStr, errorMessage)) {
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
    bool execute(QWidget* parent, AppServices* services, const ProductInfo& product) override
    {
        if (!services || !services->getUserSession()
            || !services->getUserSession()->isAuthorized()) {
            QMessageBox::warning(parent, "Ошибка", "Авторизуйтесь для оформления заказа.");
            return false;
        }

        if (product.Name.isEmpty()) {
            QMessageBox::warning(parent, "Ошибка", "Не выбран автомобиль для заказа.");
            return false;
        }

        QDialog dialog(parent);
        dialog.setWindowTitle("Заказ автомобиля");
        dialog.setFixedSize(500, 400);
        applyThemeStyle(&dialog, "DialogForm");

        QVBoxLayout* layout = new QVBoxLayout(&dialog);
        layout->setSpacing(15);
        layout->setContentsMargins(30, 30, 30, 30);

        QLabel* carInfoLabel = new QLabel(QString("Автомобиль: %1\nЦвет: %2\nЦена: %3 руб.")
                                              .arg(product.Name)
                                              .arg(product.Color)
                                              .arg(formatPrice(product.Price)),
                                          &dialog);
        carInfoLabel->setObjectName("carInfo");
        layout->addWidget(carInfoLabel);

        QLabel* trimLabel = new QLabel("Выберите комплектацию:", &dialog);
        trimLabel->setObjectName("sectionTitle");
        layout->addWidget(trimLabel);

        QComboBox* trimCombo = new QComboBox(&dialog);
        QSqlQuery query;
        query.prepare("SELECT DISTINCT trim FROM cars WHERE name = :name AND trim IS NOT NULL AND trim <> ''");
        query.bindValue(":name", product.Name);
        if (query.exec()) {
            while (query.next()) {
                trimCombo->addItem(query.value(0).toString());
            }
        }

        if (trimCombo->count() == 0) {
            trimCombo->addItem(product.Trim.isEmpty() ? QString("Стандартная") : product.Trim);
        } else if (!product.Trim.isEmpty()) {
            const int kIdx = trimCombo->findText(product.Trim);
            if (kIdx >= 0) {
                trimCombo->setCurrentIndex(kIdx);
            }
        }
        layout->addWidget(trimCombo);

        QLabel* stockLabel = new QLabel("", &dialog);
        layout->addWidget(stockLabel);

        auto updateStockInfo = [=](const QString& trim) {
            QSqlQuery stockQuery;
            stockQuery.prepare("SELECT stock_qty FROM cars WHERE name = :name AND trim = :trim AND color = :color");
            stockQuery.bindValue(":name", product.Name);
            stockQuery.bindValue(":trim", trim);
            stockQuery.bindValue(":color", product.Color);
            if (stockQuery.exec() && stockQuery.next()) {
                const int kStock = stockQuery.value(0).toInt();
                stockLabel->setText(kStock > 0 ? QString("В наличии: %1 шт.").arg(kStock) : QString("Нет в наличии - будет создана заявка на заказ"));
            } else {
                stockLabel->setText("Нет в наличии - будет создана заявка на заказ");
            }
        };
        QObject::connect(trimCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [=]() { updateStockInfo(trimCombo->currentText()); });
        updateStockInfo(trimCombo->currentText());

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        QPushButton* okButton = new QPushButton("Заказать", &dialog);
        QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
        okButton->setProperty("type", "primary");
        cancelButton->setProperty("type", "secondary");
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

        const QString kSelectedTrim = trimCombo->currentText();
        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT id, stock_qty FROM cars WHERE name = :name AND trim = :trim AND color = :color");
        checkQuery.bindValue(":name", product.Name);
        checkQuery.bindValue(":trim", kSelectedTrim);
        checkQuery.bindValue(":color", product.Color);

        int targetCarId = product.Id;
        int targetStock = 0;
        if (checkQuery.exec() && checkQuery.next()) {
            targetCarId = checkQuery.value(0).toInt();
            targetStock = checkQuery.value(1).toInt();
        }

        if (targetStock > 0) {
            QSqlQuery purchaseQuery;
            purchaseQuery.prepare("INSERT INTO purchase_requests (client_id, car_id, status) VALUES (:client_id, :car_id, 'не обработано')");
            purchaseQuery.bindValue(":client_id", services->getUserSession()->getId());
            purchaseQuery.bindValue(":car_id", targetCarId);
            if (!purchaseQuery.exec()) {
                QMessageBox::critical(parent, "Ошибка", "Не удалось создать заявку на покупку: " + purchaseQuery.lastError().text());
                return false;
            }
        } else {
            QSqlQuery orderQuery;
            orderQuery.prepare("INSERT INTO order_requests (client_id, car_name, color, trim, status) VALUES (:client_id, :car_name, :color, :trim, 'не обработано')");
            orderQuery.bindValue(":client_id", services->getUserSession()->getId());
            orderQuery.bindValue(":car_name", product.Name);
            orderQuery.bindValue(":color", product.Color);
            orderQuery.bindValue(":trim", kSelectedTrim);
            if (!orderQuery.exec()) {
                QMessageBox::critical(parent, "Ошибка", "Не удалось создать заявку на заказ: " + orderQuery.lastError().text());
                return false;
            }
        }

        return true;
    }
};

} // namespace

std::unique_ptr<SelectedCarActionStrategy> createSelectedCarActionStrategy(SelectedCarAction action)
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
