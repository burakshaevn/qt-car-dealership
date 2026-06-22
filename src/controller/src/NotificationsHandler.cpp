#include "NotificationsHandler.h"
#include "ui_notifications.h"

#include "ProductRepository.h"
#include "PriceFormatter.h"
#include "ContractTemplates.h"
#include "ThemeStyleProvider.h"

#include <QTimer>
#include <QScrollBar>
#include <QDateTime>
#include <QLabel>
#include <QMessageBox>
#include <QSqlQuery>
#include <QHBoxLayout>
#include <QPushButton>
#include <array>

namespace {
struct NotificationSource final {
    const char* type;
    const char* table;
    const char* additionalInfoExpr;
    const char* dateColumn;
    const char* carIdExpr;
};

const std::array<NotificationSource, 7> kNotificationSources{{
    {"service", "service_requests", "service_type", "scheduled_date", "car_id"},
    {"insurance", "insurance_requests", "insurance_type", "created_at", "car_id"},
    {"loan", "loan_requests", "CAST(loan_amount AS TEXT)", "created_at", "car_id"},
    {"test_drive", "test_drives", "''", "scheduled_date", "car_id"},
    {"rental", "rental_requests", "CAST(rental_days AS TEXT)", "start_date", "car_id"},
    {"purchase", "purchase_requests", "''", "created_at", "car_id"},
    {"order", "order_requests", "car_name", "created_at", "0"},
}};

QString buildNotificationSelect(const NotificationSource& source, const QString& whereClause)
{
    return QString("SELECT '%1' as type, id, status, %2 as additional_info, %3 as date_info, %4 as car_id FROM %5 WHERE %6")
        .arg(source.type, source.additionalInfoExpr, source.dateColumn, source.carIdExpr, source.table, whereClause);
}
} // namespace

