#include "RequestController.h"

#include "AppServices.h"
#include "PriceFormatter.h"
#include "UiKit.h"

#include <QButtonGroup>
#include <QCalendarWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QRadioButton>
#include <QTimeEdit>
#include <QVBoxLayout>

namespace {

enum class PaymentKind { Purchase, Credit, Rental };

/// Compact summary card of the selected car shown at the top of forms.
QWidget* carSummary(const ProductInfo& product, QWidget* parent)
{
    auto* card = UiKit::card(parent);
    auto* layout = new QHBoxLayout(card);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(14);

    auto* image = new QLabel(card);
    image->setFixedSize(96, 56);
    image->setAlignment(Qt::AlignCenter);
    const QPixmap kPixmap(product.ImagePath);
    if (!kPixmap.isNull()) {
        QPixmap scaled = kPixmap.scaled(image->size() * image->devicePixelRatioF(), Qt::KeepAspectRatio,
                                        Qt::SmoothTransformation);
        scaled.setDevicePixelRatio(image->devicePixelRatioF());
        image->setPixmap(scaled);
    }
    layout->addWidget(image);

    auto* text = new QVBoxLayout;
    text->setSpacing(2);
    text->addWidget(UiKit::label(product.Name, "h3", card));
    QString details = product.Color;
    if (!product.Trim.isEmpty()) {
        details += QStringLiteral("  ·  ") + product.Trim;
    }
    text->addWidget(UiKit::label(details, "muted", card));
    layout->addLayout(text, 1);
    layout->addWidget(UiKit::label(formatPrice(product.Price) + QStringLiteral(" ₽"), "h3", card));
    return card;
}

} // namespace

RequestController::RequestController(AppServices& services, QObject* parent)
    : QObject(parent)
    , m_services(services)
{}

void RequestController::notifySuccess(QWidget* parent, const QString& message)
{
    emit requestSubmitted();
    QMessageBox::information(parent, tr("Заявка отправлена"), message);
}

QComboBox* RequestController::createCarCombo(const std::optional<ProductInfo>& preselected) const
{
    auto* combo = new QComboBox;
    const QList<CarModel> kModels = m_services.products().models();
    for (const CarModel& model : kModels) {
        combo->addItem(model.Name, model.Id);
    }
    if (preselected) {
        // Keep the exact car (colour/trim) the user was looking at.
        const int kIndex = combo->findText(preselected->Name);
        if (kIndex >= 0) {
            combo->setItemData(kIndex, preselected->Id);
            combo->setCurrentIndex(kIndex);
        }
    }
    return combo;
}

QComboBox* RequestController::createOptionCombo(const QString& category) const
{
    auto* combo = new QComboBox;
    const QList<ChoiceOption> kOptions = m_services.reference().options(category);
    for (const ChoiceOption& option : kOptions) {
        combo->addItem(option.Label, option.Value);
    }
    return combo;
}

bool RequestController::request(QWidget* parent, const PurchaseMethod method,
                                const std::optional<ProductInfo>& preselected)
{
    switch (method) {
    case PurchaseMethod::Standart:
        emit catalogRequested();
        return false;
    case PurchaseMethod::Credit:
        return credit(parent, preselected);
    case PurchaseMethod::Rental:
        return rental(parent, preselected);
    case PurchaseMethod::TestDrive:
        return testDrive(parent, preselected);
    case PurchaseMethod::Unknown:
        break;
    }
    return false;
}

