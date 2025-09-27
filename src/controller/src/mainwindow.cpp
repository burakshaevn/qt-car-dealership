#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSpinBox>
#include <QCalendarWidget>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_database_handler.reset(new DatabaseHandler);
    m_database_handler->LoadDefault();

    // Переключаем пользователя на экран логина после запуска
    ui->stackedWidget->setCurrentWidget(ui->login);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::UpdateUser(const UserInfo& user, QWidget* parent)
{
    m_user.reset(new User(user, parent));
}

void MainWindow::on_pushButton_login_clicked()
{
    auto queryResult = m_database_handler->ExecuteSelectQuery(QString("SELECT * FROM public.admins WHERE username = '%1';").arg(ui->lineEdit_login->text()));

    if (queryResult.canConvert<QSqlQuery>())
    {
        QSqlQuery query = queryResult.value<QSqlQuery>();
        if (query.next())
        {
            UserInfo user;
            user.id_ = query.value("id").toInt();
            user.password_ = query.value("password").toString();
            user.role_ = Role::Admin;
            if (user.password_ == ui->lineEdit_password->text())
            {
                ui->lineEdit_login->clear();
                ui->lineEdit_password->clear();
                QMessageBox::information(this, "Авторизация", "Выполнена авторизация как администратор.");
                UpdateUser(user, this);

                m_table.reset(new Table(m_database_handler, m_user.get(), nullptr));
                m_table->BuildAdminTables();

                connect(m_table.get(), &Table::Logout, this, &MainWindow::on_pushButton_logout_clicked);

                ui->stackedWidget->addWidget(m_table.get());
                ui->stackedWidget->setCurrentWidget(m_table.get());
            }
            else
            {
                QMessageBox::critical(this, "Авторизация", "Неверный логин или пароль.");
            }
        }
        else
        {
            auto queryResult = m_database_handler->ExecuteSelectQuery(QString("SELECT * FROM public.clients WHERE email = '%1';").arg(ui->lineEdit_login->text()));

            if (queryResult.canConvert<QSqlQuery>())
            {
                QSqlQuery query = queryResult.value<QSqlQuery>();
                if (query.next())
                {
                    UserInfo user;
                    user.id_ = query.value("id").toInt();
                    user.full_name_ = query.value("first_name").toString();
                    user.full_name_ += " " + query.value("last_name").toString();
                    user.email_ = query.value("email").toString();
                    user.password_ = query.value("password").toString();
                    user.role_ = Role::User;
                    user.products_ = GetPurchasedProducts(user.id_);

                    // Хешируем введенный пароль для сравнения
                    QString hashedInputPassword = QString(QCryptographicHash::hash(
                        ui->lineEdit_password->text().toUtf8(),
                        QCryptographicHash::Sha256).toHex());

                    if (user.password_ == hashedInputPassword)
                    {
                        ui->lineEdit_login->clear();
                        ui->lineEdit_password->clear();
                        QMessageBox::information(this, "Авторизация", "Выполнена авторизация как пользователь.");

                        BuildDependencies();
                        UpdateUser(user, this);

                        m_products->PullProducts();

                        m_product_cards->UpdateProductsWidget(ui->scrollArea_catalog, QString("Смотреть всё"), QString("Белый"));
                        
                        if (m_floating_widget) {
                            m_floating_widget->setVisible(true);
                        }
                        
                        ui->stackedWidget->setCurrentWidget(ui->main);
                    }
                    else
                    {
                        QMessageBox::critical(this, "Авторизация", "Неверный логин или пароль.");
                    }
                }
                else
                {
                    QMessageBox::critical(this, "Авторизация", "Неверный логин или пароль.");
                }
            }
        }
    }
}

void MainWindow::on_pushButton_logout_clicked()
{
    if (!this->ui->stackedWidget) return;

    if (m_user  && m_user->GetRole() == Role::User) {
        m_products.reset();
        m_product_cards.reset();
        m_database_handler.reset();
        m_floating_widget.reset();

        if (ui->scrollArea_catalog && ui->scrollArea_catalog->widget()) {
            QWidget* oldWidget = ui->scrollArea_catalog->takeWidget();
            if (oldWidget) {
                oldWidget->deleteLater();
            }
        }

        if (ui->scrollArea_purchased_cars && ui->scrollArea_purchased_cars->widget()) {
            QWidget* oldWidget = ui->scrollArea_purchased_cars->takeWidget();
            if (oldWidget) {
                oldWidget->deleteLater();
            }
        }
    }

    m_user.reset();
    m_table.reset();
    
    if (this->ui->stackedWidget && this->ui->login) {
        this->ui->stackedWidget->setCurrentWidget(this->ui->login);
    }
}

QList<Products::ProductKey> MainWindow::GetPurchasedProducts(int m_userid) const {
    QList<Products::ProductKey> m_products; // Список названий купленных товаров

    QSqlQuery query;
    QString query_str = QString(
        "SELECT c.name, c.color "
        "FROM cars c "
        "INNER JOIN purchases p ON c.id = p.car_id "
        "WHERE p.client_id = %1"
    ).arg(m_userid);

    qDebug() << "GetPurchasedProducts: Executing query for user" << m_userid << ":" << query_str;

    if (!query.exec(query_str)) {
        qDebug() << "GetPurchasedProducts: Query failed:" << query.lastError().text();
        return m_products;
    }

    int count = 0;
    while (query.next()) {
        QString name = query.value("name").toString();
        QString color = query.value("color").toString();
        Products::ProductKey key = std::make_tuple(name, color);
        m_products.append(key);
        count++;
        qDebug() << "GetPurchasedProducts: Found purchase" << count << ":" << name << color;
    }

    qDebug() << "GetPurchasedProducts: Total purchases found:" << count;
    return m_products;
}

void MainWindow::BuildDependencies() {
    if (!m_database_handler) {
        m_database_handler.reset(new DatabaseHandler);
        m_database_handler->LoadDefault();
    }

    if (!m_floating_widget) {
        SetupFloatingMenu();
    }

    if (!m_product_cards && !m_products) {
        m_product_cards.reset(new ProductCard(m_database_handler, nullptr, this));
        m_products.reset(new Products(m_product_cards, m_database_handler));

        connect(m_products.get(), &Products::OpenInfoPage, this, [this](const ProductInfo& product) {
            auto m_current_productcolors_ = m_products->GetAllProductsWithName(product);
            ShowProductOnPersonalPage(product, m_current_productcolors_);
        });

        // Устанавливаем связь между ProductCard и Instruments
        m_product_cards->SetProductsPtr(m_products);
    }

    // Устанавливаем фильтр событий для панели уведомлений
    m_notifications_handler.reset(new NotificationsHandler(m_database_handler, this));
    // ui->notifications_panel->installEventFilter(this);
}

