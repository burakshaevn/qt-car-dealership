#include "notifications_handler.h"
#include "ui_notifications.h"

#include "products.h"
#include "domain.h"
#include "contract_templates.h"
#include "product_card.h"

#include <QTimer>
#include <QScrollBar>
#include <QDateTime>
#include <QLabel>
#include <QMessageBox>

NotificationsHandler::NotificationsHandler(QSharedPointer<DatabaseHandler> database_handler, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::notifications)
    , m_notifications_layout(nullptr)
    , m_database_handler(std::move(database_handler))
    , m_is_sorted_ascending(true)
{
    ui->setupUi(this);

    connect(ui->btn_sort_by_data, &QPushButton::clicked, this, &NotificationsHandler::onSortButtonClicked);

    QWidget *scrollWidget = new QWidget();
    m_notifications_layout = new QVBoxLayout(scrollWidget);

    m_notifications_layout->setAlignment(Qt::AlignTop);
    m_notifications_layout->setContentsMargins(8, 8, 8, 8);
    m_notifications_layout->setSpacing(4);

    scrollWidget->setLayout(m_notifications_layout);

    ui->scrollArea->setWidget(scrollWidget);
    ui->scrollArea->setWidgetResizable(true);
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

NotificationsHandler::~NotificationsHandler()
{
    delete ui;
}

void NotificationsHandler::loadAndShowNotifications(const int user_id) {

    this->clear();

    const auto notifications = getNewNotifications(user_id);

    bool hasNotifications {false};

    QSqlQuery query = notifications.value<QSqlQuery>();
    while (query.next())
    {
        hasNotifications = true;
        QString type = query.value("type").toString();
        int id = query.value("id").toInt();
        QString status = query.value("status").toString();
        QString additionalInfo = query.value("additional_info").toString();
        QDateTime dateInfo = query.value("date_info").toDateTime();
        int carId = query.value("car_id").toInt();

        QString title;
        QString message;

        if (type == "service") {
            title = "Заявка на обслуживание";
            message = QString("Статус заявки на %1\nЗапланировано на: %2\nСтатус: %3")
                          .arg(additionalInfo, dateInfo.toString("dd.MM.yyyy HH:mm"), status);
        }
        else if (type == "insurance") {
            title = "Заявка на страхование";
            message = QString("Статус заявки на %1\nСтатус: %2")
                          .arg(additionalInfo, status);
        }
        else if (type == "loan") {
            title = "Заявка на кредит";
            message = QString("Статус заявки на сумму %1 руб.\nСтатус: %2")
                          .arg(FormatPrice(additionalInfo.toLongLong()), status);
        }
        else if (type == "rental") {
            title = "Заявка на аренду";
            message = QString("Статус заявки на аренду автомобиля на %1 дней\nДата начала: %2\nСтатус: %3")
                          .arg(additionalInfo, dateInfo.toString("dd.MM.yyyy"), status);
        }
        else if (type == "test_drive") {
            title = "Заявка на тест-драйв";
            message = QString("Статус заявки на тест-драйв\nЗапланировано на: %1\nСтатус: %2")
                          .arg(dateInfo.toString("dd.MM.yyyy HH:mm"), status);
        }

        // Создаем виджет уведомления
        QWidget *notificationWidget = new QWidget();
        QVBoxLayout *notificationLayout = new QVBoxLayout(notificationWidget);

        // Добавляем заголовок и текст
        QLabel *titleLabel = new QLabel(title);
        QLabel *messageLabel = new QLabel(message);
        messageLabel->setWordWrap(true);

        titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
        messageLabel->setStyleSheet("font-size: 12px; color: #666;");

        notificationLayout->addWidget(titleLabel);
        notificationLayout->addWidget(messageLabel);

        if ((type == "service"      && status == "подтверждено") ||
            (type == "insurance"    && status == "одобрено") ||
            (type == "loan"         && status == "одобрено") ||
            (type == "test_drive"   && status == "одобрено") ||
            (type == "rental"       && status == "одобрено")) {

            QPushButton *downloadButton = new QPushButton("Скачать договор");
            notificationLayout->addWidget(downloadButton);
            connect(downloadButton, &QPushButton::clicked, [this, type, id, carId, additionalInfo, dateInfo, user_id]() {
                generateContractFromNotification(type, id, carId, additionalInfo, dateInfo.toString("dd.MM.yyyy HH:mm"));
            });
        }

        notificationWidget->setStyleSheet(R"(
            QWidget {
                background-color: white;
                border: 1px solid #e0e0e0;
                border-radius: 8px;
                padding: 10px;
                margin: 5px;
            }
        )");
        notificationWidget->setProperty("notificationDate", dateInfo);
        m_notifications_layout->addWidget(notificationWidget);
    }

    if (!hasNotifications) {
        QLabel *noNotificationsLabel = new QLabel("Уведомлений нет", this);
        noNotificationsLabel->setAlignment(Qt::AlignCenter);
        noNotificationsLabel->setStyleSheet(R"(
            QLabel {
                font-size: 14px;
                color: #666;
                padding: 10px;
            }
        )");
        m_notifications_layout->addWidget(noNotificationsLabel);
    }

    QTimer::singleShot(100, this, [this]() {
        ui->scrollArea->verticalScrollBar()->setValue(0);
    });

    // MarkNotificationsAsReaded(user_id);
}

QVariant NotificationsHandler::getNewNotifications(const int user_id) {
    // Получаем только НЕпрочитанные уведомления с дополнительными данными
    QVariant result = m_database_handler.lock()->ExecuteSelectQuery(
        QString("SELECT 'service' as type, id, status, service_type as additional_info, scheduled_date as date_info, car_id FROM service_requests "
                "WHERE client_id = %1 "
                "UNION ALL "
                "SELECT 'insurance' as type, id, status, insurance_type as additional_info, created_at as date_info, car_id FROM insurance_requests "
                "WHERE client_id = %1 "
                "UNION ALL "
                "SELECT 'loan' as type, id, status, CAST(loan_amount AS TEXT) as additional_info, created_at as date_info, car_id FROM loan_requests "
                "WHERE client_id = %1 "
                "UNION ALL "
                "SELECT 'test_drive' as type, id, status, 'Тест-драйв' as additional_info, scheduled_date as date_info, car_id FROM test_drives "
                "WHERE client_id = %1 "
                "UNION ALL "
                "SELECT 'rental' as type, id, status, CAST(rental_days AS TEXT) as additional_info, start_date as date_info, car_id FROM rental_requests "
                "WHERE client_id = %1 "
                "ORDER BY date_info DESC")
            .arg(user_id));

    return result;
}

void NotificationsHandler::markNotificationsAsReaded(const int user_id) {
    // Помечаем как прочитанные только те уведомления, которые были показаны
    // (т.е. те, которые имеют notification_shown = false или NULL)
    QString updateQuery = QString(
                              "UPDATE service_requests SET notification_shown = true "
                              "WHERE client_id = %1 AND (notification_shown = false OR notification_shown IS NULL);"
                              "UPDATE insurance_requests SET notification_shown = true "
                              "WHERE client_id = %1 AND (notification_shown = false OR notification_shown IS NULL);"
                              "UPDATE loan_requests SET notification_shown = true "
                              "WHERE client_id = %1 AND (notification_shown = false OR notification_shown IS NULL);"
                              "UPDATE test_drives SET notification_shown = true "
                              "WHERE client_id = %1 AND (notification_shown = false OR notification_shown IS NULL);"
                              "UPDATE rental_requests SET notification_shown = true "
                              "WHERE client_id = %1 AND (notification_shown = false OR notification_shown IS NULL);"
                              ).arg(user_id);

    m_database_handler.lock()->ExecuteQuery(updateQuery);
}

void NotificationsHandler::clear() {
    // Удаляем все виджеты из layout немедленно
    while (QLayoutItem* item = m_notifications_layout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            // Удаляем виджет немедленно, а не через deleteLater()
            delete widget;
        }
        delete item;
    }

    // Также очищаем scrollArea
    if (QWidget* scrollWidget = ui->scrollArea->widget()) {
        if (scrollWidget != m_notifications_layout->parentWidget()) {
            // Если scrollWidget отличается от того, где находится наш layout,
            // очищаем и его
            QLayout* scrollLayout = scrollWidget->layout();
            if (scrollLayout) {
                while (QLayoutItem* item = scrollLayout->takeAt(0)) {
                    if (QWidget* widget = item->widget()) {
                        delete widget;
                    }
                    delete item;
                }
            }
        }
    }
}

