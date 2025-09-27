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
#include <QSqlQuery>
#include <QHBoxLayout>
#include <QPushButton>

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

    connect(ui->btn_sort_by_data, &QPushButton::clicked, this, &NotificationsHandler::onSortButtonClicked);
    connect(ui->filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NotificationsHandler::onFilterChanged);
    connect(ui->btn_mark_all_read, &QPushButton::clicked, this, &NotificationsHandler::onMarkAllReadClicked);
    connect(ui->btn_clear_old, &QPushButton::clicked, this, &NotificationsHandler::onClearOldClicked);
    
    // Устанавливаем фильтр "Все уведомления" по умолчанию
    ui->filterCombo->setCurrentIndex(0); // Индекс 0 = "Все уведомления"

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
    
    // Сначала подсчитаем количество уведомлений
    int totalCount = 0;
    while (query.next()) {
        totalCount++;
    }
    qDebug() << "Total notifications found:" << totalCount;
    
    // Сбрасываем курсор в начало
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
            (type == "rental"       && status == "одобрено") ||
            (type == "purchase"     && status == "одобрено") ||
            (type == "order"        && status == "одобрено")) {

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
        qDebug() << "No notifications found for user" << user_id;
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
    } else {
        qDebug() << "Found notifications for user" << user_id;
    }

    QTimer::singleShot(100, this, [this]() {
        ui->scrollArea->verticalScrollBar()->setValue(0);
    });

    // MarkNotificationsAsReaded(user_id);
}