void MainWindow::SetupServicesScrollArea()
{
    ui->scrollArea_services->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->scrollArea_services->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea_services->setWidgetResizable(false);
    ui->scrollArea_services->setFocusPolicy(Qt::WheelFocus);

    // Стили для QScrollArea с появлением скроллбара при наведении
    ui->scrollArea_services->setStyleSheet(
        "QScrollArea {"
        "    background: transparent;"
        "    border: none;"
        "}"
        "QScrollArea > QWidget {"
        "    background: transparent;"
        "}"
        "QScrollArea::viewport {"
        "    background: transparent;"
        "}"
        "QScrollBar:horizontal {"
        "    height: 15px;"
        "    background-color: transparent;"
        "    margin: 3px 0px 3px 0px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "    background-color: transparent;"
        "    min-width: 20px;"
        "    border-radius: 7px;"
        "}"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
        "    width: 0px;"
        "    border: none;"
        "    background: none;"
        "}"
        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {"
        "    background: none;"
        "}"
        "QScrollArea:hover QScrollBar:horizontal {"
        "    background-color: #f0f0f0;"
        "}"
        "QScrollArea:hover QScrollBar::handle:horizontal {"
        "    background-color: #9b9c9c;"
        "}"
    );

    // Создаём контейнер для карточек
    QWidget* container = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(container);
    layout->setAlignment(Qt::AlignLeft);
    layout->setSpacing(21);
    layout->setContentsMargins(0, 0, 0, 0);

    // Список названий карточек
    QStringList services = { "Обслуживание", "Аренда", "Кредитование", "Страхование" };

    // Создаём карточки
    for (const QString& service : services)
    {
        QPushButton* card = new QPushButton(container);
        card->setFixedSize(360, 175);
        card->setStyleSheet(
            "QPushButton {"
            "    background-color: #fafafa;"
            "    border-radius: 39px;"
            "    border: none;"
            "    transition: background-color 0.2s;"
            "}"
            "QPushButton:hover {"
            "    background-color: #e0e0e0;"
            "}"
            );

        QLabel* label = new QLabel(service, card);
        label->setStyleSheet(
            "color: #1d1b20;"
            "font: 700 20pt 'JetBrains Mono';"
            "background: transparent;"
            );
        label->move(14, 105);
        label->setAlignment(Qt::AlignLeft);

        // Connect click event
        connect(card, &QPushButton::clicked, this, [this, service]() {
            if (!m_user)
            {
                QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для доступа к этому разделу.");
                return;
            }

            if (service == "Обслуживание")
            {
                QDialog dialog(this);
                dialog.setWindowTitle("Заявка на сервисное обслуживание");
                dialog.setFixedSize(500, 600);
                dialog.setStyleSheet(
                    "QDialog {"
                    "    background-color: #ffffff;"
                    "}"
                    "QLabel {"
                    "    color: #1d1b20;"
                    "    font: 500 12pt 'JetBrains Mono';"
                    "    margin-top: 10px;"
                    "}"
                    "QComboBox, QLineEdit, QTimeEdit {"
                    "    padding: 8px;"
                    "    border: 2px solid #e0e0e0;"
                    "    border-radius: 8px;"
                    "    background: #fafafa;"
                    "    font: 11pt 'JetBrains Mono';"
                    "    min-height: 30px;"
                    "}"
                    "QComboBox:focus, QLineEdit:focus, QTimeEdit:focus {"
                    "    border: 2px solid #2196F3;"
                    "}"
                    "QPushButton {"
                    "    padding: 10px 20px;"
                    "    border-radius: 8px;"
                    "    font: 600 11pt 'JetBrains Mono';"
                    "    min-width: 100px;"
                    "}"
                    "QPushButton[type='primary'] {"
                    "    background-color: #2196F3;"
                    "    color: white;"
                    "    border: none;"
                    "}"
                    "QPushButton[type='primary']:hover {"
                    "    background-color: #1976D2;"
                    "}"
                    "QPushButton[type='secondary'] {"
                    "    background-color: #fafafa;"
                    "    color: #1d1b20;"
                    "    border: 2px solid #e0e0e0;"
                    "}"
                    "QPushButton[type='secondary']:hover {"
                    "    background-color: #e0e0e0;"
                    "}"
                    "QCalendarWidget {"
                    "    background-color: white;"
                    "    font: 10pt 'JetBrains Mono';"
                    "}"
                    "QCalendarWidget QToolButton {"
                    "    color: #1d1b20;"
                    "    font: 10pt 'JetBrains Mono';"
                    "    padding: 5px;"
                    "}"
                    "QCalendarWidget QMenu {"
                    "    background-color: white;"
                    "    font: 10pt 'JetBrains Mono';"
                    "}"
                );

                QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);
                dialogLayout->setSpacing(15);
                dialogLayout->setContentsMargins(30, 30, 30, 30);

                // Заголовок
                QLabel* titleLabel = new QLabel("Запись на сервисное обслуживание", &dialog);
                titleLabel->setStyleSheet("font: 700 16pt 'JetBrains Mono'; margin-bottom: 20px;");
                dialogLayout->addWidget(titleLabel);

                // Выбор автомобиля
                QLabel* carLabel = new QLabel("Выберите автомобиль:", &dialog);
                dialogLayout->addWidget(carLabel);

                QComboBox* carCombo = new QComboBox(&dialog);
                QSqlQuery carQuery;
                QString queryStr = "SELECT MIN(id) AS id, name FROM cars GROUP BY name ORDER BY name";
                if (!carQuery.exec(queryStr)) {
                    QMessageBox::critical(this, "Ошибка", "Не удалось загрузить список автомобилей: " + carQuery.lastError().text());
                    return;
                }
                while (carQuery.next()) {
                    carCombo->addItem(carQuery.value("name").toString(), carQuery.value("id").toInt());
                }
                dialogLayout->addWidget(carCombo);

                // Тип обслуживания
                QLabel* serviceLabel = new QLabel("Тип обслуживания:", &dialog);
                dialogLayout->addWidget(serviceLabel);

                QLineEdit* serviceTypeEdit = new QLineEdit(&dialog);
                serviceTypeEdit->setPlaceholderText("Например: замена масла, диагностика");
                dialogLayout->addWidget(serviceTypeEdit);

                // Выбор времени
                QLabel* timeLabel = new QLabel("Выберите время:", &dialog);
                dialogLayout->addWidget(timeLabel);

                QTimeEdit* timeEdit = new QTimeEdit(&dialog);
                timeEdit->setDisplayFormat("HH:mm");
                timeEdit->setTime(QTime(9, 0));
                dialogLayout->addWidget(timeEdit);

                // Выбор даты
                QLabel* dateLabel = new QLabel("Выберите дату:", &dialog);
                dialogLayout->addWidget(dateLabel);

                QCalendarWidget* calendar = new QCalendarWidget(&dialog);
                calendar->setMinimumDate(QDate::currentDate());
                dialogLayout->addWidget(calendar);

                // Кнопки
                QHBoxLayout* buttonLayout = new QHBoxLayout();
                buttonLayout->setSpacing(15);

                QPushButton* okButton = new QPushButton("Подтвердить", &dialog);
                okButton->setProperty("type", "primary");

                QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
                cancelButton->setProperty("type", "secondary");

                buttonLayout->addWidget(cancelButton);
                buttonLayout->addWidget(okButton);
                dialogLayout->addLayout(buttonLayout);

                bool accepted = false;
                connect(okButton, &QPushButton::clicked, [&]() {
                    if (serviceTypeEdit->text().isEmpty()) {
                        QMessageBox::warning(&dialog, "Ошибка", "Укажите тип обслуживания.");
                        return;
                    }
                    accepted = true;
                    dialog.accept();
                });
                connect(cancelButton, &QPushButton::clicked, [&]() { dialog.reject(); });

                if (dialog.exec() == QDialog::Accepted && accepted) {
                    QString dateTime = calendar->selectedDate().toString("yyyy-MM-dd") + " " + timeEdit->time().toString("HH:mm");
                    QSqlQuery query;
                    QString queryStr = QString(
                                           "INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                                           "VALUES (%1, %2, '%3', '%4', 'не обработано');")
                                           .arg(m_user->GetId())
                                           .arg(carCombo->currentData().toInt())
                                           .arg(serviceTypeEdit->text())
                                           .arg(dateTime);

                    if (query.exec(queryStr)) {
                        QMessageBox::information(this, "Успех", "Заявка на обслуживание подана.");
                    } else {
                        QMessageBox::critical(this, "Ошибка", "Не удалось подать заявку: " + query.lastError().text());
                    }
                }
            }
            else if (service == "Аренда")
            {
                QDialog dialog(this);
                dialog.setWindowTitle("Заявка на аренду");
                dialog.setFixedSize(500, 400);
                dialog.setStyleSheet(
                    "QDialog {"
                    "    background-color: #ffffff;"
                    "}"
                    "QLabel {"
                    "    color: #1d1b20;"
                    "    font: 500 12pt 'JetBrains Mono';"
                    "}"
                    "QLabel[type='header'] {"
                    "    font: 700 16pt 'JetBrains Mono';"
                    "    padding-bottom: 20px;"
                    "}"
                    "QComboBox, QSpinBox {"
                    "    padding: 8px;"
                    "    border: 2px solid #e0e0e0;"
                    "    border-radius: 8px;"
                    "    background: #fafafa;"
                    "    font: 11pt 'JetBrains Mono';"
                    "    min-height: 30px;"
                    "    margin-bottom: 15px;"
                    "}"
                    "QComboBox:focus, QSpinBox:focus {"
                    "    border: 2px solid #2196F3;"
                    "}"
                    "QPushButton {"
                    "    padding: 10px 20px;"
                    "    border-radius: 8px;"
                    "    font: 600 11pt 'JetBrains Mono';"
                    "    min-width: 100px;"
                    "}"
                    "QPushButton[type='primary'] {"
                    "    background-color: #2196F3;"
                    "    color: white;"
                    "    border: none;"
                    "}"
                    "QPushButton[type='primary']:hover {"
                    "    background-color: #1976D2;"
                    "}"
                    "QPushButton[type='secondary'] {"
                    "    background-color: #fafafa;"
                    "    color: #1d1b20;"
                    "    border: 2px solid #e0e0e0;"
                    "}"
                    "QPushButton[type='secondary']:hover {"
                    "    background-color: #e0e0e0;"
                    "}"
                );

                QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);
                dialogLayout->setSpacing(10);
                dialogLayout->setContentsMargins(30, 30, 30, 30);

                // Заголовок
                QLabel* titleLabel = new QLabel("Аренда автомобиля", &dialog);
                titleLabel->setProperty("type", "header");
                dialogLayout->addWidget(titleLabel);

                // Выбор автомобиля
                QLabel* carLabel = new QLabel("Выберите автомобиль:", &dialog);
                dialogLayout->addWidget(carLabel);

                QComboBox* carCombo = new QComboBox(&dialog);
                QSqlQuery carQuery("SELECT id, CONCAT(name, ' (', color, ')') as display_name FROM cars WHERE available_for_rent = true ORDER BY name, color");
                while (carQuery.next()) {
                    carCombo->addItem(carQuery.value("display_name").toString(), carQuery.value("id").toInt());
                }
                dialogLayout->addWidget(carCombo);

                // Срок аренды
                QLabel* termLabel = new QLabel("Срок аренды:", &dialog);
                dialogLayout->addWidget(termLabel);

                QSpinBox* termEdit = new QSpinBox(&dialog);
                termEdit->setRange(1, 30);
                termEdit->setSuffix(" дней");
                dialogLayout->addWidget(termEdit);

                // Дата начала
                QLabel* dateLabel = new QLabel("Дата начала аренды:", &dialog);
                dialogLayout->addWidget(dateLabel);

                QCalendarWidget* calendar = new QCalendarWidget(&dialog);
                calendar->setMinimumDate(QDate::currentDate());
                dialogLayout->addWidget(calendar);

                // Растягивающийся элемент
                dialogLayout->addStretch();

                // Кнопки
                QHBoxLayout* buttonLayout = new QHBoxLayout();
                buttonLayout->setSpacing(15);

                QPushButton* okButton = new QPushButton("Подтвердить", &dialog);
                okButton->setProperty("type", "primary");

                QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
                cancelButton->setProperty("type", "secondary");

                buttonLayout->addWidget(cancelButton);
                buttonLayout->addWidget(okButton);
                dialogLayout->addLayout(buttonLayout);

                bool accepted = false;
                connect(okButton, &QPushButton::clicked, [&]() {
                    if (carCombo->currentText().isEmpty()) {
                        QMessageBox::warning(&dialog, "Ошибка", "Выберите автомобиль для аренды.");
                        return;
                    }
                    accepted = true;
                    dialog.accept();
                });
                connect(cancelButton, &QPushButton::clicked, [&]() { dialog.reject(); });

                if (dialog.exec() == QDialog::Accepted && accepted) {
                    QString queryStr = QString(
                        "INSERT INTO rental_requests (client_id, car_id, start_date, rental_days, status) "
                        "VALUES (%1, %2, '%3', %4, 'не обработано');")
                        .arg(m_user->GetId())
                        .arg(carCombo->currentData().toInt())
                        .arg(QDate::currentDate().toString("yyyy-MM-dd"))
                        .arg(termEdit->value());

                    QString errorMessage;
                    if (m_database_handler->ExecuteQueryWithUserMessage(queryStr, errorMessage)) {
                        QMessageBox::information(this, "✅ Успех",
                            QString("Заявка на аренду на %1 дней создана.\nДата начала: %2")
                                .arg(termEdit->value())
                                .arg(QDate::currentDate().toString("dd.MM.yyyy")));
                    } else {
                        QMessageBox::warning(this, "❌ Ошибка", errorMessage);
                    }
                }
            }
            else if (service == "Кредитование")
            {
                QDialog dialog(this);
                dialog.setWindowTitle("Заявка на кредитование");
                dialog.setFixedSize(500, 400);
                dialog.setStyleSheet(
                    "QDialog {"
                    "    background-color: #ffffff;"
                    "}"
                    "QLabel {"
                    "    color: #1d1b20;"
                    "    font: 500 12pt 'JetBrains Mono';"
                    "}"
                    "QLabel[type='header'] {"
                    "    font: 700 16pt 'JetBrains Mono';"
                    "    padding-bottom: 20px;"
                    "}"
                    "QComboBox, QSpinBox, QDoubleSpinBox {"
                    "    padding: 8px;"
                    "    border: 2px solid #e0e0e0;"
                    "    border-radius: 8px;"
                    "    background: #fafafa;"
                    "    font: 11pt 'JetBrains Mono';"
                    "    min-height: 30px;"
                    "    margin-bottom: 15px;"
                    "}"
                    "QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus {"
                    "    border: 2px solid #2196F3;"
                    "}"
                    "QPushButton {"
                    "    padding: 10px 20px;"
                    "    border-radius: 8px;"
                    "    font: 600 11pt 'JetBrains Mono';"
                    "    min-width: 100px;"
                    "}"
                    "QPushButton[type='primary'] {"
                    "    background-color: #2196F3;"
                    "    color: white;"
                    "    border: none;"
                    "}"
                    "QPushButton[type='primary']:hover {"
                    "    background-color: #1976D2;"
                    "}"
                    "QPushButton[type='secondary'] {"
                    "    background-color: #fafafa;"
                    "    color: #1d1b20;"
                    "    border: 2px solid #e0e0e0;"
                    "}"
                    "QPushButton[type='secondary']:hover {"
                    "    background-color: #e0e0e0;"
                    "}"
                );

                QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);
                dialogLayout->setSpacing(10);
                dialogLayout->setContentsMargins(30, 30, 30, 30);

                // Заголовок
                QLabel* titleLabel = new QLabel("Оформление кредита", &dialog);
                titleLabel->setProperty("type", "header");
                dialogLayout->addWidget(titleLabel);

                // Выбор автомобиля
                QLabel* carLabel = new QLabel("Выберите автомобиль:", &dialog);
                dialogLayout->addWidget(carLabel);

                QComboBox* carCombo = new QComboBox(&dialog);
                QSqlQuery carQuery("SELECT MIN(id) AS id, name FROM cars GROUP BY name ORDER BY name");
                while (carQuery.next()) {
                    carCombo->addItem(carQuery.value("name").toString(), carQuery.value("id").toInt());
                }
                dialogLayout->addWidget(carCombo);

                // Сумма кредита
                QLabel* amountLabel = new QLabel("Сумма кредита:", &dialog);
                dialogLayout->addWidget(amountLabel);

                QDoubleSpinBox* amountEdit = new QDoubleSpinBox(&dialog);
                amountEdit->setRange(100000, 50000000);
                amountEdit->setSuffix(" руб.");
                amountEdit->setSpecialValueText("Выберите сумму");
                dialogLayout->addWidget(amountEdit);

                // Срок кредита
                QLabel* termLabel = new QLabel("Срок кредита:", &dialog);
                dialogLayout->addWidget(termLabel);

                QSpinBox* termEdit = new QSpinBox(&dialog);
                termEdit->setRange(1, 360);
                termEdit->setSuffix(" мес.");
                dialogLayout->addWidget(termEdit);

                // Растягивающийся элемент для заполнения пространства
                dialogLayout->addStretch();

                // Кнопки
                QHBoxLayout* buttonLayout = new QHBoxLayout();
                buttonLayout->setSpacing(15);

                QPushButton* okButton = new QPushButton("Подтвердить", &dialog);
                okButton->setProperty("type", "primary");

                QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
                cancelButton->setProperty("type", "secondary");

                buttonLayout->addWidget(cancelButton);
                buttonLayout->addWidget(okButton);
                dialogLayout->addLayout(buttonLayout);

                bool accepted = false;
                connect(okButton, &QPushButton::clicked, [&]() {
                    accepted = true;
                    dialog.accept();
                });
                connect(cancelButton, &QPushButton::clicked, [&]() { dialog.reject(); });

                if (dialog.exec() == QDialog::Accepted && accepted) {
                    // Проверяем наличие автомобиля на складе
                    int carId = carCombo->currentData().toInt();
                    QSqlQuery stockQuery;
                    stockQuery.prepare("SELECT name, color, stock_qty FROM cars WHERE id = :car_id");
                    stockQuery.bindValue(":car_id", carId);
                    
                    if (!stockQuery.exec() || !stockQuery.next()) {
                        QMessageBox::critical(this, "Ошибка", "Не удалось проверить наличие автомобиля");
                        return;
                    }
                    
                    QString carName = stockQuery.value(0).toString();
                    QString carColor = stockQuery.value(1).toString();
                    int stockQty = stockQuery.value(2).toInt();
                    
                    if (stockQty <= 0) {
                        QMessageBox::warning(this, "Автомобиль недоступен", 
                            QString("Автомобиль %1 (%2) временно недоступен для оформления кредита.\n\n"
                                   "Возможные варианты:\n"
                                   "• Оформить заявку на заказ автомобиля\n"
                                   "• Выбрать другой автомобиль")
                            .arg(carName)
                            .arg(carColor));
                        return;
                    }
                    
                    QSqlQuery query;
                    QString queryStr = QString(
                                           "INSERT INTO loan_requests (client_id, car_id, loan_amount, loan_term_months, status) "
                                           "VALUES (%1, %2, %3, %4, 'не обработано');")
                                           .arg(m_user->GetId())
                                           .arg(carId)
                                           .arg(amountEdit->value())
                                           .arg(termEdit->value());

                    if (query.exec(queryStr)) {
                        QMessageBox::information(this, "Успех", "Заявка на кредитование подана.");
                    } else {
                        QMessageBox::critical(this, "Ошибка", "Не удалось подать заявку: " + query.lastError().text());
                    }
                }
            }
            else if (service == "Страхование") {
                QDialog dialog(this);
                dialog.setWindowTitle("Заявка на страхование");
                dialog.setFixedSize(500, 400);
                dialog.setStyleSheet(
                    "QDialog {"
                    "    background-color: #ffffff;"
                    "}"
                    "QLabel {"
                    "    color: #1d1b20;"
                    "    font: 500 12pt 'JetBrains Mono';"
                    "}"
                    "QLabel[type='header'] {"
                    "    font: 700 16pt 'JetBrains Mono';"
                    "    padding-bottom: 20px;"
                    "}"
                    "QComboBox {"
                    "    padding: 8px;"
                    "    border: 2px solid #e0e0e0;"
                    "    border-radius: 8px;"
                    "    background: #fafafa;"
                    "    font: 11pt 'JetBrains Mono';"
                    "    min-height: 30px;"
                    "    margin-bottom: 15px;"
                    "}"
                    "QComboBox:focus {"
                    "    border: 2px solid #2196F3;"
                    "}"
                    "QPushButton {"
                    "    padding: 10px 20px;"
                    "    border-radius: 8px;"
                    "    font: 600 11pt 'JetBrains Mono';"
                    "    min-width: 100px;"
                    "}"
                    "QPushButton[type='primary'] {"
                    "    background-color: #2196F3;"
                    "    color: white;"
                    "    border: none;"
                    "}"
                    "QPushButton[type='primary']:hover {"
                    "    background-color: #1976D2;"
                    "}"
                    "QPushButton[type='secondary'] {"
                    "    background-color: #fafafa;"
                    "    color: #1d1b20;"
                    "    border: 2px solid #e0e0e0;"
                    "}"
                    "QPushButton[type='secondary']:hover {"
                    "    background-color: #e0e0e0;"
                    "}"
                );

                QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);
                dialogLayout->setSpacing(10);
                dialogLayout->setContentsMargins(30, 30, 30, 30);

                // Заголовок
                QLabel* titleLabel = new QLabel("Оформление страховки", &dialog);
                titleLabel->setProperty("type", "header");
                dialogLayout->addWidget(titleLabel);

                // Выбор автомобиля
                QLabel* carLabel = new QLabel("Выберите автомобиль:", &dialog);
                dialogLayout->addWidget(carLabel);

                QComboBox* carCombo = new QComboBox(&dialog);
                QSqlQuery carQuery("SELECT MIN(id) AS id, name FROM cars GROUP BY name ORDER BY name");
                while (carQuery.next()) {
                    carCombo->addItem(carQuery.value("name").toString(), carQuery.value("id").toInt());
                }
                dialogLayout->addWidget(carCombo);

                // Тип страховки
                QLabel* insuranceLabel = new QLabel("Тип страховки:", &dialog);
                dialogLayout->addWidget(insuranceLabel);

                QComboBox* insuranceTypeCombo = new QComboBox(&dialog);
                insuranceTypeCombo->addItems({"ОСАГО", "КАСКО", "Комплекс"});
                dialogLayout->addWidget(insuranceTypeCombo);

                // Растягивающийся элемент для заполнения пространства
                dialogLayout->addStretch();

                // Кнопки
                QHBoxLayout* buttonLayout = new QHBoxLayout();
                buttonLayout->setSpacing(15);

                QPushButton* okButton = new QPushButton("Подтвердить", &dialog);
                okButton->setProperty("type", "primary");

                QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
                cancelButton->setProperty("type", "secondary");

                buttonLayout->addWidget(cancelButton);
                buttonLayout->addWidget(okButton);
                dialogLayout->addLayout(buttonLayout);

                bool accepted = false;
                connect(okButton, &QPushButton::clicked, [&]() {
                    accepted = true;
                    dialog.accept();
                });
                connect(cancelButton, &QPushButton::clicked, [&]() { dialog.reject(); });

                if (dialog.exec() == QDialog::Accepted && accepted) {
                    QSqlQuery query;
                    QString queryStr = QString(
                                           "INSERT INTO insurance_requests (client_id, car_id, insurance_type, status) "
                                           "VALUES (%1, %2, '%3', 'не обработано');")
                                           .arg(m_user->GetId())
                                           .arg(carCombo->currentData().toInt())
                                           .arg(insuranceTypeCombo->currentText());

                    if (query.exec(queryStr)) {
                        QMessageBox::information(this, "Успех", "Заявка на страхование подана.");
                    } else {
                        QMessageBox::critical(this, "Ошибка", "Не удалось подать заявку: " + query.lastError().text());
                    }
                }
            }
        });

        layout->addWidget(card);
    }

    // Устанавливаем размер контейнера
    int containerWidth = (360 + layout->spacing()) * services.size();
    container->setMinimumWidth(containerWidth);
    container->setFixedHeight(175);

    // Устанавливаем контейнер в QScrollArea
    ui->scrollArea_services->setWidget(container);
}