bool RequestController::checkout(QWidget* parent, const ProductInfo& product)
{
    if (!m_services.session().isUser() || product.Name.isEmpty()) {
        return false;
    }

    FormDialog dialog(tr("Оформление заявки"), tr("Выберите способ приобретения и дополнительные услуги."), parent);
    dialog.setAcceptText(tr("Отправить заявку"));
    dialog.addWidget(carSummary(product, &dialog));

    // Trim
    auto* trim = new QComboBox;
    const QStringList kTrims = m_services.products().trimsOf(product.Name);
    trim->addItems(kTrims);
    trim->setCurrentText(product.Trim);
    if (kTrims.size() > 1) {
        dialog.addField(tr("Комплектация"), trim);
    } else {
        trim->deleteLater();
        trim = nullptr;
    }

    // Payment kind
    auto* paymentBox = new QWidget;
    auto* paymentLayout = new QVBoxLayout(paymentBox);
    paymentLayout->setContentsMargins(0, 0, 0, 0);
    paymentLayout->setSpacing(10);
    auto* payment = new QButtonGroup(paymentBox);
    auto* purchase = new QRadioButton(tr("Покупка — полная оплата"));
    auto* credit = new QRadioButton(tr("Кредит"));
    auto* rental = new QRadioButton(tr("Аренда"));
    payment->addButton(purchase, static_cast<int>(PaymentKind::Purchase));
    payment->addButton(credit, static_cast<int>(PaymentKind::Credit));
    payment->addButton(rental, static_cast<int>(PaymentKind::Rental));
    purchase->setChecked(true);

    auto* loanTerm = createOptionCombo(ReferenceDataRepository::OptionCategory::kLoanTermMonths);
    auto* rentalTerm = createOptionCombo(ReferenceDataRepository::OptionCategory::kRentalDays);
    loanTerm->setVisible(false);
    rentalTerm->setVisible(false);

    paymentLayout->addWidget(purchase);
    paymentLayout->addWidget(credit);
    paymentLayout->addWidget(loanTerm);
    paymentLayout->addWidget(rental);
    paymentLayout->addWidget(rentalTerm);
    dialog.addField(tr("Способ"), paymentBox);

    // Insurance
    auto* insurance = new QCheckBox(tr("Оформить страховку"));
    auto* insuranceType = createOptionCombo(ReferenceDataRepository::OptionCategory::kInsuranceType);
    insuranceType->setVisible(false);
    auto* insuranceBox = new QWidget;
    auto* insuranceLayout = new QVBoxLayout(insuranceBox);
    insuranceLayout->setContentsMargins(0, 0, 0, 0);
    insuranceLayout->setSpacing(10);
    insuranceLayout->addWidget(insurance);
    insuranceLayout->addWidget(insuranceType);
    dialog.addField(tr("Дополнительно"), insuranceBox);

    auto* availability = UiKit::label(QString(), "muted");
    availability->setWordWrap(true);
    dialog.addWidget(availability);

    // Resolve the concrete stock item for the chosen trim.
    auto resolveVariant = [&]() -> CarVariant {
        const QString kTrim = trim ? trim->currentText() : product.Trim;
        if (kTrim == product.Trim) {
            return {product.Id, product.Color, product.StockQty};
        }
        return m_services.products().findVariant(product.Name, kTrim, product.Color).value_or(CarVariant{});
    };

    auto refresh = [&] {
        loanTerm->setVisible(credit->isChecked());
        rentalTerm->setVisible(rental->isChecked());
        insuranceType->setVisible(insurance->isChecked());
        const CarVariant kVariant = resolveVariant();
        if (kVariant.StockQty > 0) {
            availability->setText(tr("В наличии: %1 шт.").arg(kVariant.StockQty));
        } else {
            availability->setText(tr("Нет в наличии — покупка будет оформлена как заказ. "
                                     "Кредит и аренда доступны только для автомобилей в наличии."));
        }
        dialog.adjustSize();
    };
    QObject::connect(payment, &QButtonGroup::idToggled, &dialog, refresh);
    QObject::connect(insurance, &QCheckBox::toggled, &dialog, refresh);
    if (trim) {
        QObject::connect(trim, &QComboBox::currentIndexChanged, &dialog, refresh);
    }
    refresh();

    dialog.setValidator([&]() -> QString {
        const CarVariant kVariant = resolveVariant();
        const auto kKind = static_cast<PaymentKind>(payment->checkedId());
        if (kKind != PaymentKind::Purchase && kVariant.StockQty <= 0) {
            return tr("Выбранная комплектация отсутствует на складе.");
        }

        const int kClient = m_services.session().id();
        const int kCarId = kVariant.Id > 0 ? kVariant.Id : product.Id;
        RequestRepository& requests = m_services.requests();
        DatabaseHandler& db = *m_services.database();
        QString error;

        db.transaction();
        bool ok = true;
        switch (kKind) {
        case PaymentKind::Purchase:
            ok = kVariant.StockQty > 0
                     ? requests.createPurchase(kClient, kCarId, &error)
                     : requests.createOrder(kClient, product.Name, product.Color,
                                            trim ? trim->currentText() : product.Trim, &error);
            break;
        case PaymentKind::Credit:
            ok = requests.createLoan(kClient, kCarId, product.Price, loanTerm->currentData().toInt(), &error);
            break;
        case PaymentKind::Rental:
            ok = requests.createRental(kClient, kCarId, rentalTerm->currentData().toInt(), QDate::currentDate(),
                                       &error);
            break;
        }
        if (ok && insurance->isChecked()) {
            ok = requests.createInsurance(kClient, kCarId, insuranceType->currentData().toString(), &error);
        }
        if (!ok) {
            db.rollback();
            return error;
        }
        db.commit();
        return {};
    });

    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }
    notifySuccess(parent, tr("Мы свяжемся с вами после рассмотрения заявки. Статус можно отслеживать в уведомлениях."));
    return true;
}