QVariant NotificationsHandler::getNewNotifications(const int user_id) {
    qDebug() << "getNewNotifications: user_id =" << user_id << "filter =" << m_current_filter;
    
    QString baseQuery = QString("SELECT 'service' as type, id, status, service_type as additional_info, scheduled_date as date_info, car_id FROM service_requests "
                "WHERE client_id = %1 AND (notification_shown = false OR notification_shown IS NULL) "
                "UNION ALL "
                "SELECT 'insurance' as type, id, status, insurance_type as additional_info, created_at as date_info, car_id FROM insurance_requests "
                "WHERE client_id = %1 AND (notification_shown = false OR notification_shown IS NULL) "
                "UNION ALL "
                "SELECT 'loan' as type, id, status, CAST(loan_amount AS TEXT) as additional_info, created_at as date_info, car_id FROM loan_requests "
                "WHERE client_id = %1 AND (notification_shown = false OR notification_shown IS NULL) "
                "UNION ALL "
                "SELECT 'test_drive' as type, id, status, 'Тест-драйв' as additional_info, scheduled_date as date_info, car_id FROM test_drives "
                "WHERE client_id = %1 AND (notification_shown = false OR notification_shown IS NULL) "
                "UNION ALL "
                "SELECT 'rental' as type, id, status, CAST(rental_days AS TEXT) as additional_info, start_date as date_info, car_id FROM rental_requests "
                "WHERE client_id = %1 AND (notification_shown = false OR notification_shown IS NULL) "
                "UNION ALL "
                "SELECT 'purchase' as type, id, status, 'Покупка автомобиля' as additional_info, created_at as date_info, car_id FROM purchase_requests "
                "WHERE client_id = %1 AND (notification_shown = false OR notification_shown IS NULL) "
                "UNION ALL "
                "SELECT 'order' as type, id, status, car_name as additional_info, created_at as date_info, 0 as car_id FROM order_requests "
                "WHERE client_id = %1 AND (notification_shown = false OR notification_shown IS NULL)").arg(user_id);

    // Добавляем фильтры
    QString filteredQuery = baseQuery;
    
    if (m_current_filter == "Только новые") {
        // Для фильтра "только новые" применяем к каждой таблице
        filteredQuery = QString("SELECT 'service' as type, id, status, service_type as additional_info, scheduled_date as date_info, car_id FROM service_requests "
                    "WHERE client_id = %1 AND notification_shown = false "
                    "UNION ALL "
                    "SELECT 'insurance' as type, id, status, insurance_type as additional_info, created_at as date_info, car_id FROM insurance_requests "
                    "WHERE client_id = %1 AND notification_shown = false "
                    "UNION ALL "
                    "SELECT 'loan' as type, id, status, CAST(loan_amount AS TEXT) as additional_info, created_at as date_info, car_id FROM loan_requests "
                    "WHERE client_id = %1 AND notification_shown = false "
                    "UNION ALL "
                    "SELECT 'test_drive' as type, id, status, 'Тест-драйв' as additional_info, scheduled_date as date_info, car_id FROM test_drives "
                    "WHERE client_id = %1 AND notification_shown = false "
                    "UNION ALL "
                    "SELECT 'rental' as type, id, status, CAST(rental_days AS TEXT) as additional_info, start_date as date_info, car_id FROM rental_requests "
                    "WHERE client_id = %1 AND notification_shown = false "
                    "UNION ALL "
                    "SELECT 'purchase' as type, id, status, 'Покупка автомобиля' as additional_info, created_at as date_info, car_id FROM purchase_requests "
                    "WHERE client_id = %1 AND notification_shown = false "
                    "UNION ALL "
                    "SELECT 'order' as type, id, status, car_name as additional_info, created_at as date_info, 0 as car_id FROM order_requests "
                    "WHERE client_id = %1 AND notification_shown = false").arg(user_id);
    } else if (m_current_filter == "Только одобренные") {
        // Для фильтра "только одобренные" применяем к каждой таблице
        filteredQuery = QString("SELECT 'service' as type, id, status, service_type as additional_info, scheduled_date as date_info, car_id FROM service_requests "
                    "WHERE client_id = %1 AND status IN ('одобрено', 'подтверждено') "
                    "UNION ALL "
                    "SELECT 'insurance' as type, id, status, insurance_type as additional_info, created_at as date_info, car_id FROM insurance_requests "
                    "WHERE client_id = %1 AND status IN ('одобрено', 'подтверждено') "
                    "UNION ALL "
                    "SELECT 'loan' as type, id, status, CAST(loan_amount AS TEXT) as additional_info, created_at as date_info, car_id FROM loan_requests "
                    "WHERE client_id = %1 AND status IN ('одобрено', 'подтверждено') "
                    "UNION ALL "
                    "SELECT 'test_drive' as type, id, status, 'Тест-драйв' as additional_info, scheduled_date as date_info, car_id FROM test_drives "
                    "WHERE client_id = %1 AND status IN ('одобрено', 'подтверждено') "
                    "UNION ALL "
                    "SELECT 'rental' as type, id, status, CAST(rental_days AS TEXT) as additional_info, start_date as date_info, car_id FROM rental_requests "
                    "WHERE client_id = %1 AND status IN ('одобрено', 'подтверждено') "
                    "UNION ALL "
                    "SELECT 'purchase' as type, id, status, 'Покупка автомобиля' as additional_info, created_at as date_info, car_id FROM purchase_requests "
                    "WHERE client_id = %1 AND status IN ('одобрено', 'подтверждено') "
                    "UNION ALL "
                    "SELECT 'order' as type, id, status, car_name as additional_info, created_at as date_info, 0 as car_id FROM order_requests "
                    "WHERE client_id = %1 AND status IN ('одобрено', 'подтверждено')").arg(user_id);
    } else if (m_current_filter == "Последние 7 дней") {
        // Для фильтра "последние 7 дней" применяем к правильным колонкам дат
        filteredQuery = QString("SELECT 'service' as type, id, status, service_type as additional_info, scheduled_date as date_info, car_id FROM service_requests "
                    "WHERE client_id = %1 AND scheduled_date >= NOW() - INTERVAL '7 days' "
                    "UNION ALL "
                    "SELECT 'insurance' as type, id, status, insurance_type as additional_info, created_at as date_info, car_id FROM insurance_requests "
                    "WHERE client_id = %1 AND created_at >= NOW() - INTERVAL '7 days' "
                    "UNION ALL "
                    "SELECT 'loan' as type, id, status, CAST(loan_amount AS TEXT) as additional_info, created_at as date_info, car_id FROM loan_requests "
                    "WHERE client_id = %1 AND created_at >= NOW() - INTERVAL '7 days' "
                    "UNION ALL "
                    "SELECT 'test_drive' as type, id, status, 'Тест-драйв' as additional_info, scheduled_date as date_info, car_id FROM test_drives "
                    "WHERE client_id = %1 AND scheduled_date >= NOW() - INTERVAL '7 days' "
                    "UNION ALL "
                    "SELECT 'rental' as type, id, status, CAST(rental_days AS TEXT) as additional_info, start_date as date_info, car_id FROM rental_requests "
                    "WHERE client_id = %1 AND start_date >= NOW() - INTERVAL '7 days' "
                    "UNION ALL "
                    "SELECT 'purchase' as type, id, status, 'Покупка автомобиля' as additional_info, created_at as date_info, car_id FROM purchase_requests "
                    "WHERE client_id = %1 AND created_at >= NOW() - INTERVAL '7 days' "
                    "UNION ALL "
                    "SELECT 'order' as type, id, status, car_name as additional_info, created_at as date_info, 0 as car_id FROM order_requests "
                    "WHERE client_id = %1 AND created_at >= NOW() - INTERVAL '7 days'").arg(user_id);
    } else if (m_current_filter == "Последние 30 дней") {
        // Для фильтра "последние 30 дней" применяем к правильным колонкам дат
        filteredQuery = QString("SELECT 'service' as type, id, status, service_type as additional_info, scheduled_date as date_info, car_id FROM service_requests "
                    "WHERE client_id = %1 AND scheduled_date >= NOW() - INTERVAL '30 days' "
                    "UNION ALL "
                    "SELECT 'insurance' as type, id, status, insurance_type as additional_info, created_at as date_info, car_id FROM insurance_requests "
                    "WHERE client_id = %1 AND created_at >= NOW() - INTERVAL '30 days' "
                    "UNION ALL "
                    "SELECT 'loan' as type, id, status, CAST(loan_amount AS TEXT) as additional_info, created_at as date_info, car_id FROM loan_requests "
                    "WHERE client_id = %1 AND created_at >= NOW() - INTERVAL '30 days' "
                    "UNION ALL "
                    "SELECT 'test_drive' as type, id, status, 'Тест-драйв' as additional_info, scheduled_date as date_info, car_id FROM test_drives "
                    "WHERE client_id = %1 AND scheduled_date >= NOW() - INTERVAL '30 days' "
                    "UNION ALL "
                    "SELECT 'rental' as type, id, status, CAST(rental_days AS TEXT) as additional_info, start_date as date_info, car_id FROM rental_requests "
                    "WHERE client_id = %1 AND start_date >= NOW() - INTERVAL '30 days' "
                    "UNION ALL "
                    "SELECT 'purchase' as type, id, status, 'Покупка автомобиля' as additional_info, created_at as date_info, car_id FROM purchase_requests "
                    "WHERE client_id = %1 AND created_at >= NOW() - INTERVAL '30 days' "
                    "UNION ALL "
                    "SELECT 'order' as type, id, status, car_name as additional_info, created_at as date_info, 0 as car_id FROM order_requests "
                    "WHERE client_id = %1 AND created_at >= NOW() - INTERVAL '30 days'").arg(user_id);
    }

    filteredQuery += " ORDER BY date_info DESC";

    qDebug() << "Executing query:" << filteredQuery;
    QVariant result = m_database_handler.lock()->ExecuteSelectQuery(filteredQuery);
    
    return result;
}