// Service button handlers
void MainWindow::ServiceRequestHandler()
{
    if (!m_user)
    {
        QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для доступа к сервису.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Запись на обслуживание");
    dialog.setFixedSize(400, 300);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    // Service type selection
    QComboBox* serviceTypeCombo = new QComboBox(&dialog);
    serviceTypeCombo->addItems({"Техническое обслуживание", "Диагностика", "Ремонт"});
    layout->addWidget(new QLabel("Тип обслуживания:", &dialog));
    layout->addWidget(serviceTypeCombo);

    // Date selection
    QCalendarWidget* calendar = new QCalendarWidget(&dialog);
    calendar->setMinimumDate(QDate::currentDate());
    layout->addWidget(new QLabel("Дата:", &dialog));
    layout->addWidget(calendar);

    // Time selection
    QTimeEdit* timeEdit = new QTimeEdit(&dialog);
    timeEdit->setDisplayFormat("HH:mm");
    timeEdit->setTime(QTime(9, 0));
    layout->addWidget(new QLabel("Время:", &dialog));
    layout->addWidget(timeEdit);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton("OK", &dialog);
    QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(okButton);
    layout->addLayout(buttonLayout);

    bool accepted = false;
    connect(okButton, &QPushButton::clicked, [&]() {
        accepted = true;
        dialog.accept();
    });
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted && accepted)
    {
        QDateTime selectedDateTime(calendar->selectedDate(), timeEdit->time());
        QString serviceType = serviceTypeCombo->currentText();

        QSqlQuery query;
        QString queryStr = QString(
            "INSERT INTO service_requests (client_id, service_type, scheduled_date, status) "
            "VALUES (%1, '%2', '%3', 'не обработано');")
            .arg(m_user->GetId())
            .arg(serviceType)
            .arg(selectedDateTime.toString("yyyy-MM-dd HH:mm:ss"));

        if (query.exec(queryStr))
        {
            QMessageBox::information(this, "Успех",
                QString("Заявка на %1 создана на %2")
                    .arg(serviceType)
                    .arg(selectedDateTime.toString("dd.MM.yyyy HH:mm")));
        }
        else
        {
            QMessageBox::critical(this, "Ошибка",
                "Не удалось создать заявку: " + query.lastError().text());
        }
    }
}

void MainWindow::RentalRequestHandler() {
    if (!m_user)
    {
        QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для доступа к сервису аренды.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Заявка на аренду");
    dialog.setFixedSize(400, 400);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    // Car selection
    QComboBox* carCombo = new QComboBox(&dialog);
    QSqlQuery carQuery("SELECT id, CONCAT(name, ' (', color, ')') as display_name, stock_qty FROM cars WHERE available_for_rent = true AND stock_qty > 0 ORDER BY name, color");
    while (carQuery.next()) {
        QString displayName = carQuery.value("display_name").toString();
        int stockQty = carQuery.value("stock_qty").toInt();
        displayName += QString(" [В наличии: %1]").arg(stockQty);
        carCombo->addItem(displayName, carQuery.value("id").toInt());
    }
    layout->addWidget(new QLabel("Автомобиль:", &dialog));
    layout->addWidget(carCombo);

    // Rental period
    QSpinBox* daysSpinBox = new QSpinBox(&dialog);
    daysSpinBox->setRange(1, 30);
    daysSpinBox->setSuffix(" дней");
    layout->addWidget(new QLabel("Срок аренды:", &dialog));
    layout->addWidget(daysSpinBox);

    // Start date
    QCalendarWidget* calendar = new QCalendarWidget(&dialog);
    calendar->setMinimumDate(QDate::currentDate());
    layout->addWidget(new QLabel("Дата начала:", &dialog));
    layout->addWidget(calendar);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton("OK", &dialog);
    QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(okButton);
    layout->addLayout(buttonLayout);

    bool accepted = false;
    connect(okButton, &QPushButton::clicked, [&]() {
        accepted = true;
        dialog.accept();
    });
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted && accepted)
    {
        QDate startDate = calendar->selectedDate();
        int carId = carCombo->currentData().toInt();
        int rentalDays = daysSpinBox->value();

        // Проверяем наличие автомобиля на складе
        QSqlQuery stockQuery;
        stockQuery.prepare("SELECT name, color, stock_qty FROM cars WHERE id = :car_id");
        stockQuery.bindValue(":car_id", carId);
        
        if (!stockQuery.exec() || !stockQuery.next()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось проверить наличие автомобиля");
            return;
        }
        
        QString carName = stockQuery.value(0).toString();
        QString carColor = stockQuery.value(1).toString();
        int stockQty = stockQuery.value(2).toInt();
        
        if (stockQty <= 0) {
            QMessageBox::warning(this, "Автомобиль недоступен", 
                QString("Автомобиль %1 (%2) временно недоступен для аренды.\n\n"
                       "Возможные варианты:\n"
                       "• Оформить заявку на заказ автомобиля\n"
                       "• Выбрать другой автомобиль")
                .arg(carName)
                .arg(carColor));
            return;
        }

        QString queryStr = QString(
            "INSERT INTO rental_requests (client_id, car_id, start_date, rental_days, status) "
            "VALUES (%1, %2, '%3', %4, 'не обработано');")
            .arg(m_user->GetId())
            .arg(carId)
            .arg(startDate.toString("yyyy-MM-dd"))
            .arg(rentalDays);

        QString errorMessage;
        if (m_database_handler->ExecuteQueryWithUserMessage(queryStr, errorMessage))
        {
            QMessageBox::information(this, "✅ Успех",
                QString("Заявка на аренду на %1 дней создана.\nДата начала: %2")
                    .arg(rentalDays)
                    .arg(startDate.toString("dd.MM.yyyy")));
        }
        else
        {
            QMessageBox::warning(this, "❌ Ошибка", errorMessage);
        }
    }
}