bool RequestController::order(QWidget* parent, const ProductInfo& product)
{
    if (!m_services.session().isUser() || product.Name.isEmpty()) {
        return false;
    }

    FormDialog dialog(tr("Заказ автомобиля"), tr("Автомобиля нет в наличии — мы доставим его под заказ."), parent);
    dialog.setAcceptText(tr("Заказать"));
    dialog.addWidget(carSummary(product, &dialog));

    auto* trim = new QComboBox;
    QStringList trims = m_services.products().trimsOf(product.Name);
    if (trims.isEmpty()) {
        trims << (product.Trim.isEmpty() ? m_services.reference().defaultTrim() : product.Trim);
    }
    trim->addItems(trims);
    trim->setCurrentText(product.Trim);
    dialog.addField(tr("Комплектация"), trim);

    auto* availability = UiKit::label(QString(), "muted");
    availability->setWordWrap(true);
    dialog.addWidget(availability);

    auto variant = [&] { return m_services.products().findVariant(product.Name, trim->currentText(), product.Color); };
    auto refresh = [&] {
        const auto kVariant = variant();
        const bool kInStock = kVariant && kVariant->StockQty > 0 && kVariant->Color == product.Color;
        availability->setText(kInStock ? tr("Эта комплектация есть в наличии (%1 шт.) — будет оформлена покупка.")
                                             .arg(kVariant->StockQty)
                                       : tr("Будет создана заявка на заказ."));
    };
    QObject::connect(trim, &QComboBox::currentIndexChanged, &dialog, refresh);
    refresh();

    dialog.setValidator([&]() -> QString {
        const auto kVariant = variant();
        const bool kInStock = kVariant && kVariant->StockQty > 0 && kVariant->Color == product.Color;
        QString error;
        const bool kOk = kInStock
                             ? m_services.requests().createPurchase(m_services.session().id(), kVariant->Id, &error)
                             : m_services.requests().createOrder(m_services.session().id(), product.Name,
                                                                 product.Color, trim->currentText(), &error);
        return kOk ? QString() : error;
    });

    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }
    notifySuccess(parent, tr("Заявка на заказ отправлена. Мы сообщим, когда автомобиль будет готов."));
    return true;
}