void NotificationsHandler::resizeEvent(QResizeEvent *event) {
    QDialog::resizeEvent(event);
    for (int i = 0; i < m_notifications_layout->count(); ++i) {
        QLayoutItem *item = m_notifications_layout->itemAt(i);
        if (item && item->widget()) {
            item->widget()->setMaximumWidth(ui->groupBox->width() -
                                            m_notifications_layout->contentsMargins().left() -
                                            m_notifications_layout->contentsMargins().right());
        }
    }
}

void NotificationsHandler::onSortButtonClicked()
{
    m_is_sorted_ascending = !m_is_sorted_ascending;

    if (m_is_sorted_ascending) {
        ui->btn_sort_by_data->setText("Сортировка по дате ↑");
    } else {
        ui->btn_sort_by_data->setText("Сортировка по дате ↓");
    }

    sortNotifications(m_is_sorted_ascending);
}

void NotificationsHandler::addNotification(const QStringView title, const QStringView date, const QStringView text) {
    QWidget *notificationWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(notificationWidget);

    QLabel *titleLabel = new QLabel(title.toString());
    QLabel *dateLabel = new QLabel(date.toString());
    QLabel *textLabel = new QLabel(text.toString());
    textLabel->setWordWrap(true);

    titleLabel->setStyleSheet("font-weight: bold;");
    dateLabel->setStyleSheet("color: #666; font-size: 12px;");
    dateLabel->setProperty("sortDate", date.toString());  // Дата сохраняется для сортировки в будущем

    layout->addWidget(titleLabel);
    layout->addWidget(dateLabel);
    layout->addWidget(textLabel);

    notificationWidget->setStyleSheet(R"(
        QWidget {
            background-color: white;
            border: 1px solid #e0e0e0;
            border-radius: 8px;
            padding: 10px;
            margin: 5px;
        }
    )");

    QDateTime notificationDate = QDateTime::fromString(date.toString(), "dd.MM.yyyy");
    notificationWidget->setProperty("notificationDate", notificationDate);

    m_notifications_layout->addWidget(notificationWidget);
}