void MainWindow::LoanRequestHandler()
{
    if (!m_user)
    {
        QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для доступа к кредитованию.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Заявка на кредит");
    dialog.setFixedSize(400, 300);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    // Loan amount
    QDoubleSpinBox* amountSpinBox = new QDoubleSpinBox(&dialog);
    amountSpinBox->setRange(100'000, 10'000'000);
    amountSpinBox->setSingleStep(50'000);
    amountSpinBox->setValue(1'000'000);
    amountSpinBox->setSuffix(" руб.");
    layout->addWidget(new QLabel("Сумма кредита:", &dialog));
    layout->addWidget(amountSpinBox);

    // Loan term
    QSpinBox* termSpinBox = new QSpinBox(&dialog);
    termSpinBox->setRange(12, 84);
    termSpinBox->setSingleStep(12);
    termSpinBox->setValue(36);
    termSpinBox->setSuffix(" месяцев");
    layout->addWidget(new QLabel("Срок кредита:", &dialog));
    layout->addWidget(termSpinBox);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton("OK", &dialog);
    QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(okButton);
    layout->addLayout(buttonLayout);

    bool accepted = false;
    connect(okButton, &QPushButton::clicked, [&]() {
        accepted = true;
        dialog.accept();
    });
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted && accepted)
    {
        double loanAmount = amountSpinBox->value();
        int loanTerm = termSpinBox->value();

        QSqlQuery query;
        QString queryStr = QString(
            "INSERT INTO loan_requests (client_id, loan_amount, loan_term, status) "
            "VALUES (%1, %2, %3, 'не обработано');")
            .arg(m_user->GetId())
            .arg(loanAmount)
            .arg(loanTerm);

        if (query.exec(queryStr))
        {
            QMessageBox::information(this, "Успех",
                QString("Заявка на кредит на сумму %1 руб. на %2 месяцев создана.")
                    .arg(FormatPrice(loanAmount))
                    .arg(loanTerm));
        }
        else
        {
            QMessageBox::critical(this, "Ошибка", "Не удалось создать заявку: " + query.lastError().text());
        }
    }
}

void MainWindow::InsuranceRequestHandler()
{
    if (!m_user)
    {
        QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для доступа к страхованию.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Заявка на страхование");
    dialog.setFixedSize(400, 300);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    // Insurance type
    QComboBox* insuranceTypeCombo = new QComboBox(&dialog);
    insuranceTypeCombo->addItems({"ОСАГО", "КАСКО", "ДСАГО"});
    layout->addWidget(new QLabel("Тип страхования:", &dialog));
    layout->addWidget(insuranceTypeCombo);

    // Car selection for the user's purchased cars
    QComboBox* carCombo = new QComboBox(&dialog);
    QSqlQuery carQuery;
    QString carQueryStr = QString(
        "SELECT DISTINCT c.name "
        "FROM cars c "
        "INNER JOIN purchases p ON c.id = p.car_id "
        "WHERE p.client_id = %1")
        .arg(m_user->GetId());

    if (carQuery.exec(carQueryStr))
    {
        while (carQuery.next())
        {
            carCombo->addItem(carQuery.value("name").toString());
        }
    }
    layout->addWidget(new QLabel("Автомобиль:", &dialog));
    layout->addWidget(carCombo);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton("OK", &dialog);
    QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(okButton);
    layout->addLayout(buttonLayout);

    bool accepted = false;
    connect(okButton, &QPushButton::clicked, [&]() {
        if (carCombo->currentText().isEmpty())
        {
            QMessageBox::warning(&dialog, "Ошибка",
                "У вас нет автомобилей для страхования. Сначала приобретите автомобиль.");
            return;
        }
        accepted = true;
        dialog.accept();
    });
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted && accepted)
    {
        QString insuranceType = insuranceTypeCombo->currentText();
        QString carName = carCombo->currentText();

        QSqlQuery query;
        QString queryStr = QString(
            "INSERT INTO insurance_requests (client_id, insurance_type, car_name, status) "
            "VALUES (%1, '%2', '%3', 'не обработано');")
            .arg(m_user->GetId())
            .arg(insuranceType)
            .arg(carName);

        if (query.exec(queryStr))
        {
            QMessageBox::information(this, "Успех",
                QString("Заявка на страхование %1 для автомобиля %2 создана.")
                    .arg(insuranceType)
                    .arg(carName));
        }
        else
        {
            QMessageBox::critical(this, "Ошибка",
                "Не удалось создать заявку: " + query.lastError().text());
        }
    }
}

void MainWindow::SelectionProcessing(const bool ok, const QStringView selected_type, const QStringView selected_color)
{
    if (ok && !selected_type.isEmpty())
    {
        if (selected_type == "Смотреть всё")
        {
            m_product_cards->UpdateProductsWidget(ui->scrollArea_catalog, QString("Смотреть всё"), selected_color);
            ui->stackedWidget->setCurrentWidget(ui->main);
        }
        else
        {
            m_product_cards->UpdateProductsWidget(ui->scrollArea_catalog, selected_type.toString(), QString("Белый"));
            ui->stackedWidget->setCurrentWidget(ui->main);
        }
    }
}

void MainWindow::ShowProductOnPersonalPage(const ProductInfo& product, QList<ProductInfo>& m_current_productcolors_) {
    ui->label_name->setText(product.name_);
    ui->label_price->setText(FormatPrice(product.price_) + " руб.");
    m_current_product = product;
    // ui->label_color_index->setText(QString::number(m_current_color_index + 1) + "/" + QString::number(m_current_productcolors_.size()));

    QString imagePath = QDir::cleanPath(product.image_path_);
    QPixmap originalPixmap(imagePath);

    if (!originalPixmap.isNull()) {
        int fixedWidth = 394;
        QPixmap scaledPixmap = originalPixmap.scaled(fixedWidth, originalPixmap.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        int imageHeight = scaledPixmap.height();

        ui->label_car_image->setPixmap(scaledPixmap);
        ui->label_car_image->setFixedSize(fixedWidth, imageHeight);
        ui->label_car_image->show();
    }

    ui->pushButton_order->setVisible(product.stock_qty_ <= 0); // Показываем кнопку "Заказать" только если нет в наличии
    ui->pushButton_to_pay->setVisible(product.stock_qty_ > 0); // Показываем кнопку "К заявке" только если есть в наличии

    ui->stackedWidget->setCurrentWidget(ui->personal);
}

void MainWindow::on_pushButton_next_left_clicked()
{
    // Доступные варианты цветов для current_product
    auto m_current_productcolors_ = m_products->GetAllProductsWithName(m_current_product);
    if (!m_current_productcolors_.isEmpty())
    {
        m_current_color_index = (m_current_color_index - 1 + m_current_productcolors_.size()) % m_current_productcolors_.size();
        m_current_product = m_current_productcolors_.at(m_current_color_index);
        ShowProductOnPersonalPage(m_current_productcolors_.at(m_current_color_index), m_current_productcolors_);
    }
}

void MainWindow::on_pushButton_next_right_clicked()
{
    // Доступные варианты цветов для current_product
    auto m_current_productcolors_ = m_products->GetAllProductsWithName(m_current_product);
    if (!m_current_productcolors_.isEmpty())
    {
        m_current_color_index = (m_current_color_index + 1) % m_current_productcolors_.size();
        m_current_product = m_current_productcolors_.at(m_current_color_index);
        ShowProductOnPersonalPage(m_current_productcolors_.at(m_current_color_index), m_current_productcolors_);
    }
}

void MainWindow::on_pushButton_back_clicked()
{
    if (!m_user || m_user->GetRole() != Role::User) return;

    // Clean up scroll areas
    if (ui->scrollArea_catalog && ui->scrollArea_catalog->widget())
    {
        QWidget* oldCatalogWidget = ui->scrollArea_catalog->takeWidget();
        if (oldCatalogWidget)
        {
            oldCatalogWidget->hide();
            oldCatalogWidget->deleteLater();
        }
    }

    if (ui->scrollArea_purchased_cars && ui->scrollArea_purchased_cars->widget())
    {
        QWidget* oldPurchasedWidget = ui->scrollArea_purchased_cars->takeWidget();
        if (oldPurchasedWidget)
        {
            oldPurchasedWidget->hide();
            oldPurchasedWidget->deleteLater();
        }
    }

    // Reset current product state
    m_current_product = ProductInfo();
    m_current_color_index = 0;

    // Rebuild the main catalog view
    if (m_product_cards)
    {
        m_product_cards->UpdateProductsWidget(ui->scrollArea_catalog, QString("Смотреть всё"), QString("Белый"));
    }

    ui->stackedWidget->setCurrentWidget(ui->main);
}

void MainWindow::on_pushButton_info_clicked()
{
    if (m_current_product.name_.isEmpty())
    {
        QMessageBox::warning(this, "Ошибка", "Не выбран автомобиль для просмотра информации.");
        return;
    }

    // Формируем детальную информацию об автомобиле
    QString info = QString("Название: %1\nЦена: %2 руб.\nЦвет: %3\nОписание: %4")
                       .arg(m_current_product.name_)
                       .arg(FormatPrice(m_current_product.price_))
                       .arg(m_current_product.color_)
                       .arg(m_current_product.description_.isEmpty() ? "Описание отсутствует." : m_current_product.description_);

    QMessageBox::information(this, "Информация об автомобиле", info);
}

void MainWindow::on_pushButton_test_drive_clicked()
{
    if (!m_user)
    {
        QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для записи на тест-драйв.");
        return;
    }

    if (m_current_product.name_.isEmpty())
    {
        QMessageBox::warning(this, "Ошибка", "Не выбран автомобиль для записи на тест-драйв.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Запись на тест-драйв");
    dialog.setFixedSize(500, 500);
    dialog.setStyleSheet(
        "QDialog {"
        "    background-color: #ffffff;"
        "}"
        "QLabel {"
        "    color: #1d1b20;"
        "    font: 500 12pt 'JetBrains Mono';"
        "    margin-top: 10px;"
        "}"
        "QTimeEdit, QCalendarWidget {"
        "    padding: 8px;"
        "    border: 2px solid #e0e0e0;"
        "    border-radius: 8px;"
        "    background: #fafafa;"
        "    font: 11pt 'JetBrains Mono';"
        "    min-height: 30px;"
        "}"
        "QCalendarWidget QToolButton {"
        "    color: #1d1b20;"
        "    font: 10pt 'JetBrains Mono';"
        "    padding: 5px;"
        "}"
        "QCalendarWidget QMenu {"
        "    background-color: white;"
        "    font: 10pt 'JetBrains Mono';"
        "}"
        "QPushButton {"
        "    padding: 10px 20px;"
        "    border-radius: 8px;"
        "    font: 600 11pt 'JetBrains Mono';"
        "    min-width: 100px;"
        "}"
        "QPushButton[type='primary'] {"
        "    background-color: #2196F3;"
        "    color: white;"
        "    border: none;"
        "}"
        "QPushButton[type='primary']:hover {"
        "    background-color: #1976D2;"
        "}"
        "QPushButton[type='secondary'] {"
        "    background-color: #fafafa;"
        "    color: #1d1b20;"
        "    border: 2px solid #e0e0e0;"
        "}"
        "QPushButton[type='secondary']:hover {"
        "    background-color: #e0e0e0;"
        "}"
    );

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(15);
    layout->setContentsMargins(30, 30, 30, 30);

    // Заголовок
    QLabel* titleLabel = new QLabel("Запись на тест-драйв", &dialog);
    titleLabel->setStyleSheet("font: 700 16pt 'JetBrains Mono'; margin-bottom: 20px;");
    layout->addWidget(titleLabel);

    // Информация об автомобиле
    QLabel* carInfoLabel = new QLabel(QString("Автомобиль: %1\nЦвет: %2\nЦена: %3 руб.")
        .arg(m_current_product.name_)
        .arg(m_current_product.color_)
        .arg(FormatPrice(m_current_product.price_)), &dialog);
    layout->addWidget(carInfoLabel);

    // Выбор времени
    QLabel* timeLabel = new QLabel("Выберите время:", &dialog);
    layout->addWidget(timeLabel);

    QTimeEdit* timeEdit = new QTimeEdit(&dialog);
    timeEdit->setDisplayFormat("HH:mm");
    timeEdit->setTime(QTime(9, 0)); // По умолчанию 9:00
    timeEdit->setMinimumTime(QTime(9, 0)); // Начало рабочего дня
    timeEdit->setMaximumTime(QTime(18, 0)); // Конец рабочего дня
    layout->addWidget(timeEdit);

    // Выбор даты
    QLabel* dateLabel = new QLabel("Выберите дату:", &dialog);
    layout->addWidget(dateLabel);

    QCalendarWidget* calendar = new QCalendarWidget(&dialog);
    calendar->setMinimumDate(QDate::currentDate()); // Запрет выбора прошедших дат
    calendar->setStyleSheet(
        "QCalendarWidget QWidget {"
        "    background-color: white;"
        "}"
        "QCalendarWidget QToolButton {"
        "    color: #1d1b20;"
        "    background-color: transparent;"
        "    border: none;"
        "}"
        "QCalendarWidget QToolButton:hover {"
        "    background-color: #e0e0e0;"
        "    border-radius: 4px;"
        "}"
        "QCalendarWidget QMenu {"
        "    background-color: white;"
        "    border: 1px solid #e0e0e0;"
        "}"
        "QCalendarWidget QSpinBox {"
        "    background-color: white;"
        "    border: 1px solid #e0e0e0;"
        "    border-radius: 4px;"
        "}"
        "QCalendarWidget QAbstractItemView:enabled {"
        "    background-color: white;"
        "    color: #1d1b20;"
        "}"
        "QCalendarWidget QAbstractItemView:disabled {"
        "    color: #858585;"
        "}"
    );
    layout->addWidget(calendar);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    QPushButton* okButton = new QPushButton("Подтвердить", &dialog);
    okButton->setProperty("type", "primary");

    QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
    cancelButton->setProperty("type", "secondary");

    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(okButton);
    layout->addLayout(buttonLayout);

    bool accepted = false;
    connect(okButton, &QPushButton::clicked, [&]() {
        accepted = true;
        dialog.accept();
    });
    connect(cancelButton, &QPushButton::clicked, [&]() {
        dialog.reject();
    });

    if (dialog.exec() == QDialog::Accepted && accepted)
    {
        // Проверяем наличие автомобиля на складе
        QSqlQuery stockQuery;
        stockQuery.prepare("SELECT stock_qty FROM cars WHERE id = :car_id");
        stockQuery.bindValue(":car_id", m_current_product.id_);
        
        if (!stockQuery.exec() || !stockQuery.next()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось проверить наличие автомобиля");
            return;
        }
        
        int stockQty = stockQuery.value(0).toInt();
        
        if (stockQty <= 0) {
            QMessageBox::warning(this, "Автомобиль недоступен", 
                QString("Автомобиль %1 (%2) временно недоступен для тест-драйва.\n\n"
                       "Возможные варианты:\n"
                       "• Оформить заявку на заказ автомобиля\n"
                       "• Выбрать другой автомобиль")
                .arg(m_current_product.name_)
                .arg(m_current_product.color_));
            return;
        }

        QDate selectedDate = calendar->selectedDate();
        QTime selectedTime = timeEdit->time();
        QString dateTime = selectedDate.toString("yyyy-MM-dd") + " " + selectedTime.toString("HH:mm");

        QSqlQuery query;
        QString queryStr = QString("INSERT INTO test_drives (client_id, car_id, scheduled_date, status) "
                                 "VALUES (%1, %2, '%3', 'не обработано');")
                                 .arg(m_user->GetId())
                                 .arg(m_current_product.id_)
                                 .arg(dateTime);

        if (query.exec(queryStr))
        {
            QMessageBox::information(this, "Успех", QString("Вы записаны на тест-драйв автомобиля %1\nДата: %2\nВремя: %3")
                .arg(m_current_product.name_)
                .arg(selectedDate.toString("dd.MM.yyyy"))
                .arg(selectedTime.toString("HH:mm")));
        }
        else
        {
            QMessageBox::critical(this, "Ошибка", "Не удалось записаться на тест-драйв: " + query.lastError().text());
        }
    }
}

void MainWindow::on_pushButton_to_pay_clicked()
{
    if (!m_user) {
        QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для оформления заявки.");
        return;
    }

    if (m_current_product.name_.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Не выбран автомобиль для оформления заявки.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Оформление заявки на покупку");
    dialog.setMinimumSize(500, 400);
    dialog.resize(500, 500); // Начальный размер, но может увеличиваться
    dialog.setStyleSheet(
        "QDialog {"
        "    background-color: #ffffff;"
        "}"
        "QLabel {"
        "    color: #1d1b20;"
        "    font: 500 12pt 'JetBrains Mono';"
        "    margin-top: 10px;"
        "}"
        "QCheckBox {"
        "    font: 11pt 'JetBrains Mono';"
        "    spacing: 8px;"
        "}"
        "QComboBox {"
        "    padding: 8px;"
        "    border: 2px solid #e0e0e0;"
        "    border-radius: 8px;"
        "    background: #fafafa;"
        "    font: 11pt 'JetBrains Mono';"
        "    min-height: 30px;"
        "}"
        "QPushButton {"
        "    padding: 10px 20px;"
        "    border-radius: 8px;"
        "    font: 600 11pt 'JetBrains Mono';"
        "    min-width: 100px;"
        "}"
        "QPushButton[type='primary'] {"
        "    background-color: #2196F3;"
        "    color: white;"
        "    border: none;"
        "}"
        "QPushButton[type='primary']:hover {"
        "    background-color: #1976D2;"
        "}"
        "QPushButton[type='secondary'] {"
        "    background-color: #fafafa;"
        "    color: #1d1b20;"
        "    border: 2px solid #e0e0e0;"
        "}"
        "QPushButton[type='secondary']:hover {"
        "    background-color: #e0e0e0;"
        "}"
    );

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(15);
    layout->setContentsMargins(30, 30, 30, 30);

    // Информация об автомобиле
    QLabel* carInfoLabel = new QLabel(QString("Автомобиль: %1\nЦвет: %2\nЦена: %3 руб.")
        .arg(m_current_product.name_)
        .arg(m_current_product.color_)
        .arg(FormatPrice(m_current_product.price_)), &dialog);
    layout->addWidget(carInfoLabel);

    // Опции оформления
    QLabel* optionsLabel = new QLabel("Выберите вариант оформления:", &dialog);
    layout->addWidget(optionsLabel);

    // Чекбокс для прямой покупки
    QCheckBox* buyCheckBox = new QCheckBox("Купить", &dialog);
    layout->addWidget(buyCheckBox);

    // Чекбокс для кредита
    QCheckBox* loanCheckBox = new QCheckBox("Оформить в кредит", &dialog);
    layout->addWidget(loanCheckBox);

    // Выпадающий список для срока кредита
    QComboBox* loanTermCombo = new QComboBox(&dialog);
    loanTermCombo->addItems({"12 месяцев", "24 месяца", "36 месяцев", "48 месяцев", "60 месяцев"});
    loanTermCombo->hide();
    layout->addWidget(loanTermCombo);

    // Чекбокс для страховки
    QCheckBox* insuranceCheckBox = new QCheckBox("Добавить страховку", &dialog);
    layout->addWidget(insuranceCheckBox);

    // Выпадающий список для типа страховки
    QComboBox* insuranceTypeCombo = new QComboBox(&dialog);
    insuranceTypeCombo->addItems({"ОСАГО", "КАСКО", "Комплекс"});
    insuranceTypeCombo->hide();
    layout->addWidget(insuranceTypeCombo);

    // Выбор комплектации (трим)
    QLabel* trimLabel = new QLabel("Комплектация (если доступно):", &dialog);
    QComboBox* trimCombo = new QComboBox(&dialog);
    {
        QSqlQuery q;
        q.prepare("SELECT DISTINCT trim FROM cars WHERE name = :name AND trim IS NOT NULL AND trim <> ''");
        q.bindValue(":name", m_current_product.name_);
        if (q.exec()) {
            while (q.next()) {
                trimCombo->addItem(q.value(0).toString());
            }
        }
        if (!m_current_product.trim_.isEmpty()) {
            int idx = trimCombo->findText(m_current_product.trim_);
            if (idx >= 0) trimCombo->setCurrentIndex(idx);
        }
    }
    if (trimCombo->count() > 0) {
        layout->addWidget(trimLabel);
        layout->addWidget(trimCombo);
    } else {
        trimLabel->hide();
        trimCombo->hide();
    }

    // Чекбокс для аренды
    QCheckBox* rentalCheckBox = new QCheckBox("Взять в аренду", &dialog);
    layout->addWidget(rentalCheckBox);

    // Выпадающий список для срока аренды
    QComboBox* rentalTermCombo = new QComboBox(&dialog);
    rentalTermCombo->addItems({"1 месяц", "3 месяца", "6 месяцев", "12 месяцев"});
    rentalTermCombo->hide();
    layout->addWidget(rentalTermCombo);

    // Соединяем чекбоксы с их комбобоксами
    connect(loanCheckBox, &QCheckBox::toggled, loanTermCombo, &QComboBox::setVisible);
    connect(insuranceCheckBox, &QCheckBox::toggled, insuranceTypeCombo, &QComboBox::setVisible);
    connect(rentalCheckBox, &QCheckBox::toggled, rentalTermCombo, &QComboBox::setVisible);

    // Взаимоисключающая логика для покупки, кредита и аренды
    connect(buyCheckBox, &QCheckBox::toggled, [loanCheckBox, rentalCheckBox](bool checked) {
        if (checked) {
            loanCheckBox->setChecked(false);
            rentalCheckBox->setChecked(false);
        }
    });
    connect(loanCheckBox, &QCheckBox::toggled, [buyCheckBox, rentalCheckBox](bool checked) {
        if (checked) {
            buyCheckBox->setChecked(false);
            rentalCheckBox->setChecked(false);
        }
    });
    connect(rentalCheckBox, &QCheckBox::toggled, [buyCheckBox, loanCheckBox](bool checked) {
        if (checked) {
            buyCheckBox->setChecked(false);
            loanCheckBox->setChecked(false);
        }
    });

    // Кнопки
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    QPushButton* okButton = new QPushButton("Подтвердить", &dialog);
    okButton->setProperty("type", "primary");

    QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
    cancelButton->setProperty("type", "secondary");

    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(okButton);
    layout->addLayout(buttonLayout);

    bool accepted = false;
    connect(okButton, &QPushButton::clicked, [&]() {
        if (!buyCheckBox->isChecked() && !loanCheckBox->isChecked() && !insuranceCheckBox->isChecked() && !rentalCheckBox->isChecked()) {
            QMessageBox::warning(&dialog, "Ошибка", "Выберите вариант оформления.");
            return;
        }
        accepted = true;
        dialog.accept();
    });
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted && accepted)
    {
        QSqlQuery query;
        QString selectedTrim = trimCombo->isVisible() ? trimCombo->currentText() : QString();
        int targetCarId = m_current_product.id_;
        int targetStock = m_current_product.stock_qty_;
        if (trimCombo->isVisible() && !selectedTrim.isEmpty() && selectedTrim != m_current_product.trim_) {
            QSqlQuery find;
            find.prepare("SELECT id, stock_qty FROM cars WHERE name = :name AND trim = :trim AND color = :color ORDER BY stock_qty DESC LIMIT 1");
            find.bindValue(":name", m_current_product.name_);
            find.bindValue(":trim", selectedTrim);
            find.bindValue(":color", m_current_product.color_);
            if (find.exec() && find.next()) {
                targetCarId = find.value(0).toInt();
                targetStock = find.value(1).toInt();
            } else {
                // Попробуем без учета цвета
                find.finish();
                find.prepare("SELECT id, stock_qty FROM cars WHERE name = :name AND trim = :trim ORDER BY stock_qty DESC LIMIT 1");
                find.bindValue(":name", m_current_product.name_);
                find.bindValue(":trim", selectedTrim);
                if (find.exec() && find.next()) {
                    targetCarId = find.value(0).toInt();
                    targetStock = find.value(1).toInt();
                } else {
                    // Если автомобиль с выбранным trim не найден, то его нет в наличии
                    targetStock = 0;
                }
            }
        }

        // Прямая покупка
        if (buyCheckBox->isChecked())
        {
            if (targetStock > 0) {
                // Создаем заявку на покупку вместо прямой покупки
                QString queryStr = QString("INSERT INTO purchase_requests (client_id, car_id, status) VALUES (%1, %2, 'не обработано');")
                    .arg(m_user->GetId())
                    .arg(targetCarId);
                
                QSqlQuery purchaseQuery;
                if (purchaseQuery.exec(queryStr)) {
                    QMessageBox::information(this, "Заявка создана", QString("Заявка на покупку автомобиля %1 отправлена на рассмотрение!")
                        .arg(m_current_product.name_));
                } else {
                    QMessageBox::critical(this, "Ошибка", "Не удалось создать заявку на покупку: " + purchaseQuery.lastError().text());
                    return;
                }
            } else {
                // Нет в наличии — оформляем заявку на заказ
                QSqlQuery ins;
                ins.prepare("INSERT INTO order_requests (client_id, car_name, color, trim, status) VALUES (:client_id, :car_name, :color, :trim, 'не обработано')");
                ins.bindValue(":client_id", m_user->GetId());
                ins.bindValue(":car_name", m_current_product.name_);
                ins.bindValue(":color", m_current_product.color_);
                ins.bindValue(":trim", selectedTrim.isEmpty() ? m_current_product.trim_ : selectedTrim);
                if (ins.exec()) {
                    QMessageBox::information(this, "Заявка создана", "Автомобиль отсутствует на складе. Заявка на заказ отправлена менеджеру.");
                } else {
                    QMessageBox::critical(this, "Ошибка", "Не удалось создать заявку на заказ: " + ins.lastError().text());
                    return;
                }
            }
        }

        // Оформление кредита
        if (loanCheckBox->isChecked())
        {
            // Проверяем наличие автомобиля на складе
            qDebug() << "Кредит: targetStock =" << targetStock << "для автомобиля" << m_current_product.name_;
            if (targetStock <= 0) {
                QMessageBox::warning(this, "Автомобиль недоступен", 
                    QString("Автомобиль %1 (%2) временно недоступен для оформления кредита.\n\n"
                           "Возможные варианты:\n"
                           "• Оформить заявку на заказ автомобиля\n"
                           "• Выбрать другой автомобиль")
                    .arg(m_current_product.name_)
                    .arg(m_current_product.color_));
                return;
            }
            
            int months = loanTermCombo->currentText().split(" ")[0].toInt();
            QString queryStr = QString(
                "INSERT INTO loan_requests (client_id, car_id, loan_amount, loan_term_months, status) "
                "VALUES (%1, %2, %3, %4, 'не обработано');")
                .arg(m_user->GetId())
                .arg(targetCarId)
                .arg(m_current_product.price_)
                .arg(months);

            if (!query.exec(queryStr))
            {
                QMessageBox::critical(this, "Ошибка", "Не удалось оформить заявку на кредит: " + query.lastError().text());
                return;
            }
        }

        // Оформление страховки
        if (insuranceCheckBox->isChecked())
        {
            // Страхование можно оформить даже если автомобиля нет в наличии
            // (клиент может застраховать свою машину через автосалон)
            QString queryStr = QString(
                "INSERT INTO insurance_requests (client_id, car_id, insurance_type, status) "
                "VALUES (%1, %2, '%3', 'не обработано');")
                .arg(m_user->GetId())
                .arg(targetCarId)
                .arg(insuranceTypeCombo->currentText());

            if (!query.exec(queryStr))
            {
                QMessageBox::critical(this, "Ошибка", "Не удалось оформить заявку на страхование: " + query.lastError().text());
                return;
            }
        }

        // Оформление аренды
        if (rentalCheckBox->isChecked())
        {
            // Проверяем наличие автомобиля на складе
            if (targetStock <= 0) {
                QMessageBox::warning(this, "Автомобиль недоступен", 
                    QString("Автомобиль %1 (%2) временно недоступен для аренды.\n\n"
                           "Возможные варианты:\n"
                           "• Оформить заявку на заказ автомобиля\n"
                           "• Выбрать другой автомобиль")
                    .arg(m_current_product.name_)
                    .arg(m_current_product.color_));
                return;
            }
            
            QString queryStr = QString(
                "INSERT INTO rental_requests (client_id, car_id, rental_days, start_date, status) "
                "VALUES (%1, %2, %3, '%4', 'не обработано');")
                .arg(m_user->GetId())
                .arg(targetCarId)
                .arg(rentalTermCombo->currentText().split(" ")[0].toInt())
                .arg(QDate::currentDate().toString("yyyy-MM-dd"));

            QString errorMessage;
            if (!m_database_handler->ExecuteQueryWithUserMessage(queryStr, errorMessage))
            {
                QMessageBox::warning(this, "❌ Ошибка", errorMessage);
                return;
            }
        }

        QMessageBox::information(this, "Успех", "Заявка успешно оформлена!");
    }
}

void MainWindow::on_pushButton_order_clicked()
{
    if (!m_user) {
        QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для оформления заказа.");
        return;
    }

    if (m_current_product.name_.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Не выбран автомобиль для заказа.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Заказ автомобиля");
    dialog.setFixedSize(500, 400);
    dialog.setStyleSheet(
        "QDialog {"
        "    background-color: #ffffff;"
        "}"
        "QLabel {"
        "    color: #1d1b20;"
        "    font: 500 12pt 'JetBrains Mono';"
        "    margin-top: 10px;"
        "}"
        "QComboBox {"
        "    padding: 8px;"
        "    border: 2px solid #e0e0e0;"
        "    border-radius: 8px;"
        "    background: #fafafa;"
        "    font: 11pt 'JetBrains Mono';"
        "    min-height: 30px;"
        "}"
        "QPushButton {"
        "    padding: 10px 20px;"
        "    border-radius: 8px;"
        "    font: 600 11pt 'JetBrains Mono';"
        "    min-width: 100px;"
        "}"
        "QPushButton[type='primary'] {"
        "    background-color: #FF6B35;"
        "    color: white;"
        "    border: none;"
        "}"
        "QPushButton[type='primary']:hover {"
        "    background-color: #E55A2B;"
        "}"
        "QPushButton[type='secondary'] {"
        "    background-color: #fafafa;"
        "    color: #1d1b20;"
        "    border: 2px solid #e0e0e0;"
        "}"
        "QPushButton[type='secondary']:hover {"
        "    background-color: #e0e0e0;"
        "}"
    );

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(15);
    layout->setContentsMargins(30, 30, 30, 30);

    // Информация об автомобиле
    QLabel* carInfoLabel = new QLabel(QString("Автомобиль: %1\nЦвет: %2\nЦена: %3 руб.")
        .arg(m_current_product.name_)
        .arg(m_current_product.color_)
        .arg(FormatPrice(m_current_product.price_)), &dialog);
    layout->addWidget(carInfoLabel);

    // Выбор комплектации
    QLabel* trimLabel = new QLabel("Выберите комплектацию:", &dialog);
    layout->addWidget(trimLabel);

    QComboBox* trimCombo = new QComboBox(&dialog);
    
    // Загружаем доступные комплектации
    QSqlQuery query;
    query.prepare("SELECT DISTINCT trim FROM cars WHERE name = :name AND trim IS NOT NULL AND trim <> ''");
    query.bindValue(":name", m_current_product.name_);
    if (query.exec()) {
        while (query.next()) {
            trimCombo->addItem(query.value(0).toString());
        }
    }
    
    // Если нет комплектаций, добавляем текущую или "Стандартная"
    if (trimCombo->count() == 0) {
        if (!m_current_product.trim_.isEmpty()) {
            trimCombo->addItem(m_current_product.trim_);
        } else {
            trimCombo->addItem("Стандартная");
        }
    } else {
        // Выбираем текущую комплектацию если есть
        if (!m_current_product.trim_.isEmpty()) {
            int idx = trimCombo->findText(m_current_product.trim_);
            if (idx >= 0) trimCombo->setCurrentIndex(idx);
        }
    }
    
    layout->addWidget(trimCombo);

    // Показываем информацию о наличии
    QLabel* stockLabel = new QLabel("", &dialog);
    layout->addWidget(stockLabel);
    
    // Функция для обновления информации о наличии
    auto updateStockInfo = [=](const QString& trim) {
        QSqlQuery stockQuery;
        stockQuery.prepare("SELECT stock_qty FROM cars WHERE name = :name AND trim = :trim AND color = :color");
        stockQuery.bindValue(":name", m_current_product.name_);
        stockQuery.bindValue(":trim", trim);
        stockQuery.bindValue(":color", m_current_product.color_);
        
        if (stockQuery.exec() && stockQuery.next()) {
            int stock = stockQuery.value(0).toInt();
            if (stock > 0) {
                stockLabel->setText(QString("В наличии: %1 шт.").arg(stock));
                stockLabel->setStyleSheet("color: #4CAF50;");
            } else {
                stockLabel->setText("Нет в наличии - будет создана заявка на заказ");
                stockLabel->setStyleSheet("color: #FF6B35;");
            }
        } else {
            stockLabel->setText("Нет в наличии - будет создана заявка на заказ");
            stockLabel->setStyleSheet("color: #FF6B35;");
        }
    };
    
    // Обновляем при изменении комплектации
    connect(trimCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [=]() {
        updateStockInfo(trimCombo->currentText());
    });
    
    // Инициализируем информацию о наличии
    updateStockInfo(trimCombo->currentText());

    // Кнопки
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    QPushButton* okButton = new QPushButton("Заказать", &dialog);
    okButton->setProperty("type", "primary");

    QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
    cancelButton->setProperty("type", "secondary");

    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(okButton);
    layout->addLayout(buttonLayout);

    bool accepted = false;
    connect(okButton, &QPushButton::clicked, [&]() {
        accepted = true;
        dialog.accept();
    });
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted && accepted)
    {
        QString selectedTrim = trimCombo->currentText();
        
        // Проверяем наличие выбранной комплектации
        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT id, stock_qty FROM cars WHERE name = :name AND trim = :trim AND color = :color");
        checkQuery.bindValue(":name", m_current_product.name_);
        checkQuery.bindValue(":trim", selectedTrim);
        checkQuery.bindValue(":color", m_current_product.color_);
        
        int targetCarId = m_current_product.id_;
        int targetStock = 0;
        
        if (checkQuery.exec() && checkQuery.next()) {
            targetCarId = checkQuery.value(0).toInt();
            targetStock = checkQuery.value(1).toInt();
        }
        
        if (targetStock > 0) {
            // Есть в наличии - создаем заявку на покупку
            QSqlQuery purchaseQuery;
            purchaseQuery.prepare("INSERT INTO purchase_requests (client_id, car_id, status) VALUES (:client_id, :car_id, 'не обработано')");
            purchaseQuery.bindValue(":client_id", m_user->GetId());
            purchaseQuery.bindValue(":car_id", targetCarId);
            
            if (purchaseQuery.exec()) {
                QMessageBox::information(this, "Заявка создана", QString("Заявка на покупку автомобиля %1 (%2) отправлена на рассмотрение!")
                    .arg(m_current_product.name_)
                    .arg(selectedTrim));
            } else {
                QMessageBox::critical(this, "Ошибка", "Не удалось создать заявку на покупку: " + purchaseQuery.lastError().text());
            }
        } else {
            // Нет в наличии - создаем заявку на заказ
            QSqlQuery orderQuery;
            orderQuery.prepare("INSERT INTO order_requests (client_id, car_name, color, trim, status) VALUES (:client_id, :car_name, :color, :trim, 'не обработано')");
            orderQuery.bindValue(":client_id", m_user->GetId());
            orderQuery.bindValue(":car_name", m_current_product.name_);
            orderQuery.bindValue(":color", m_current_product.color_);
            orderQuery.bindValue(":trim", selectedTrim);
            
            if (orderQuery.exec()) {
                QMessageBox::information(this, "Заявка создана", 
                    QString("Автомобиль %1 (%2, %3) отсутствует на складе.\nЗаявка на заказ отправлена менеджеру.")
                    .arg(m_current_product.name_)
                    .arg(m_current_product.color_)
                    .arg(selectedTrim));
            } else {
                QMessageBox::critical(this, "Ошибка", "Не удалось создать заявку на заказ: " + orderQuery.lastError().text());
            }
        }
    }
}

void MainWindow::on_pushButton_notifications_clicked()
{
    m_notifications_handler->loadAndShowNotifications(m_user->GetId());
    m_notifications_handler->exec();
}

void MainWindow::on_pushButton_settings_clicked()
{
    if (!m_user) {
        QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для доступа к настройкам.");
        return;
    }

    // Проверяем, существует ли уже открытый диалог
    if (m_settings_dialog) {
        m_settings_dialog->activateWindow();
        return;
    }

    // Создаем диалог
    m_settings_dialog.reset(new QDialog(this));
    m_settings_dialog->setWindowTitle("Настройки профиля");
    m_settings_dialog->setFixedSize(450, 600);
    m_settings_dialog->setStyleSheet(
        "QDialog {"
        "    background-color: #ffffff;"
        "}"
        "QLabel {"
        "    color: #1d1b20;"
        "    font: 500 12pt 'JetBrains Mono';"
        "    min-height: 20px;"
        "    margin: 3px 0px;"
        "}"
        "QLabel[type='header'] {"
        "    font: 700 16pt 'JetBrains Mono';"
        "    min-height: 30px;"
        "    margin: 0px 0px 15px 0px;"
        "}"
        "QLineEdit {"
        "    padding: 5px 8px;"
        "    border: 2px solid #e0e0e0;"
        "    border-radius: 8px;"
        "    background: #fafafa;"
        "    font: 11pt 'JetBrains Mono';"
        "    min-height: 16px;"
        "    margin-bottom: 10px;"
        "}"
        "QLineEdit:focus {"
        "    border: 2px solid #2196F3;"
        "}"
        "QPushButton {"
        "    padding: 8px 16px;"
        "    border-radius: 8px;"
        "    font: 600 11pt 'JetBrains Mono';"
        "    min-width: 90px;"
        "    min-height: 32px;"
        "}"
        "QPushButton[type='primary'] {"
        "    background-color: #2196F3;"
        "    color: white;"
        "    border: none;"
        "}"
        "QPushButton[type='primary']:hover {"
        "    background-color: #1976D2;"
        "}"
        "QPushButton[type='secondary'] {"
        "    background-color: #fafafa;"
        "    color: #1d1b20;"
        "    border: 2px solid #e0e0e0;"
        "}"
        "QPushButton[type='secondary']:hover {"
        "    background-color: #e0e0e0;"
        "}"
    );

    QVBoxLayout* dialogLayout = new QVBoxLayout(m_settings_dialog.get());
    dialogLayout->setSpacing(10);
    dialogLayout->setContentsMargins(20, 20, 20, 20);

    // Заголовок
    QLabel* titleLabel = new QLabel("Редактирование профиля", m_settings_dialog.get());
    titleLabel->setProperty("type", "header");
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    dialogLayout->addWidget(titleLabel);

    // Создаем поля ввода
    m_settings_first_name.reset(new QLineEdit(m_settings_dialog.get()));
    settings_last_name_.reset(new QLineEdit(m_settings_dialog.get()));
    settings_email_.reset(new QLineEdit(m_settings_dialog.get()));
    settings_phone_.reset(new QLineEdit(m_settings_dialog.get()));
    settings_password_.reset(new QLineEdit(m_settings_dialog.get()));
    settings_password_->setEchoMode(QLineEdit::Password);

    // Получаем информацию о пользователе из базы данных
    QSqlQuery query;
    QString queryStr = QString(
        "SELECT first_name, last_name, email, phone, password "
        "FROM clients WHERE id = %1")
        .arg(m_user->GetId());

    if (query.exec(queryStr) && query.next()) {
        // Имя
        QLabel* firstNameLabel = new QLabel("Имя:", m_settings_dialog.get());
        dialogLayout->addWidget(firstNameLabel);
        m_settings_first_name->setText(query.value("first_name").toString());
        dialogLayout->addWidget(m_settings_first_name.get());

        // Фамилия
        QLabel* lastNameLabel = new QLabel("Фамилия:", m_settings_dialog.get());
        dialogLayout->addWidget(lastNameLabel);
        settings_last_name_->setText(query.value("last_name").toString());
        dialogLayout->addWidget(settings_last_name_.get());

        // Email
        QLabel* emailLabel = new QLabel("Email:", m_settings_dialog.get());
        dialogLayout->addWidget(emailLabel);
        settings_email_->setText(query.value("email").toString());
        dialogLayout->addWidget(settings_email_.get());

        // Телефон
        QLabel* phoneLabel = new QLabel("Телефон:", m_settings_dialog.get());
        dialogLayout->addWidget(phoneLabel);
        settings_phone_->setText(query.value("phone").toString());
        dialogLayout->addWidget(settings_phone_.get());

        // Пароль
        QLabel* passwordLabel = new QLabel("Пароль:", m_settings_dialog.get());
        dialogLayout->addWidget(passwordLabel);
        settings_password_->setText(""); // Не показываем старый пароль
        settings_password_->setPlaceholderText("Введите новый пароль или оставьте пустым");
        settings_password_->setEchoMode(QLineEdit::Password);
        dialogLayout->addWidget(settings_password_.get());
    }

    // Растягивающийся элемент
    dialogLayout->addStretch();

    // Кнопки
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    QPushButton* saveButton = new QPushButton("Сохранить", m_settings_dialog.get());
    saveButton->setProperty("type", "primary");
    QPushButton* cancelButton = new QPushButton("Отмена", m_settings_dialog.get());
    cancelButton->setProperty("type", "secondary");

    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(saveButton);
    dialogLayout->addLayout(buttonLayout);

    // Сохраняем указатель на диалог для использования в лямбда-функциях
    QDialog* dialogPtr = m_settings_dialog.get();

    // Обработка нажатия кнопки "Сохранить"
    connect(saveButton, &QPushButton::clicked, dialogPtr, [this, dialogPtr]() {
        // Проверяем, что обязательные поля заполнены
        if (m_settings_first_name->text().isEmpty() || settings_last_name_->text().isEmpty() ||
            settings_email_->text().isEmpty() || settings_phone_->text().isEmpty()) {
            QMessageBox::warning(dialogPtr, "Ошибка", "Все поля кроме пароля должны быть заполнены.");
            return;
        }

        // Сохраняем значения полей во временные переменные
        QString firstName = m_settings_first_name->text();
        QString lastName = settings_last_name_->text();
        QString email = settings_email_->text();
        QString phone = settings_phone_->text();
        QString password = settings_password_->text();

        // Формируем базовый запрос без пароля
        QString updateQuery = QString(
            "UPDATE clients SET "
            "first_name = '%1', "
            "last_name = '%2', "
            "email = '%3', "
            "phone = '%4'")
            .arg(firstName)
            .arg(lastName)
            .arg(email)
            .arg(phone);

        // Если введен новый пароль, добавляем его в запрос
        if (!password.isEmpty()) {
            // Хешируем новый пароль
            QString hashedPassword = QString(QCryptographicHash::hash(
                password.toUtf8(),
                QCryptographicHash::Sha256).toHex());
            
            updateQuery += QString(", password = '%1'").arg(hashedPassword);
        }

        // Добавляем условие WHERE
        updateQuery += QString(" WHERE id = %1").arg(m_user->GetId());

        if (m_database_handler->ExecuteQuery(updateQuery)) {
            // Обновляем данные пользователя в памяти
            if (m_user) {
                m_user->SetName(firstName + " " + lastName);
                m_user->SetEmail(email);
                // Обновляем отображаемое имя в интерфейсе
                if (ui->label_clientname) {
                    ui->label_clientname->setText(m_user->GetName() + " — профиль");
                }
            }

            QMessageBox::information(dialogPtr, "Успех", "Данные профиля обновлены.");
            dialogPtr->accept();
        } else {
            QMessageBox::critical(dialogPtr, "Ошибка",
                "Не удалось обновить данные профиля: " + m_database_handler->GetLastError());
        }
    });

    connect(cancelButton, &QPushButton::clicked, dialogPtr, &QDialog::reject);

    // Очистка указателей при закрытии диалога
    connect(dialogPtr, &QDialog::finished, this, [this]() {
        m_settings_first_name.reset();
        settings_last_name_.reset();
        settings_email_.reset();
        settings_phone_.reset();
        settings_password_.reset();
        m_settings_dialog.reset();
    });

    m_settings_dialog->show();
}

void MainWindow::on_pushButton_registration_clicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Регистрация");
    dialog.setFixedSize(500, 800);
    dialog.setStyleSheet(
        "QDialog {"
        "    background-color: #ffffff;"
        "}"
        "QLabel {"
        "    color: #1d1b20;"
        "    font: 500 12pt 'JetBrains Mono';"
        "    margin-top: 10px;"
        "}"
        "QLineEdit {"
        "    padding: 8px;"
        "    border: 2px solid #e0e0e0;"
        "    border-radius: 8px;"
        "    background: #fafafa;"
        "    font: 11pt 'JetBrains Mono';"
        "    min-height: 30px;"
        "    margin-bottom: 10px;"
        "}"
        "QPushButton {"
        "    padding: 10px 20px;"
        "    border-radius: 8px;"
        "    font: 600 11pt 'JetBrains Mono';"
        "    min-width: 100px;"
        "}"
        "QPushButton[type='primary'] {"
        "    background-color: #2196F3;"
        "    color: white;"
        "    border: none;"
        "}"
        "QPushButton[type='primary']:hover {"
        "    background-color: #1976D2;"
        "}"
        "QPushButton[type='secondary'] {"
        "    background-color: #fafafa;"
        "    color: #1d1b20;"
        "    border: 2px solid #e0e0e0;"
        "}"
        "QPushButton[type='secondary']:hover {"
        "    background-color: #e0e0e0;"
        "}"
    );

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(10);
    layout->setContentsMargins(30, 30, 30, 30);

    // Заголовок
    QLabel* titleLabel = new QLabel("Создание учетной записи", &dialog);
    titleLabel->setStyleSheet("font: 700 16pt 'JetBrains Mono'; margin-bottom: 20px;");
    layout->addWidget(titleLabel);

    // Создаем поля ввода с метками
    QLabel* firstNameLabel = new QLabel("Имя:", &dialog);
    layout->addWidget(firstNameLabel);
    QLineEdit* firstNameEdit = new QLineEdit(&dialog);
    firstNameEdit->setPlaceholderText("Введите имя");
    layout->addWidget(firstNameEdit);

    QLabel* lastNameLabel = new QLabel("Фамилия:", &dialog);
    layout->addWidget(lastNameLabel);
    QLineEdit* lastNameEdit = new QLineEdit(&dialog);
    lastNameEdit->setPlaceholderText("Введите фамилию");
    layout->addWidget(lastNameEdit);

    QLabel* phoneLabel = new QLabel("Телефон:", &dialog);
    layout->addWidget(phoneLabel);
    QLineEdit* phoneEdit = new QLineEdit(&dialog);
    phoneEdit->setPlaceholderText("+7XXXXXXXXXX");
    layout->addWidget(phoneEdit);

    QLabel* emailLabel = new QLabel("Email:", &dialog);
    layout->addWidget(emailLabel);
    QLineEdit* emailEdit = new QLineEdit(&dialog);
    emailEdit->setPlaceholderText("example@domain.com");
    layout->addWidget(emailEdit);

    QLabel* passwordLabel = new QLabel("Пароль:", &dialog);
    layout->addWidget(passwordLabel);
    QLineEdit* passwordEdit = new QLineEdit(&dialog);
    passwordEdit->setPlaceholderText("Минимум 8 символов");
    passwordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(passwordEdit);

    QLabel* confirmPasswordLabel = new QLabel("Подтвердите пароль:", &dialog);
    layout->addWidget(confirmPasswordLabel);
    QLineEdit* confirmPasswordEdit = new QLineEdit(&dialog);
    confirmPasswordEdit->setPlaceholderText("Повторите пароль");
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(confirmPasswordEdit);

    // Добавляем растягивающийся элемент
    layout->addStretch();

    // Кнопки
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    QPushButton* registerButton = new QPushButton("Зарегистрироваться", &dialog);
    registerButton->setProperty("type", "primary");
    QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
    cancelButton->setProperty("type", "secondary");

    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(registerButton);
    layout->addLayout(buttonLayout);

    bool accepted = false;

    // Обработчик кнопки регистрации
    connect(registerButton, &QPushButton::clicked, [&]() {
        // Проверяем заполнение всех полей
        if (firstNameEdit->text().isEmpty() || lastNameEdit->text().isEmpty() ||
            phoneEdit->text().isEmpty() || emailEdit->text().isEmpty() ||
            passwordEdit->text().isEmpty() || confirmPasswordEdit->text().isEmpty()) {
            QMessageBox::warning(&dialog, "Ошибка", "Все поля должны быть заполнены.");
            return;
        }

        // Проверяем длину пароля
        if (passwordEdit->text().length() < 8) {
            QMessageBox::warning(&dialog, "Ошибка", "Пароль должен содержать минимум 8 символов.");
            return;
        }

        // Проверяем совпадение паролей
        if (passwordEdit->text() != confirmPasswordEdit->text()) {
            QMessageBox::warning(&dialog, "Ошибка", "Пароли не совпадают.");
            return;
        }

        // Проверяем формат email
        QRegularExpression emailRegex(R"((\w+)(\.\w+)*@(\w+)(\.\w{2,})+)");
        if (!emailRegex.match(emailEdit->text()).hasMatch()) {
            QMessageBox::warning(&dialog, "Ошибка", "Введите корректный email адрес.");
            return;
        }

        // Проверяем формат телефона (11 цифр, может начинаться с +)
        QString phone = phoneEdit->text();
        if (phone.startsWith("+")) {
            phone = phone.mid(1);
        }
        QRegularExpression phoneRegex("^[0-9]{11}$");
        if (!phoneRegex.match(phone).hasMatch()) {
            QMessageBox::warning(&dialog, "Ошибка", "Введите корректный номер телефона (11 цифр).");
            return;
        }

        // Проверяем, не существует ли уже пользователь с таким email или телефоном
        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT id FROM clients WHERE email = :email OR phone = :phone");
        checkQuery.bindValue(":email", emailEdit->text());
        checkQuery.bindValue(":phone", phone);

        if (checkQuery.exec() && checkQuery.next()) {
            QMessageBox::warning(&dialog, "Ошибка", "Пользователь с таким email или телефоном уже существует.");
            return;
        }

        // Хешируем пароль напрямую
        QString hashedPassword = QString(QCryptographicHash::hash(
            passwordEdit->text().toUtf8(),
            QCryptographicHash::Sha256).toHex());

        // Создаем нового пользователя
        QSqlQuery query;
        query.prepare("INSERT INTO clients (first_name, last_name, phone, email, password) "
                     "VALUES (:first_name, :last_name, :phone, :email, :password)");
        query.bindValue(":first_name", firstNameEdit->text());
        query.bindValue(":last_name", lastNameEdit->text());
        query.bindValue(":phone", phone);
        query.bindValue(":email", emailEdit->text());
        query.bindValue(":password", hashedPassword);

        if (query.exec()) {
            QMessageBox::information(&dialog, "Успех", "Регистрация успешно завершена.\nТеперь вы можете войти в систему, используя email и пароль.");
            accepted = true;
            dialog.accept();
        } else {
            QMessageBox::critical(&dialog, "Ошибка", "Не удалось создать учетную запись: " + query.lastError().text());
        }
    });

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted && accepted) {
        // Очищаем поля логина после успешной регистрации
        ui->lineEdit_login->clear();
        ui->lineEdit_password->clear();
    }
}

void MainWindow::on_btn_profile_clicked()
{
    if (!m_user || m_user->GetRole() != Role::User) {
        QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для доступа к профилю.");
        return;
    }

    if (ui->stackedWidget->currentWidget() == ui->user_page) return;
    
    // Проверяем, есть ли у пользователя покупки
    QList<Products::ProductKey> purchasedProducts = GetPurchasedProducts(m_user->GetId());
    bool hasPurchases = !purchasedProducts.isEmpty();
    
    qDebug() << "Profile: User" << m_user->GetId() << "has" << purchasedProducts.size() << "purchases";
    
    // Скрываем/показываем секцию "Приобретено ранее" в зависимости от наличия покупок
    if (ui->groupBox_6) {
        ui->groupBox_6->setVisible(hasPurchases);
        qDebug() << "groupBox_6 (label) visibility set to:" << hasPurchases;
    }
    if (ui->scrollArea_purchased_cars) {
        ui->scrollArea_purchased_cars->setVisible(hasPurchases);
        qDebug() << "scrollArea_purchased_cars visibility set to:" << hasPurchases;
    }
    
    // Обновляем виджет купленных товаров только если есть покупки
    if (hasPurchases && m_product_cards && m_products) {
        m_product_cards->UpdatePurchasedProductsWidget(ui->scrollArea_purchased_cars, m_user->GetId());
        qDebug() << "Updated purchased products widget for user" << m_user->GetId();
    } else {
        qDebug() << "No purchases found for user" << m_user->GetId() << "- not updating widget";
    }

    SetupServicesScrollArea();

    if (ui->label_clientname) ui->label_clientname->setText(m_user->GetName() + " — профиль");

    ui->stackedWidget->setCurrentWidget(ui->user_page);
}

void MainWindow::on_btn_sortByColor_clicked()
{
    if (m_user.get()) {
        if (!m_product_cards->hidden_to_cart_buttons_IsEmpty())
        {
            m_product_cards->RestoreHiddenToCartButtons();
        }
        if (m_user->GetRole() == Role::User)
        {
            bool ok;
            QString selectedColor = QInputDialog::getItem(
                this,                   // Родительский виджет (this, чтобы окно было модальным относительно MainWindow)
                "Поиск по цветам", // Заголовок окна
                "Выберите цвет:",       // Текст перед выпадающим списком
                m_products->GetAvailableColors(), // Список цветов
                0,                      // Индекс выбранного элемента по умолчанию
                false,                  // Редактируемый ли список
                &ok                     // Успешно ли выбрано значение
                );

            SelectionProcessing(ok, QString("Смотреть всё"), selectedColor);
            return;
        }
    }
    QMessageBox::warning(this, "Ошибка", "Чтобы переключаться по остальным разделам, необходимо авторизоваться как пользователь.");
}

void MainWindow::on_btn_search_clicked()
{
    bool ok;
    QString term = QInputDialog::getText(
        this,
        "Поиск",
        "Укажите поисковый запрос:",
        QLineEdit::Normal,
        "",
        &ok
        );

    if (ok && !term.isEmpty())
    {
        ProductInfo product;
        product.name_ = term;

        int relevant_results = m_product_cards->DrawRelevantProducts(ui->scrollArea_catalog, product.name_);
        if (relevant_results > 0)
        {
            ui->stackedWidget->setCurrentWidget(ui->main);
            QMessageBox::information(this, "Поиск", "Найдено " + QString::number(relevant_results) + " результатов по запросу «" + term + "».");
        }
        else
        {
            QMessageBox::warning(this, "Поиск", "Отсутствуют релевантные результаты.");
        }
    }
}

void MainWindow::on_btn_sortByType_clicked()
{
    if (m_user.get())
    {
        if (!m_product_cards->hidden_to_cart_buttons_IsEmpty())
        {
            m_product_cards->RestoreHiddenToCartButtons();
        }
        if (m_user->GetRole() == Role::User)
        {
            QStringList types;  // Список типов авто для выпадающего списка

            QSqlQuery query("SELECT name FROM car_types");
            while (query.next()) {
                types << query.value("name").toString();
            }
            types << "Смотреть всё"; // В конец добавляем пункт "Смотреть всё"

            bool ok;
            QString selected_type = QInputDialog::getItem(
                this,                 // Родительский виджет (this, чтобы окно было модальным относительно MainWindow)
                "Поиск по типу авто", // Заголовок окна
                "Тип:",               // Текст перед выпадающим списком
                types,                // Список элементов
                0,                    // Индекс выбранного элемента по умолчанию
                false,                // Редактируемый ли список
                &ok                   // Успешно ли выбрано значение
                );

            SelectionProcessing(ok, selected_type);
            return;
        }
    }
    QMessageBox::warning(this, "Ошибка", "Чтобы переключаться по остальным разделам, необходимо авторизоваться как пользователь.");
}

void MainWindow::SetupFloatingMenu()
{
    if (!m_floating_widget) {
        m_floating_widget.reset(new FloatingWidget(this));
    }

    m_floating_widget->BuildFloatingMenu(
        58,                                                 // Позиция виджета по оси Х
        58,                                                 // Позиция виджета по оси Y
        [this]() { this->on_btn_sortByType_clicked(); },    // Соритровка по типам
        [this]() { this->on_btn_search_clicked(); },        // Поиск по запросу
        [this]() { this->on_btn_sortByColor_clicked(); },   // Сортировка по цветам
        [this]() { this->on_btn_profile_clicked(); }        // Переход на страницу профиля
    );

    // Скрываем контейнер бокового меню по умолчанию
    if (m_floating_widget) {
        m_floating_widget->setVisible(false);
    }
}