bool RequestController::credit(QWidget* parent, const std::optional<ProductInfo>& preselected)
{
    FormDialog dialog(tr("Заявка на кредит"), tr("Предварительное решение — в течение рабочего дня."), parent);
    dialog.setAcceptText(tr("Отправить заявку"));

    auto* car = createCarCombo(preselected);
    dialog.addField(tr("Автомобиль"), car);

    auto* amount = new QDoubleSpinBox;
    amount->setDecimals(0);
    amount->setRange(100'000, 100'000'000);
    amount->setSingleStep(50'000);
    amount->setGroupSeparatorShown(true);
    amount->setSuffix(QStringLiteral(" ₽"));
    dialog.addField(tr("Сумма кредита"), amount);

    auto* term = createOptionCombo(ReferenceDataRepository::OptionCategory::kLoanTermMonths);
    dialog.addField(tr("Срок"), term);

    // Suggest the car price as the loan amount.
    auto suggestAmount = [&] {
        const QList<ProductInfo> kProducts = m_services.products().products();
        const int kId = car->currentData().toInt();
        for (const ProductInfo& p : kProducts) {
            if (p.Id == kId || (p.Name == car->currentText() && amount->value() <= amount->minimum())) {
                amount->setValue(static_cast<double>(p.Price));
                return;
            }
        }
    };
    QObject::connect(car, &QComboBox::currentIndexChanged, &dialog, suggestAmount);
    suggestAmount();

    dialog.setValidator([&]() -> QString {
        if (car->currentIndex() < 0) {
            return tr("Выберите автомобиль.");
        }
        QString error;
        const bool kOk = m_services.requests().createLoan(m_services.session().id(), car->currentData().toInt(),
                                                          static_cast<qint64>(amount->value()),
                                                          term->currentData().toInt(), &error);
        return kOk ? QString() : error;
    });

    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }
    notifySuccess(parent, tr("Заявка на кредит отправлена."));
    return true;
}

bool RequestController::rental(QWidget* parent, const std::optional<ProductInfo>& preselected)
{
    FormDialog dialog(tr("Заявка на аренду"), tr("Выберите автомобиль, срок и дату начала аренды."), parent);
    dialog.setAcceptText(tr("Отправить заявку"));

    auto* car = createCarCombo(preselected);
    dialog.addField(tr("Автомобиль"), car);
    auto* term = createOptionCombo(ReferenceDataRepository::OptionCategory::kRentalDays);
    dialog.addField(tr("Срок аренды"), term);
    auto* calendar = new QCalendarWidget;
    calendar->setMinimumDate(QDate::currentDate());
    calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    calendar->setGridVisible(false);
    dialog.addField(tr("Дата начала"), calendar);

    dialog.setValidator([&]() -> QString {
        if (car->currentIndex() < 0) {
            return tr("Выберите автомобиль.");
        }
        QString error;
        const bool kOk = m_services.requests().createRental(m_services.session().id(), car->currentData().toInt(),
                                                            term->currentData().toInt(), calendar->selectedDate(),
                                                            &error);
        return kOk ? QString() : error;
    });

    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }
    notifySuccess(parent, tr("Заявка на аренду отправлена."));
    return true;
}

bool RequestController::testDrive(QWidget* parent, const std::optional<ProductInfo>& preselected)
{
    FormDialog dialog(tr("Запись на тест-драйв"), tr("Менеджер подтвердит запись и пришлёт уведомление."), parent);
    dialog.setAcceptText(tr("Записаться"));

    auto* car = createCarCombo(preselected);
    dialog.addField(tr("Автомобиль"), car);
    auto* calendar = new QCalendarWidget;
    calendar->setMinimumDate(QDate::currentDate().addDays(1));
    calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    dialog.addField(tr("Дата"), calendar);
    auto* time = new QTimeEdit(QTime(10, 0));
    time->setDisplayFormat(QStringLiteral("HH:mm"));
    time->setTimeRange(QTime(9, 0), QTime(20, 0));
    dialog.addField(tr("Время"), time);

    dialog.setValidator([&]() -> QString {
        if (car->currentIndex() < 0) {
            return tr("Выберите автомобиль.");
        }
        QString error;
        const bool kOk = m_services.requests().createTestDrive(m_services.session().id(), car->currentData().toInt(),
                                                               QDateTime(calendar->selectedDate(), time->time()),
                                                               &error);
        return kOk ? QString() : error;
    });

    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }
    notifySuccess(parent, tr("Вы записаны на тест-драйв. Ожидайте подтверждения."));
    return true;
}
