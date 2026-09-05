#include "NotificationsHandler.h"
#include "ui_notifications.h"

#include "ContractTemplates.h"
#include "PriceFormatter.h"
#include "ProductRepository.h"
#include "ThemeStyleProvider.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollBar>
#include <QSqlQuery>
#include <QTimer>
#include <array>

namespace {
struct NotificationSource final {
    const char* Type;
    const char* Table;
    const char* AdditionalInfoExpr;
    const char* DateColumn;
    const char* CarIdExpr;
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
        .arg(source.Type, source.AdditionalInfoExpr, source.DateColumn, source.CarIdExpr, source.Table, whereClause);
}
} // namespace

NotificationsHandler::NotificationsHandler(QSharedPointer<DatabaseHandler> databaseHandler,
                                           QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::notifications)
    , m_notificationsLayout(nullptr)
    , m_databaseHandler(std::move(databaseHandler))
    , m_isSortedAscending(true)
    , m_currentUserId(-1)
    , m_currentFilter("Все уведомления")
{
    m_ui->setupUi(this);
    applyThemeStyle(this, "NotificationsDialog");
    m_ui->btn_sort_by_data->setProperty("type", "secondary");
    m_ui->btn_mark_all_read->setProperty("type", "primary");
    setWindowTitle(QStringLiteral("Уведомления"));
    setWindowIcon(loadThemeIcon("inbox.svg"));

    connect(m_ui->btn_sort_by_data,
            &QPushButton::clicked,
            this,
            &NotificationsHandler::onSortButtonClicked);
    connect(m_ui->filterCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &NotificationsHandler::onFilterChanged);
    connect(m_ui->btn_mark_all_read,
            &QPushButton::clicked,
            this,
            &NotificationsHandler::onMarkAllReadClicked);

    // Устанавливаем фильтр "Все уведомления" по умолчанию
    m_ui->filterCombo->setCurrentIndex(0);

    QWidget *scrollWidget = new QWidget();
    m_notificationsLayout = new QVBoxLayout(scrollWidget);

    m_notificationsLayout->setAlignment(Qt::AlignTop);
    m_notificationsLayout->setContentsMargins(8, 8, 8, 8);
    m_notificationsLayout->setSpacing(4);

    scrollWidget->setLayout(m_notificationsLayout);

    m_ui->scrollArea->setWidget(scrollWidget);
    m_ui->scrollArea->setWidgetResizable(true);
    m_ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

NotificationsHandler::~NotificationsHandler()
{
    delete m_ui;
}

void NotificationsHandler::loadAndShowNotifications(const int kUserId) {
    qDebug() << "loadAndShowNotifications: user_id =" << kUserId;
    m_currentUserId = kUserId;
    this->clear();

    const auto kNotifications = getNewNotifications(kUserId);

    bool hasNotifications {false};

    QSqlQuery query = kNotifications.value<QSqlQuery>();
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
                          .arg(formatPrice(additionalInfo.toLongLong()), status);
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

        applyThemeStyle(titleLabel, "NotificationTitle");
        applyThemeStyle(messageLabel, "NotificationMessage");

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
            connect(downloadButton, &QPushButton::clicked, [this, type, id, carId, additionalInfo, dateInfo, kUserId]() {
                generateContractFromNotification(type, id, carId, additionalInfo, dateInfo.toString("dd.MM.yyyy HH:mm"));
            });
        }

        applyThemeStyle(notificationWidget, "NotificationItem");
        notificationWidget->setProperty("notificationDate", dateInfo);
        m_notificationsLayout->addWidget(notificationWidget);
    }

    if (!hasNotifications) {
        qDebug() << "No notifications found for user" << kUserId;
        QLabel *noNotificationsLabel = new QLabel("Уведомлений нет", this);
        noNotificationsLabel->setAlignment(Qt::AlignCenter);
        applyThemeStyle(noNotificationsLabel, "NotificationEmpty");
        m_notificationsLayout->addWidget(noNotificationsLabel);
    } else {
        qDebug() << "Found notifications for user" << kUserId;
    }

    QTimer::singleShot(100, this, [this]() { m_ui->scrollArea->verticalScrollBar()->setValue(0); });
}

QVariant NotificationsHandler::getNewNotifications(const int kUserId) {
    qDebug() << "getNewNotifications: user_id =" << kUserId << "filter =" << m_currentFilter;

    QStringList queryParts;
    queryParts.reserve(static_cast<int>(kNotificationSources.size()));

    const int kFilterIndex = m_ui->filterCombo->currentIndex();
    for (const auto& source : kNotificationSources) {
        QString whereClause = QString("client_id = %1").arg(kUserId);

        if (kFilterIndex == 1) {
            whereClause += " AND notification_shown = false";
        } else if (kFilterIndex == 2) {
            whereClause += " AND status IN ('одобрено', 'подтверждено')";
        } else if (kFilterIndex == 3) {
            whereClause += QString(" AND %1 >= NOW() - INTERVAL '7 days'").arg(source.DateColumn);
        } else if (kFilterIndex == 4) {
            whereClause += QString(" AND %1 >= NOW() - INTERVAL '30 days'").arg(source.DateColumn);
        } else {
            whereClause += " AND (notification_shown = false OR notification_shown IS NULL)";
        }

        queryParts.append(buildNotificationSelect(source, whereClause));
    }

    QString filteredQuery = queryParts.join(" UNION ALL ");
    filteredQuery += " ORDER BY date_info DESC";

    qDebug() << "Executing query:" << filteredQuery;
    QVariant result = m_databaseHandler.lock()->executeSelectQuery(filteredQuery);

    return result;
}