void NotificationsHandler::generateContractFromNotification(const QString& type, int requestId, int carId,
                                                            const QString& additionalInfo, const QString& dateInfo) {
    // Получаем информацию об автомобиле
    QSqlQuery carQuery;
    QString carQueryStr = QString("SELECT name, color, price FROM cars WHERE id = %1").arg(carId);

    if (!carQuery.exec(carQueryStr) || !carQuery.next()) {
        QMessageBox::warning(this, "Ошибка", "Не удалось получить информацию об автомобиле");
        return;
    }

    QString carName = carQuery.value("name").toString();
    QString carColor = carQuery.value("color").toString();
    QString carPrice = carQuery.value("price").toString();
    QString currentDate = QDateTime::currentDateTime().toString("dd.MM.yyyy");

    QString htmlContent;

    // Генерируем соответствующий договор в зависимости от типа
    if (type == "service") {
        htmlContent = ContractTemplates::getServiceContractHtml(
            currentDate, carName, carColor, additionalInfo, dateInfo
            );
    }
    else if (type == "insurance") {
        htmlContent = ContractTemplates::getInsuranceContractHtml(
            currentDate, carName, carColor, additionalInfo
            );
    }
    else if (type == "loan") {
        // Для кредита получаем дополнительные данные
        QSqlQuery loanQuery;
        QString loanQueryStr = QString("SELECT loan_term_months FROM loan_requests WHERE id = %1").arg(requestId);
        if (loanQuery.exec(loanQueryStr) && loanQuery.next()) {
            QString loanTerm = loanQuery.value("loan_term_months").toString();
            htmlContent = ContractTemplates::getLoanContractHtml(
                currentDate, carName, carColor, carPrice, additionalInfo, loanTerm
                );
        }
    }
    else if (type == "rental") {
        htmlContent = ContractTemplates::getRentalContractHtml(
            currentDate, carName, carColor, additionalInfo, dateInfo.split(" ")[0] // Берем только дату
            );
    }
    else if (type == "test_drive") {
        htmlContent = ContractTemplates::getTestDriveContractHtml(
            currentDate, carName, carColor, dateInfo
            );
    }

    if (!htmlContent.isEmpty()) {
        // Создаем временный ProductInfo для сохранения
        ProductInfo product;
        product.name_ = carName;
        product.color_ = carColor;
        product.price_ = carPrice.toDouble();

        // Сохраняем договор
        ContractTemplates::saveContract(htmlContent, product);
    }
}

void NotificationsHandler::sortNotifications(const bool ascending)
{
    // Сохраняем все виджеты уведомлений вместе с их датами
    QList<QPair<QDateTime, QWidget*>> notificationWidgets;

    while (QLayoutItem* item = m_notifications_layout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            // Извлекаем дату из свойства виджета
            QVariant dateVariant = widget->property("notificationDate");
            if (dateVariant.isValid()) {
                QDateTime date = dateVariant.toDateTime();
                notificationWidgets.append(qMakePair(date, widget));
            }
        }
        delete item;
    }

    // Сортируем виджеты по дате
    std::sort(notificationWidgets.begin(), notificationWidgets.end(),
              [ascending](const QPair<QDateTime, QWidget*>& a,
                          const QPair<QDateTime, QWidget*>& b) {
                  if (ascending) {
                      return a.first < b.first;
                  } else {
                      return a.first > b.first;
                  }
              });

    // Добавляем отсортированные виджеты обратно в layout
    for (const auto& pair : notificationWidgets) {
        m_notifications_layout->addWidget(pair.second);
    }
}