NotificationsHandler::NotificationsHandler(QSharedPointer<DatabaseHandler> database_handler, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::notifications)
    , m_notifications_layout(nullptr)
    , m_database_handler(std::move(database_handler))
    , m_is_sorted_ascending(true)
    , m_current_user_id(-1)
    , m_current_filter("Все уведомления")
{
    ui->setupUi(this);
    ApplyThemeStyle(this, "NotificationsDialog");
    ui->btn_sort_by_data->setProperty("type", "secondary");
    ui->btn_mark_all_read->setProperty("type", "primary");
    setWindowTitle(QStringLiteral("Уведомления"));
    setWindowIcon(LoadThemeIcon("inbox.svg"));

    connect(ui->btn_sort_by_data, &QPushButton::clicked, this, &NotificationsHandler::onSortButtonClicked);
    connect(ui->filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NotificationsHandler::onFilterChanged);
    connect(ui->btn_mark_all_read, &QPushButton::clicked, this, &NotificationsHandler::onMarkAllReadClicked);

    // Устанавливаем фильтр "Все уведомления" по умолчанию
    ui->filterCombo->setCurrentIndex(0);

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
    qDebug() << "loadAndShowNotifications: user_id =" << user_id;
    m_current_user_id = user_id;
    this->clear();

    const auto notifications = getNewNotifications(user_id);

    bool hasNotifications {false};

    QSqlQuery query = notifications.value<QSqlQuery>();
    qDebug() << "Processing notifications query...";
    
    query.first();
    query.previous();
    
    while (query.next())
    {
        hasNotifications = true;
        QString type = query.value("type").toString();
        int id = query.value("id").toInt();
        QString status = query.value("status").toString();
        QString additionalInfo = query.value("additional_info").toString();
        QDateTime dateInfo = query.value("date_info").toDateTime();
        int carId = query.value("car_id").toInt();
        
        qDebug() << "Processing notification:" << type << "id:" << id << "status:" << status;

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
        else if (type == "purchase") {
            title = "Заявка на покупку";
            message = QString("Статус заявки на покупку автомобиля\nДата: %1\nСтатус: %2")
                          .arg(dateInfo.toString("dd.MM.yyyy HH:mm"), status);
        }
        else if (type == "order") {
            title = "Заявка на заказ";
            message = QString("Статус заявки на заказ автомобиля %1\nДата: %2\nСтатус: %3")
                          .arg(additionalInfo, dateInfo.toString("dd.MM.yyyy HH:mm"), status);
        }

        // Создаем виджет уведомления
        QWidget *notificationWidget = new QWidget();
        notificationWidget->setObjectName("notificationCard");
        QVBoxLayout *notificationLayout = new QVBoxLayout(notificationWidget);

        // Добавляем заголовок и текст
        QLabel *titleLabel = new QLabel(title);
        QLabel *messageLabel = new QLabel(message);
        messageLabel->setWordWrap(true);

        ApplyThemeStyle(titleLabel, "NotificationTitle");
        ApplyThemeStyle(messageLabel, "NotificationMessage");

        notificationLayout->addWidget(titleLabel);
        notificationLayout->addWidget(messageLabel);

        if ((type == "service"      && status == "подтверждено") ||
            (type == "insurance"    && status == "одобрено") ||
            (type == "loan"         && status == "одобрено") ||
            (type == "test_drive"   && status == "одобрено") ||
            (type == "rental"       && status == "одобрено") ||
            (type == "purchase"     && status == "одобрено") ||
            (type == "order"        && status == "одобрено")) {

            QPushButton *downloadButton = new QPushButton("Скачать договор");
            notificationLayout->addWidget(downloadButton);
            connect(downloadButton, &QPushButton::clicked, [this, type, id, carId, additionalInfo, dateInfo, user_id]() {
                generateContractFromNotification(type, id, carId, additionalInfo, dateInfo.toString("dd.MM.yyyy HH:mm"));
            });
        }

        ApplyThemeStyle(notificationWidget, "NotificationItem");
        notificationWidget->setProperty("notificationDate", dateInfo);
        m_notifications_layout->addWidget(notificationWidget);
    }

    if (!hasNotifications) {
        qDebug() << "No notifications found for user" << user_id;
        QLabel *noNotificationsLabel = new QLabel("Уведомлений нет", this);
        noNotificationsLabel->setAlignment(Qt::AlignCenter);
        ApplyThemeStyle(noNotificationsLabel, "NotificationEmpty");
        m_notifications_layout->addWidget(noNotificationsLabel);
    } else {
        qDebug() << "Found notifications for user" << user_id;
    }

    QTimer::singleShot(100, this, [this]() {
        ui->scrollArea->verticalScrollBar()->setValue(0);
    });
}

QVariant NotificationsHandler::getNewNotifications(const int user_id) {
    qDebug() << "getNewNotifications: user_id =" << user_id << "filter =" << m_current_filter;

    QStringList queryParts;
    queryParts.reserve(static_cast<int>(kNotificationSources.size()));

    const int filterIndex = ui->filterCombo->currentIndex();
    for (const auto& source : kNotificationSources) {
        QString whereClause = QString("client_id = %1").arg(user_id);

        if (filterIndex == 1) {
            whereClause += " AND notification_shown = false";
        } else if (filterIndex == 2) {
            whereClause += " AND status IN ('одобрено', 'подтверждено')";
        } else if (filterIndex == 3) {
            whereClause += QString(" AND %1 >= NOW() - INTERVAL '7 days'").arg(source.dateColumn);
        } else if (filterIndex == 4) {
            whereClause += QString(" AND %1 >= NOW() - INTERVAL '30 days'").arg(source.dateColumn);
        } else {
            whereClause += " AND (notification_shown = false OR notification_shown IS NULL)";
        }

        queryParts.append(buildNotificationSelect(source, whereClause));
    }

    QString filteredQuery = queryParts.join(" UNION ALL ");
    filteredQuery += " ORDER BY date_info DESC";

    qDebug() << "Executing query:" << filteredQuery;
    QVariant result = m_database_handler.lock()->ExecuteSelectQuery(filteredQuery);

    return result;
}