void NotificationsHandler::markNotificationsAsReaded(const int user_id) {
    qDebug() << "markNotificationsAsReaded: user_id =" << user_id;
    
    // Помечаем как прочитанные только те уведомления, которые были показаны
    // (т.е. те, которые имеют notification_shown = false или NULL)
    QStringList tables = {"service_requests", "insurance_requests", "loan_requests", 
                         "test_drives", "rental_requests", "purchase_requests", "order_requests"};
    
    for (const QString& table : tables) {
        QString updateQuery = QString("UPDATE %1 SET notification_shown = true WHERE client_id = %2 AND (notification_shown = false OR notification_shown IS NULL)")
            .arg(table)
            .arg(user_id);
        
        qDebug() << "Executing:" << updateQuery;
        QVariant result = m_database_handler.lock()->ExecuteQuery(updateQuery);
        qDebug() << "Result for" << table << ":" << result.toBool();
    }
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
    // Создаем красивый виджет уведомления в стиле диалогов выбора услуг
    QWidget *notificationWidget = new QWidget();
    notificationWidget->setFixedHeight(120);
    
    QHBoxLayout *mainLayout = new QHBoxLayout(notificationWidget);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(15);
    
    // Иконка уведомления
    QLabel *iconLabel = new QLabel("🔔");
    iconLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 24px;"
        "    color: #2196F3;"
        "    background: #e3f2fd;"
        "    border-radius: 20px;"
        "    padding: 10px;"
        "    min-width: 40px;"
        "    max-width: 40px;"
        "    min-height: 40px;"
        "    max-height: 40px;"
        "}"
    );
    iconLabel->setAlignment(Qt::AlignCenter);
    
    // Основной контент
    QVBoxLayout *contentLayout = new QVBoxLayout();
    contentLayout->setSpacing(5);
    
    // Заголовок уведомления
    QLabel *titleLabel = new QLabel(title.toString());
    titleLabel->setStyleSheet(
        "QLabel {"
        "    font: 700 14pt 'JetBrains Mono';"
        "    color: #1d1b20;"
        "    margin-bottom: 5px;"
        "}"
    );
    
    // Текст уведомления
    QLabel *textLabel = new QLabel(text.toString());
    textLabel->setWordWrap(true);
    textLabel->setStyleSheet(
        "QLabel {"
        "    font: 11pt 'JetBrains Mono';"
        "    color: #666666;"
        "    line-height: 1.4;"
        "}"
    );
    
    // Дата уведомления
    QLabel *dateLabel = new QLabel(date.toString());
    dateLabel->setStyleSheet(
        "QLabel {"
        "    font: 10pt 'JetBrains Mono';"
        "    color: #999999;"
        "    margin-top: 5px;"
        "}"
    );
    dateLabel->setProperty("sortDate", date.toString());
    
    contentLayout->addWidget(titleLabel);
    contentLayout->addWidget(textLabel);
    contentLayout->addWidget(dateLabel);
    contentLayout->addStretch();
    
    // Кнопка действия (опционально)
    QPushButton *actionButton = new QPushButton("📄");
    actionButton->setFixedSize(35, 35);
    actionButton->setStyleSheet(
        "QPushButton {"
        "    background: #f5f5f5;"
        "    border: 2px solid #e0e0e0;"
        "    border-radius: 17px;"
        "    font-size: 16px;"
        "}"
        "QPushButton:hover {"
        "    background: #e3f2fd;"
        "    border: 2px solid #2196F3;"
        "}"
        "QPushButton:pressed {"
        "    background: #bbdefb;"
        "}"
    );
    actionButton->setToolTip("Создать договор");
    
    // Соединяем кнопку с генерацией договора
    connect(actionButton, &QPushButton::clicked, [this, title]() {
        // Здесь можно добавить логику для генерации договора
        QMessageBox::information(this, "Договор", "Функция создания договора будет реализована");
    });
    
    mainLayout->addWidget(iconLabel);
    mainLayout->addLayout(contentLayout, 1);
    mainLayout->addWidget(actionButton);
    
    // Стиль для всего виджета уведомления
    notificationWidget->setStyleSheet(
        "QWidget {"
        "    background: #ffffff;"
        "    border: 2px solid #e0e0e0;"
        "    border-radius: 12px;"
        "    margin: 8px 0px;"
        "}"
        "QWidget:hover {"
        "    border: 2px solid #2196F3;"
        "    background: #fafafa;"
        "}"
    );
    
    // Сохраняем дату для сортировки
    QDateTime notificationDate = QDateTime::fromString(date.toString(), "dd.MM.yyyy");
    notificationWidget->setProperty("notificationDate", notificationDate);
    
    m_notifications_layout->addWidget(notificationWidget);
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