void NotificationsHandler::markNotificationsAsReaded(const int kUserId) {
    qDebug() << "markNotificationsAsReaded: user_id =" << kUserId;

    // Помечаем как прочитанные только те уведомления, которые были показаны
    // (т.е. те, которые имеют notification_shown = false или NULL)
    for (const auto& source : kNotificationSources) {
        QString updateQuery = QString("UPDATE %1 SET notification_shown = true WHERE client_id = %2 AND (notification_shown = false OR notification_shown IS NULL)")
            .arg(source.Table)
            .arg(kUserId);

        qDebug() << "Executing:" << updateQuery;
        QVariant result = m_databaseHandler.lock()->executeQuery(updateQuery);
        qDebug() << "Result for" << source.Table << ":" << result.toBool();
    }
}

void NotificationsHandler::clear() {
    while (QLayoutItem* item = m_notificationsLayout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }

    // Также очищаем scrollArea
    if (QWidget* scrollWidget = m_ui->scrollArea->widget()) {
        if (scrollWidget != m_notificationsLayout->parentWidget()) {
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
    for (int i = 0; i < m_notificationsLayout->count(); ++i) {
        QLayoutItem* item = m_notificationsLayout->itemAt(i);
        if (item && item->widget()) {
            item->widget()->setMaximumWidth(m_ui->groupBox->width()
                                            - m_notificationsLayout->contentsMargins().left()
                                            - m_notificationsLayout->contentsMargins().right());
        }
    }
}

void NotificationsHandler::onSortButtonClicked()
{
    m_isSortedAscending = !m_isSortedAscending;

    if (m_isSortedAscending) {
        m_ui->btn_sort_by_data->setText("Сортировка по дате ↑");
    } else {
        m_ui->btn_sort_by_data->setText("Сортировка по дате ↓");
    }

    sortNotifications(m_isSortedAscending);
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
        htmlContent = contract_templates::getServiceContractHtml(currentDate,
                                                                 carName,
                                                                 carColor,
                                                                 additionalInfo,
                                                                 dateInfo);
    }
    else if (type == "insurance") {
        htmlContent = contract_templates::getInsuranceContractHtml(currentDate,
                                                                   carName,
                                                                   carColor,
                                                                   additionalInfo);
    }
    else if (type == "loan") {
        // Для кредита получаем дополнительные данные
        QSqlQuery loanQuery;
        QString loanQueryStr = QString("SELECT loan_term_months FROM loan_requests WHERE id = %1").arg(requestId);
        if (loanQuery.exec(loanQueryStr) && loanQuery.next()) {
            QString loanTerm = loanQuery.value("loan_term_months").toString();
            htmlContent = contract_templates::getLoanContractHtml(currentDate,
                                                                  carName,
                                                                  carColor,
                                                                  carPrice,
                                                                  additionalInfo,
                                                                  loanTerm);
        }
    }
    else if (type == "rental") {
        htmlContent = contract_templates::getRentalContractHtml(currentDate,
                                                                carName,
                                                                carColor,
                                                                additionalInfo,
                                                                dateInfo.split(
                                                                    " ")[0] // Берем только дату
        );
    }
    else if (type == "test_drive") {
        htmlContent = contract_templates::getTestDriveContractHtml(currentDate,
                                                                   carName,
                                                                   carColor,
                                                                   dateInfo);
    }
    else if (type == "purchase") {
        htmlContent = contract_templates::getPurchaseContractHtml(currentDate,
                                                                  carName,
                                                                  carColor,
                                                                  carPrice);
    }
    else if (type == "order") {
        // Для заказа получаем дополнительные данные из order_requests
        QSqlQuery orderQuery;
        QString orderQueryStr = QString("SELECT trim FROM order_requests WHERE id = %1").arg(requestId);
        if (orderQuery.exec(orderQueryStr) && orderQuery.next()) {
            QString trim = orderQuery.value("trim").toString();
            htmlContent = contract_templates::getOrderContractHtml(currentDate,
                                                                   carName,
                                                                   carColor,
                                                                   carPrice,
                                                                   trim);
        }
    }

    if (!htmlContent.isEmpty()) {
        // Создаем временный ProductInfo для сохранения
        ProductInfo product;
        product.Name = carName;
        product.Color = carColor;
        product.Price = carPrice.toDouble();

        // Сохраняем договор
        contract_templates::saveContract(htmlContent, product);
    }
}

void NotificationsHandler::sortNotifications(const bool kAscending)
{
    // Сохраняем все виджеты уведомлений вместе с их датами
    QList<QPair<QDateTime, QWidget*>> notificationWidgets;

    while (QLayoutItem* item = m_notificationsLayout->takeAt(0)) {
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
              [kAscending](const QPair<QDateTime, QWidget*>& a,
                          const QPair<QDateTime, QWidget*>& b) {
                  if (kAscending) {
                      return a.first < b.first;
                  } else {
                      return a.first > b.first;
                  }
              });

    // Добавляем отсортированные виджеты обратно в layout
    for (const auto& pair : notificationWidgets) {
        m_notificationsLayout->addWidget(pair.second);
    }
}

void NotificationsHandler::onFilterChanged()
{
    if (m_currentUserId == -1)
        return;

    m_currentFilter = m_ui->filterCombo->currentText();
    loadAndShowNotifications(m_currentUserId);
}

void NotificationsHandler::onMarkAllReadClicked()
{
    if (m_currentUserId == -1)
        return;

    markNotificationsAsReaded(m_currentUserId);
    QMessageBox::information(this, "Успех", "Все уведомления помечены как прочитанные");
    loadAndShowNotifications(m_currentUserId);
}