void NotificationsHandler::markNotificationsAsReaded(const int user_id) {
    qDebug() << "markNotificationsAsReaded: user_id =" << user_id;

    // Помечаем как прочитанные только те уведомления, которые были показаны
    // (т.е. те, которые имеют notification_shown = false или NULL)
    for (const auto& source : kNotificationSources) {
        QString updateQuery = QString("UPDATE %1 SET notification_shown = true WHERE client_id = %2 AND (notification_shown = false OR notification_shown IS NULL)")
            .arg(source.table)
            .arg(user_id);

        qDebug() << "Executing:" << updateQuery;
        QVariant result = m_database_handler.lock()->ExecuteQuery(updateQuery);
        qDebug() << "Result for" << source.table << ":" << result.toBool();
    }
}

void NotificationsHandler::clear() {
    while (QLayoutItem* item = m_notifications_layout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
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
                        widget->deleteLater();
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

void NotificationsHandler::generateContractFromNotification(const QString& type, int requestId, int carId,
                                                            const QString& additionalInfo, const QString& dateInfo) {
    QString carName, carColor, carPrice;
    QString currentDate = QDateTime::currentDateTime().toString("dd.MM.yyyy");

    if (type == "order") {
        // Для заказа получаем данные из order_requests
        QSqlQuery orderQuery;
        QString orderQueryStr = QString("SELECT car_name, color FROM order_requests WHERE id = %1").arg(requestId);
        if (!orderQuery.exec(orderQueryStr) || !orderQuery.next()) {
            QMessageBox::warning(this, "Ошибка", "Не удалось получить информацию о заказе");
            return;
        }
        carName = orderQuery.value("car_name").toString();
        carColor = orderQuery.value("color").toString();
        
        // Получаем цену по имени автомобиля
        QSqlQuery priceQuery;
        QString priceQueryStr = QString("SELECT price FROM cars WHERE name = '%1' LIMIT 1").arg(carName);
        if (priceQuery.exec(priceQueryStr) && priceQuery.next()) {
            carPrice = priceQuery.value("price").toString();
        } else {
            carPrice = "0"; // Fallback
        }
    } else {
        // Для остальных типов получаем информацию об автомобиле по carId
        QSqlQuery carQuery;
        QString carQueryStr = QString("SELECT name, color, price FROM cars WHERE id = %1").arg(carId);

        if (!carQuery.exec(carQueryStr) || !carQuery.next()) {
            QMessageBox::warning(this, "Ошибка", "Не удалось получить информацию об автомобиле");
            return;
        }

        carName = carQuery.value("name").toString();
        carColor = carQuery.value("color").toString();
        carPrice = carQuery.value("price").toString();
    }

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
    else if (type == "purchase") {
        htmlContent = ContractTemplates::getPurchaseContractHtml(
            currentDate, carName, carColor, carPrice
            );
    }
    else if (type == "order") {
        // Для заказа получаем дополнительные данные из order_requests
        QSqlQuery orderQuery;
        QString orderQueryStr = QString("SELECT trim FROM order_requests WHERE id = %1").arg(requestId);
        if (orderQuery.exec(orderQueryStr) && orderQuery.next()) {
            QString trim = orderQuery.value("trim").toString();
            htmlContent = ContractTemplates::getOrderContractHtml(
                currentDate, carName, carColor, carPrice, trim
                );
        }
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

void NotificationsHandler::onFilterChanged()
{
    if (m_current_user_id == -1) return;
    
    m_current_filter = ui->filterCombo->currentText();
    loadAndShowNotifications(m_current_user_id);
}

void NotificationsHandler::onMarkAllReadClicked()
{
    if (m_current_user_id == -1) return;
    
    markNotificationsAsReaded(m_current_user_id);
    QMessageBox::information(this, "Успех", "Все уведомления помечены как прочитанные");
    loadAndShowNotifications(m_current_user_id);
}