void NotificationsHandler::onClearOldClicked()
{
    if (m_current_user_id == -1) return;
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Подтверждение", 
        "Скрыть все уведомления?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // Список таблиц для скрытия всех уведомлений
        QStringList tables = {"service_requests", "insurance_requests", "loan_requests", 
                             "test_drives", "rental_requests", "purchase_requests", "order_requests"};
        
        int totalHidden = 0;
        bool allSuccess = true;
        
        // Скрываем все уведомления в каждой таблице отдельно
        for (const QString& table : tables) {
            // Сначала считаем количество записей для скрытия
            QSqlQuery countQuery;
            QString countQueryStr = QString("SELECT COUNT(*) FROM %1 WHERE client_id = %2 AND (notification_shown = false OR notification_shown IS NULL)")
                .arg(table)
                .arg(m_current_user_id);
            
            int recordsToHide = 0;
            if (countQuery.exec(countQueryStr) && countQuery.next()) {
                recordsToHide = countQuery.value(0).toInt();
            }
            
            if (recordsToHide > 0) {
                QString updateQuery = QString("UPDATE %1 SET notification_shown = true WHERE client_id = %2 AND (notification_shown = false OR notification_shown IS NULL)")
                    .arg(table)
                    .arg(m_current_user_id);
                
                qDebug() << "Executing update query for" << table << ":" << updateQuery;
                qDebug() << "Records to hide from" << table << ":" << recordsToHide;
                
                QVariant result = m_database_handler.lock()->ExecuteQuery(updateQuery);
                if (result.toBool()) {
                    totalHidden += recordsToHide;
                    qDebug() << "Successfully hidden" << recordsToHide << "records from" << table;
                } else {
                    qDebug() << "Failed to hide records from" << table;
                    allSuccess = false;
                }
            } else {
                qDebug() << "No records to hide found in" << table;
            }
        }
        
        if (totalHidden > 0) {
            if (allSuccess) {
                QMessageBox::information(this, "Успех", QString("Скрыто %1 уведомлений").arg(totalHidden));
            } else {
                QMessageBox::warning(this, "Предупреждение", QString("Скрыто %1 уведомлений, но некоторые не удалось скрыть. Проверьте логи для подробностей.").arg(totalHidden));
            }
            loadAndShowNotifications(m_current_user_id);
        } else {
            QMessageBox::information(this, "Информация", "Нет уведомлений для скрытия.");
        }
    }
}

