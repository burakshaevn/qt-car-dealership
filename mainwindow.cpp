#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMainWindow>
#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QCalendarWidget>
#include <QPushButton>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(1100, 560);

    // Автоматически подключаемся к базе данных и открываем её
    db_manager_ = std::make_shared<DatabaseHandler>();
    db_manager_->LoadDefault();

    // Переключаем пользователя на экран логина после запуска
    ui->stackedWidget->setCurrentWidget(ui->login);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::UpdateUser(const UserInfo& user, QWidget* parent)
{
    user_ = std::make_unique<User>(user, parent);
}

void MainWindow::on_pushButton_login_clicked()
{
    auto queryResult = db_manager_->ExecuteSelectQuery(QString("SELECT * FROM public.admins WHERE username = '%1';").arg(ui->lineEdit_login->text()));

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

                table_ = std::make_unique<Table>(db_manager_, user_.get(), nullptr);
                table_->BuildAdminTables();

                connect(table_.get(), &Table::Logout, this, &MainWindow::on_pushButton_logout_clicked);

                ui->stackedWidget->addWidget(table_.get());
                ui->stackedWidget->setCurrentWidget(table_.get());
            }
            else
            {
                QMessageBox::critical(this, "Авторизация", "Неверный логин или пароль.");
            }
        }
        else
        {
            auto queryResult = db_manager_->ExecuteSelectQuery(QString("SELECT * FROM public.clients WHERE email = '%1';").arg(ui->lineEdit_login->text()));

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

                        products_->PullProducts();

                        product_card_->UpdateProductsWidget(ui->scrollArea_catalog, QString("Смотреть всё"), QString("Белый"));
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
    qDebug() << "Logout: Starting...";

    if (!this->ui->stackedWidget)
    {
        qDebug() << "Logout: No stacked widget";
        return;
    }

    if (user_  && user_->GetRole() == Role::User)
    {
        qDebug() << "Logout: Cleaning up user interface";

        floating_widgets_.reset();
        products_.reset();
        cart_.reset();
        product_card_.reset();
        db_manager_.reset();

        // Очищаем scroll areas
        if (ui->scrollArea_catalog && ui->scrollArea_catalog->widget())
        {
            QWidget* oldWidget = ui->scrollArea_catalog->takeWidget();
            if (oldWidget)
            {
                oldWidget->deleteLater();
            }
        }

        if (ui->scrollArea_purchased_cars && ui->scrollArea_purchased_cars->widget())
        {
            QWidget* oldWidget = ui->scrollArea_purchased_cars->takeWidget();
            if (oldWidget)
            {
                oldWidget->deleteLater();
            }
        }

        // Очищаем кнопки сервисов
        qDebug() << "Logout: Cleaning up service buttons";
        for (QPushButton* button : service_buttons_)
        {
            if (button)
            {
                button->deleteLater();
            }
        }
        service_buttons_.clear();
    }

    qDebug() << "Logout: Resetting user";
    user_.reset();
    table_.reset();

    // Переключение на экран логина
    if (this->ui->stackedWidget && this->ui->login)
    {
        qDebug() << "Logout: Switching to login screen";
        this->ui->stackedWidget->setCurrentWidget(this->ui->login);
    }

    qDebug() << "Logout: Complete";
}

QList<Products::ProductKey> MainWindow::GetPurchasedProducts(int user_id) const {
    QList<Products::ProductKey> products_; // Список названий купленных инструментов
    qDebug() << "GetPurchasedProducts: Fetching products for user" << user_id;

    QSqlQuery query;
    QString query_str = QString(
        "SELECT c.name, c.color "
        "FROM cars c "
        "INNER JOIN purchases p ON c.id = p.car_id "
        "WHERE p.client_id = %1"
    ).arg(user_id);

    qDebug() << "GetPurchasedProducts: Executing query:" << query_str;

    if (!query.exec(query_str)) {
        qWarning() << "GetPurchasedProducts: Failed to execute query:" << query.lastError().text();
        return products_;
    }

    while (query.next()) {
        QString name = query.value("name").toString();
        QString color = query.value("color").toString();
        Products::ProductKey key = std::make_tuple(name, color);
        products_.append(key);
        qDebug() << "GetPurchasedProducts: Found product:" << name << "color:" << color;
    }

    qDebug() << "GetPurchasedProducts: Total products found:" << products_.size();
    return products_;
}

void MainWindow::BuildDependencies() {
    if (!db_manager_) {
        db_manager_ = std::make_shared<DatabaseHandler>();
        db_manager_->LoadDefault();
    }

    if (!floating_widgets_) {
        floating_widgets_ = std::make_unique<FloatingWidgets>(db_manager_, this);
        SetupFloatingMenu();
    }

    if (!product_card_ && !cart_ && !products_) {
        product_card_ = std::make_shared<ProductCard>(db_manager_, nullptr, this);
        cart_ = std::make_shared<Cart>(product_card_, this);
        products_ = std::make_shared<Products>(product_card_, cart_, db_manager_);
        // connect(products_.get(), &Products::OpenInfoPage, this, &MainWindow::ShowProductOnPersonalPage);

        connect(products_.get(), &Products::OpenInfoPage, this, [this](const ProductInfo& product) {
            auto current_product_colors_ = products_->PullAvailableColorsForProduct(product);
            ShowProductOnPersonalPage(product, current_product_colors_);
        });

        // connect(products_.get(), &Products::CartUpdated, this, [this]{
        //     ui->label_cart_total->setText("Корзина — " + FormatPrice(cart_->GetTotalCost()) + " руб.");
        // });

        // Устанавливаем связь между ProductCard и Instruments
        product_card_->SetProductsPtr(products_);
    }

    // Устанавливаем фильтр событий для панели уведомлений
    ui->notifications_panel->installEventFilter(this);
}

void MainWindow::SetupFloatingMenu() {
    floating_widgets_->BuildFloatingMenu(
        [this]() { this->SortByProductType(); }, // Соритровка по типам
        [this]() { this->SearchByTerm(); }, // Поиск по запросу
        [this]() { this->SortByColor(); }, // Сортировка по цветам
        [this]() { this->ProfileClicked(); } // Переход на страницу профиля
        );
}

void MainWindow::SortByProductType()
{
    if (user_.get())
    {
        if (!product_card_->hidden_to_cart_buttons_IsEmpty())
        {
            product_card_->RestoreHiddenToCartButtons();
        }
        if (user_->GetRole() == Role::User)
        {
            QStringList types;  // Список типов для выпадающего списка

            QSqlQuery query("SELECT name FROM car_types");
            while (query.next()) {
                types << query.value("name").toString();
            }
            types << "Смотреть всё"; // В конец добавляем пункт "Смотреть всё"

            bool ok;
            QString selected_type = QInputDialog::getItem(
                this, // Родительский виджет (this, чтобы окно было модальным относительно MainWindow)
                "Выберите тип", // Заголовок окна
                "Тип:", // Текст перед выпадающим списком
                types, // Список элементов
                0, // Индекс выбранного элемента по умолчанию
                false, // Редактируемый ли список
                &ok // Успешно ли выбрано значение
                );

            // Обработка выбора
            SelectionProcessing(ok, selected_type);
            return;
        }
    }
    QMessageBox::warning(this, "Ошибка", "Чтобы переключаться по остальным разделам, необходимо авторизоваться как пользователь.");
}
void MainWindow::ProfileClicked() {
    if (!user_ || user_->GetRole() != Role::User) {
        QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для доступа к профилю.");
        return;
    }

    qDebug() << "ProfileClicked: User ID:" << user_->GetId();

    if (ui->stackedWidget->currentWidget() == ui->user_page) {
        return;
    }

    // Скрываем боковое меню
    if (floating_widgets_ && floating_widgets_->GetSideMenu()) {
        floating_widgets_->GetSideMenu()->setVisible(false);
    }

    if (product_card_ && products_) {
        // Обновляем список купленных автомобилей
        product_card_->UpdatePurchasedProductsWidget(ui->scrollArea_purchased_cars, user_->GetId());
    }

    SetupServicesScrollArea();

    if (ui->label_clientname) {
        ui->label_clientname->setText(user_->GetName() + " — профиль");
    }

    ui->stackedWidget->setCurrentWidget(ui->user_page);
}

void MainWindow::SetupServicesScrollArea()
{
    // Настройка QScrollArea
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
            if (!user_)
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
                                           .arg(user_->GetId())
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
                    QSqlQuery query;
                    QString queryStr = QString(
                        "INSERT INTO rental_requests (client_id, car_id, start_date, rental_days, status) "
                        "VALUES (%1, %2, '%3', %4, 'не обработано');")
                        .arg(user_->GetId())
                        .arg(carCombo->currentData().toInt())
                        .arg(QDate::currentDate().toString("yyyy-MM-dd"))
                        .arg(termEdit->value());

                    if (query.exec(queryStr)) {
                        QMessageBox::information(this, "Успех",
                            QString("Заявка на аренду на %1 дней создана.\nДата начала: %2")
                                .arg(termEdit->value())
                                .arg(QDate::currentDate().toString("dd.MM.yyyy")));
                    } else {
                        QMessageBox::critical(this, "Ошибка",
                            "Не удалось создать заявку: " + query.lastError().text());
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
                    QSqlQuery query;
                    QString queryStr = QString(
                                           "INSERT INTO loan_requests (client_id, car_id, loan_amount, loan_term_months, status) "
                                           "VALUES (%1, %2, %3, %4, 'не обработано');")
                                           .arg(user_->GetId())
                                           .arg(carCombo->currentData().toInt())
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
                                           .arg(user_->GetId())
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
void MainWindow::handleServiceRequest() {
    qDebug() << "Handling service request";

    if (!user_) {
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

    if (dialog.exec() == QDialog::Accepted && accepted) {
        QDateTime selectedDateTime(calendar->selectedDate(), timeEdit->time());
        QString serviceType = serviceTypeCombo->currentText();

        QSqlQuery query;
        QString queryStr = QString(
            "INSERT INTO service_requests (client_id, service_type, scheduled_date, status) "
            "VALUES (%1, '%2', '%3', 'не обработано');")
            .arg(user_->GetId())
            .arg(serviceType)
            .arg(selectedDateTime.toString("yyyy-MM-dd HH:mm:ss"));

        if (query.exec(queryStr)) {
            QMessageBox::information(this, "Успех",
                QString("Заявка на %1 создана на %2")
                    .arg(serviceType)
                    .arg(selectedDateTime.toString("dd.MM.yyyy HH:mm")));
        } else {
            QMessageBox::critical(this, "Ошибка",
                "Не удалось создать заявку: " + query.lastError().text());
        }
    }
}

void MainWindow::handleRentalRequest() {
    qDebug() << "Handling rental request";

    if (!user_) {
        QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для доступа к сервису аренды.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Заявка на аренду");
    dialog.setFixedSize(400, 400);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    // Car selection
    QComboBox* carCombo = new QComboBox(&dialog);
    QSqlQuery carQuery("SELECT id, CONCAT(name, ' (', color, ')') as display_name FROM cars WHERE available_for_rent = true ORDER BY name, color");
    while (carQuery.next()) {
        carCombo->addItem(carQuery.value("display_name").toString(), carQuery.value("id").toInt());
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

    if (dialog.exec() == QDialog::Accepted && accepted) {
        QDate startDate = calendar->selectedDate();
        int carId = carCombo->currentData().toInt();
        int rentalDays = daysSpinBox->value();

        QSqlQuery query;
        QString queryStr = QString(
            "INSERT INTO rental_requests (client_id, car_id, start_date, rental_days, status) "
            "VALUES (%1, %2, '%3', %4, 'не обработано');")
            .arg(user_->GetId())
            .arg(carId)
            .arg(startDate.toString("yyyy-MM-dd"))
            .arg(rentalDays);

        if (query.exec(queryStr)) {
            QMessageBox::information(this, "Успех",
                QString("Заявка на аренду на %1 дней создана.\nДата начала: %2")
                    .arg(rentalDays)
                    .arg(startDate.toString("dd.MM.yyyy")));
        } else {
            QMessageBox::critical(this, "Ошибка",
                "Не удалось создать заявку: " + query.lastError().text());
        }
    }
}

void MainWindow::handleLoanRequest() {
    qDebug() << "Handling loan request";

    if (!user_) {
        QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для доступа к кредитованию.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Заявка на кредит");
    dialog.setFixedSize(400, 300);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    // Loan amount
    QDoubleSpinBox* amountSpinBox = new QDoubleSpinBox(&dialog);
    amountSpinBox->setRange(100000, 10000000);
    amountSpinBox->setSingleStep(50000);
    amountSpinBox->setValue(1000000);
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

    if (dialog.exec() == QDialog::Accepted && accepted) {
        double loanAmount = amountSpinBox->value();
        int loanTerm = termSpinBox->value();

        QSqlQuery query;
        QString queryStr = QString(
            "INSERT INTO loan_requests (client_id, loan_amount, loan_term, status) "
            "VALUES (%1, %2, %3, 'не обработано');")
            .arg(user_->GetId())
            .arg(loanAmount)
            .arg(loanTerm);

        if (query.exec(queryStr)) {
            QMessageBox::information(this, "Успех",
                QString("Заявка на кредит на сумму %1 руб. на %2 месяцев создана.")
                    .arg(FormatPrice(loanAmount))
                    .arg(loanTerm));
        } else {
            QMessageBox::critical(this, "Ошибка",
                "Не удалось создать заявку: " + query.lastError().text());
        }
    }
}

void MainWindow::handleInsuranceRequest() {
    qDebug() << "Handling insurance request";

    if (!user_) {
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
        .arg(user_->GetId());

    if (carQuery.exec(carQueryStr)) {
        while (carQuery.next()) {
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
        if (carCombo->currentText().isEmpty()) {
            QMessageBox::warning(&dialog, "Ошибка",
                "У вас нет автомобилей для страхования. Сначала приобретите автомобиль.");
            return;
        }
        accepted = true;
        dialog.accept();
    });
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted && accepted) {
        QString insuranceType = insuranceTypeCombo->currentText();
        QString carName = carCombo->currentText();

        QSqlQuery query;
        QString queryStr = QString(
            "INSERT INTO insurance_requests (client_id, insurance_type, car_name, status) "
            "VALUES (%1, '%2', '%3', 'не обработано');")
            .arg(user_->GetId())
            .arg(insuranceType)
            .arg(carName);

        if (query.exec(queryStr)) {
            QMessageBox::information(this, "Успех",
                QString("Заявка на страхование %1 для автомобиля %2 создана.")
                    .arg(insuranceType)
                    .arg(carName));
        } else {
            QMessageBox::critical(this, "Ошибка",
                "Не удалось создать заявку: " + query.lastError().text());
        }
    }
}

void MainWindow::SortByColor()
{
    if (user_.get()) {
        if (!product_card_->hidden_to_cart_buttons_IsEmpty())
        {
            product_card_->RestoreHiddenToCartButtons();
        }
        if (user_->GetRole() == Role::User)
        {
            bool ok;
            QString selectedColor = QInputDialog::getItem(
                this, // Родительский виджет (this, чтобы окно было модальным относительно MainWindow)
                "Сортировка по цветам", // Заголовок окна
                "На экране будут отображены предметы, соответствующие выбранному цвету.", // Текст перед выпадающим списком
                products_->GetAvailableColors(), // Список цветов
                0, // Индекс выбранного элемента по умолчанию
                false, // Редактируемый ли список
                &ok // Успешно ли выбрано значение
                );

            // Обработка выбора
            SelectionProcessing(ok, QString("Смотреть всё"), selectedColor);
            return;
        }
    }
    QMessageBox::warning(this, "Ошибка", "Чтобы переключаться по остальным разделам, необходимо авторизоваться как пользователь.");
}

void MainWindow::SearchByTerm()
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

        int relevant_results = product_card_->DrawRelevantProducts(ui->scrollArea_catalog, product.name_);
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

void MainWindow::SelectionProcessing(const bool ok, const QStringView selected_type, const QStringView selected_color)
{
    if (ok && !selected_type.isEmpty())
    {
        if (selected_type == "Смотреть всё")
        {
            product_card_->UpdateProductsWidget(ui->scrollArea_catalog, QString("Смотреть всё"), selected_color);
            ui->stackedWidget->setCurrentWidget(ui->main);
            // qDebug() << "Выбрано: Смотреть всё";
        }
        else
        {
            product_card_->UpdateProductsWidget(ui->scrollArea_catalog, selected_type.toString(), QString("Белый"));
            ui->stackedWidget->setCurrentWidget(ui->main);
            // qDebug() << "Выбран тип:" << selected_type;
        }
    }
}

void MainWindow::CleanCart()
{
    for (auto& [name_, card_] : cart_->GetCart())
    {
        if (card_)
        {
            auto to_cart_button = card_->findChild<QPushButton*>("to_cart_", Qt::FindChildrenRecursively);
            if (to_cart_button)
            {
                to_cart_button->setIcon(QIcon(":/bookmark.svg"));
            }
        }
    }
    product_card_->HideOldCards();
    ui->label_cart_total->setText("Корзина");
    ui->label_cart_total_2->setText("Корзина пуста.");
    cart_->ClearCart();
}

void MainWindow::ToPayCart()
{
    // Начало транзакции
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось начать транзакцию: " + db.lastError().text());
        return;
    }

    int new_id = db_manager_->GetMaxOrMinValueFromTable("MAX", "id", "purchases") + 1;
    bool success = true; // Флаг для отслеживания статуса операций

    for (auto& [name_, card_] : cart_->GetCart())
    {
        auto instrument = products_->FindProduct(name_);
        if (!db_manager_->ExecuteQuery(QString("INSERT INTO public.purchases(id, client_id, instrument_id) VALUES (%1, %2, %3);").arg(new_id).arg(user_->GetId()).arg(instrument->id_)))
        {
            success = false;
            QMessageBox::critical(this, "Ошибка", "Не удалось выполнить операцию: " + db_manager_->GetLastError());
            break; // Прерываем цикл при ошибке
        }
        ++new_id;
    }

    // Если все операции успешны, фиксируем транзакцию
    if (success)
    {
        if (!db.commit())
        {
            QMessageBox::critical(this, "Ошибка", "Не удалось зафиксировать транзакцию: " + db.lastError().text());
        }
        else
        {
            // All succsess
        }
    }
    else
    {
        if (!db.rollback()) // Если есть ошибки, откатываем транзакцию
        {
            QMessageBox::critical(this, "Ошибка", "Не удалось откатить транзакцию: " + db.lastError().text());
        }
    }
}

void MainWindow::on_pushButton_clean_cart_clicked()
{
    if (cart_->CartIsEmpty())
    {
        QMessageBox::warning(this, "Предупреждение", "Невозможно очистить корзину: отсутствует содержимое.");
        return;
    }
    CleanCart();
    QMessageBox::information(this, "", "Корзина очищена.");
}

void MainWindow::on_pushButton_submit_cart_clicked()
{
    if (cart_->CartIsEmpty())
    {
        QMessageBox::warning(this, "Предупреждение", "Невозможно выполить оплату: отсутствует содержимое.");
        return;
    }
    ToPayCart();
    CleanCart();
    QMessageBox::information(this, "", "Произведена оплата. Купленные инструменты добавлены в профиль.");
}


void MainWindow::ShowProductOnPersonalPage(const ProductInfo& product, QList<ProductInfo>& current_product_colors_)
{
    ui->label_name->setText(product.name_);
    ui->label_price->setText(FormatPrice(product.price_) + " руб.");
    current_product_ = product;
    // ui->label_color_index->setText(QString::number(current_color_index_ + 1) + "/" + QString::number(current_product_colors_.size()));

    QString imagePath = QDir::cleanPath(product.image_path_);
    QPixmap originalPixmap(imagePath);

    if (!originalPixmap.isNull())
    {
        int fixedWidth = 394;
        QPixmap scaledPixmap = originalPixmap.scaled(fixedWidth, originalPixmap.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        int imageHeight = scaledPixmap.height();
        int labelNameY = ui->label_name->y();
        int imageY = labelNameY - imageHeight - 18;

        ui->label_car_image->setPixmap(scaledPixmap);
        ui->label_car_image->setFixedSize(fixedWidth, imageHeight);
        ui->label_car_image->move(353, imageY);
        ui->label_car_image->show();
    }

    ui->stackedWidget->setCurrentWidget(ui->personal);
}

void MainWindow::on_pushButton_next_left_clicked()
{
    // Доступные варианты цветов для current_product
    auto current_product_colors_ = products_->PullAvailableColorsForProduct(current_product_);
    if (!current_product_colors_.isEmpty())
    {
        current_color_index_ = (current_color_index_ - 1 + current_product_colors_.size()) % current_product_colors_.size();
        current_product_ = current_product_colors_.at(current_color_index_);
        ShowProductOnPersonalPage(current_product_colors_.at(current_color_index_), current_product_colors_);
    }
}

void MainWindow::on_pushButton_next_right_clicked()
{
    // Доступные варианты цветов для current_product
    auto current_product_colors_ = products_->PullAvailableColorsForProduct(current_product_);
    if (!current_product_colors_.isEmpty())
    {
        current_color_index_ = (current_color_index_ + 1) % current_product_colors_.size();
        current_product_ = current_product_colors_.at(current_color_index_);
        ShowProductOnPersonalPage(current_product_colors_.at(current_color_index_), current_product_colors_);
    }
}

void MainWindow::on_pushButton_back_clicked()
{
    if (!user_ || user_->GetRole() != Role::User) {
        return;
    }

    qDebug() << "Back button: Starting cleanup...";

    // Clean up scroll areas
    if (ui->scrollArea_catalog && ui->scrollArea_catalog->widget()) {
        QWidget* oldCatalogWidget = ui->scrollArea_catalog->takeWidget();
        if (oldCatalogWidget) {
            qDebug() << "Back button: Cleaning up catalog widget";
            oldCatalogWidget->hide();
            oldCatalogWidget->deleteLater();
        }
    }

    if (ui->scrollArea_purchased_cars && ui->scrollArea_purchased_cars->widget()) {
        QWidget* oldPurchasedWidget = ui->scrollArea_purchased_cars->takeWidget();
        if (oldPurchasedWidget) {
            qDebug() << "Back button: Cleaning up purchased cars widget";
            oldPurchasedWidget->hide();
            oldPurchasedWidget->deleteLater();
        }
    }

    // Reset current product state
    current_product_ = ProductInfo();
    current_color_index_ = 0;

    // Show the side menu if it exists
    if (floating_widgets_ && floating_widgets_->GetSideMenu()) {
        floating_widgets_->GetSideMenu()->setVisible(true);
    }

    // Rebuild the main catalog view
    if (product_card_) {
        qDebug() << "Back button: Rebuilding catalog view";
        product_card_->UpdateProductsWidget(ui->scrollArea_catalog, QString("Смотреть всё"), QString("Белый"));
    }

    qDebug() << "Back button: Switching to main page";
    ui->stackedWidget->setCurrentWidget(ui->main);
    qDebug() << "Back button: Cleanup complete";
}

void MainWindow::on_pushButton_info_clicked()
{
    if (current_product_.name_.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Не выбран автомобиль для просмотра информации.");
        return;
    }

    // Формируем детальную информацию об автомобиле
    QString info = QString("Название: %1\nЦена: %2 руб.\nЦвет: %3\nОписание: %4")
                       .arg(current_product_.name_)
                       .arg(FormatPrice(current_product_.price_))
                       .arg(current_product_.color_)
                       .arg(current_product_.description_.isEmpty() ? "Описание отсутствует." : current_product_.description_);

    QMessageBox::information(this, "Информация об автомобиле", info);
}

void MainWindow::on_pushButton_test_drive_clicked()
{
    if (!user_) {
        QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для записи на тест-драйв.");
        return;
    }

    if (current_product_.name_.isEmpty()) {
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
        .arg(current_product_.name_)
        .arg(current_product_.color_)
        .arg(FormatPrice(current_product_.price_)), &dialog);
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
        accepted = true;
        dialog.accept();
    });
    connect(cancelButton, &QPushButton::clicked, [&]() {
        dialog.reject();
    });

    if (dialog.exec() == QDialog::Accepted && accepted) {
        QDate selectedDate = calendar->selectedDate();
        QTime selectedTime = timeEdit->time();
        QString dateTime = selectedDate.toString("yyyy-MM-dd") + " " + selectedTime.toString("HH:mm");

        QSqlQuery query;
        QString queryStr = QString("INSERT INTO test_drives (client_id, car_id, scheduled_date, status) "
                                 "VALUES (%1, %2, '%3', 'не обработано');")
                                 .arg(user_->GetId())
                                 .arg(current_product_.id_)
                                 .arg(dateTime);

        if (query.exec(queryStr)) {
            QMessageBox::information(this, "Успех", QString("Вы записаны на тест-драйв автомобиля %1\nДата: %2\nВремя: %3")
                .arg(current_product_.name_)
                .arg(selectedDate.toString("dd.MM.yyyy"))
                .arg(selectedTime.toString("HH:mm")));
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось записаться на тест-драйв: " + query.lastError().text());
        }
    }
}

void MainWindow::on_pushButton_to_pay_clicked()
{
    if (!user_) {
        QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для оформления заявки.");
        return;
    }

    if (current_product_.name_.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Не выбран автомобиль для оформления заявки.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Оформление заявки на покупку");
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
        .arg(current_product_.name_)
        .arg(current_product_.color_)
        .arg(FormatPrice(current_product_.price_)), &dialog);
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

    if (dialog.exec() == QDialog::Accepted && accepted) {
        QSqlQuery query;
        
        // Прямая покупка
        if (buyCheckBox->isChecked()) {
            QString queryStr = QString("INSERT INTO purchases (client_id, car_id) VALUES (%1, %2);")
                .arg(user_->GetId())
                .arg(current_product_.id_);

            if (query.exec(queryStr)) {
                QMessageBox::information(this, "Успех", QString("Поздравляем с покупкой автомобиля %1!")
                    .arg(current_product_.name_));
            } else {
                QMessageBox::critical(this, "Ошибка", "Не удалось оформить покупку: " + query.lastError().text());
                return;
            }
        }

        // Оформление кредита
        if (loanCheckBox->isChecked()) {
            int months = loanTermCombo->currentText().split(" ")[0].toInt();
            QString queryStr = QString(
                "INSERT INTO loan_requests (client_id, car_id, loan_amount, loan_term_months, status) "
                "VALUES (%1, %2, %3, %4, 'не обработано');")
                .arg(user_->GetId())
                .arg(current_product_.id_)
                .arg(current_product_.price_)
                .arg(months);

            if (!query.exec(queryStr)) {
                QMessageBox::critical(this, "Ошибка", "Не удалось оформить заявку на кредит: " + query.lastError().text());
                return;
            }
        }

        // Оформление страховки
        if (insuranceCheckBox->isChecked()) {
            QString queryStr = QString(
                "INSERT INTO insurance_requests (client_id, car_id, insurance_type, status) "
                "VALUES (%1, %2, '%3', 'не обработано');")
                .arg(user_->GetId())
                .arg(current_product_.id_)
                .arg(insuranceTypeCombo->currentText());

            if (!query.exec(queryStr)) {
                QMessageBox::critical(this, "Ошибка", "Не удалось оформить заявку на страхование: " + query.lastError().text());
                return;
            }
        }

        // Оформление аренды
        if (rentalCheckBox->isChecked()) {
            QString queryStr = QString(
                "INSERT INTO rental_requests (client_id, car_id, rental_days, start_date, status) "
                "VALUES (%1, %2, %3, '%4', 'не обработано');")
                .arg(user_->GetId())
                .arg(current_product_.id_)
                .arg(rentalTermCombo->currentText().split(" ")[0].toInt())
                .arg(QDate::currentDate().toString("yyyy-MM-dd"));

            if (!query.exec(queryStr)) {
                QMessageBox::critical(this, "Ошибка", "Не удалось оформить заявку на аренду: " + query.lastError().text());
                return;
            }
        }

        QMessageBox::information(this, "Успех", "Заявка успешно оформлена!");
    }
}

void MainWindow::on_pushButton_notifications_clicked()
{
    bool isVisible = ui->notifications_panel->isVisible();
    ui->notifications_panel->setVisible(!isVisible);

    if (!isVisible)
    {
        // Проверяем наличие новых уведомлений
        if (CheckNotifications())
        {
            // Обновляем уведомления
            UpdateNotifications();

            // Поднимаем панель поверх других виджетов
            ui->notifications_panel->raise();

            // Устанавливаем фокус на панель, чтобы она оставалась поверх
            ui->notifications_panel->setFocus();
        }
        else
        {
            QMessageBox::information(this, "Информация", QString("Отсутствуют новые уведомления."));
        }
    }
}

// Добавляем обработчик события показа для панели уведомлений
bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == ui->notifications_panel) {
        if (event->type() == QEvent::Show) {
            // При показе панели поднимаем её поверх других виджетов
            ui->notifications_panel->raise();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

bool MainWindow::CheckNotifications()
{
    if (!user_ || user_->GetRole() != Role::User) {
        return false;
    }

    // Получаем количество необработанных уведомлений
    QVariant result = db_manager_->ExecuteSelectQuery(
        QString("SELECT COUNT(*) as count FROM ("
                "    SELECT id FROM service_requests "
                "    WHERE client_id = %1 AND (notification_shown = false OR notification_shown IS NULL) "
                "    UNION ALL "
                "    SELECT id FROM insurance_requests "
                "    WHERE client_id = %1 AND (notification_shown = false OR notification_shown IS NULL) "
                "    UNION ALL "
                "    SELECT id FROM loan_requests "
                "    WHERE client_id = %1 AND (notification_shown = false OR notification_shown IS NULL) "
                "    UNION ALL "
                "    SELECT id FROM rental_requests "
                "    WHERE client_id = %1 AND (notification_shown = false OR notification_shown IS NULL) "
                ") as notifications")
            .arg(user_->GetId()));

    if (result.canConvert<QSqlQuery>()) {
        QSqlQuery query = result.value<QSqlQuery>();
        if (query.next()) {
            int count = query.value("count").toInt();
            if (count > 0)
            {
                // Есть новые уведомления - меняем иконку
                ui->pushButton_notifications->setStyleSheet(
                    "QPushButton {"
                    "   background-color: #fafafa;"
                    "   border-radius: 17px;"
                    "}"
                );
                return true;
            }
            else
            {
                return false;
            }
        }
    }
}

void MainWindow::UpdateNotifications()
{
    if (!user_ || user_->GetRole() != Role::User) {
        return;
    }

    // Очищаем текущие уведомления
    ClearNotifications();

    // Получаем все необработанные уведомления
    QVariant result = db_manager_->ExecuteSelectQuery(
        QString("SELECT 'service' as type, id, status, service_type as additional_info, scheduled_date as date_info "
                "FROM service_requests "
                "WHERE client_id = %1 AND (notification_shown = false OR notification_shown IS NULL) "
                "UNION ALL "
                "SELECT 'insurance' as type, id, status, insurance_type as additional_info, created_at as date_info "
                "FROM insurance_requests "
                "WHERE client_id = %1 AND (notification_shown = false OR notification_shown IS NULL) "
                "UNION ALL "
                "SELECT 'loan' as type, id, status, CAST(loan_amount AS TEXT) as additional_info, created_at as date_info "
                "FROM loan_requests "
                "WHERE client_id = %1 AND (notification_shown = false OR notification_shown IS NULL) "
                "UNION ALL "
                "SELECT 'rental' as type, id, status, CAST(rental_days AS TEXT) as additional_info, start_date as date_info "
                "FROM rental_requests "
                "WHERE client_id = %1 AND (notification_shown = false OR notification_shown IS NULL) "
                "ORDER BY date_info DESC")
            .arg(user_->GetId()));

    if (result.canConvert<QSqlQuery>())
    {
        QSqlQuery query = result.value<QSqlQuery>();
        bool hasNotifications = false;

        while (query.next())
        {
            hasNotifications = true;
            QString type = query.value("type").toString();
            int id = query.value("id").toInt();
            QString status = query.value("status").toString();
            QString additionalInfo = query.value("additional_info").toString();
            QDateTime dateInfo = query.value("date_info").toDateTime();

            QString title;
            QString message;

            if (type == "service") {
                title = "Заявка на обслуживание";
                message = QString("Статус заявки на %1\nЗапланировано на: %2\nСтатус: %3")
                        .arg(additionalInfo)
                        .arg(dateInfo.toString("dd.MM.yyyy HH:mm"))
                        .arg(status);

                // Добавляем кнопку скачивания договора для подтвержденных заявок
                if (status == "подтверждено") {
                    // Получаем информацию об автомобиле
                    QSqlQuery carQuery;
                    QString carQueryStr = QString(
                        "SELECT c.* FROM cars c "
                        "INNER JOIN service_requests sr ON c.id = sr.car_id "
                        "WHERE sr.id = %1")
                        .arg(id);

                    if (carQuery.exec(carQueryStr) && carQuery.next()) {
                        ProductInfo product;
                        product.id_ = carQuery.value("id").toInt();
                        product.name_ = carQuery.value("name").toString();
                        product.color_ = carQuery.value("color").toString();
                        product.price_ = carQuery.value("price").toDouble();

                        QPushButton* downloadButton = new QPushButton("Скачать договор", this);
                        connect(downloadButton, &QPushButton::clicked, [this, product, additionalInfo, dateInfo]() {
                            product_card_->generateAndShowServiceContract(
                                product,
                                additionalInfo,
                                dateInfo.toString("dd.MM.yyyy HH:mm")
                            );
                        });

                        message += "\n\n";
                        QWidget* container = new QWidget(this);
                        QVBoxLayout* layout = new QVBoxLayout(container);
                        layout->addWidget(new QLabel(message, container));
                        layout->addWidget(downloadButton);
                        AddNotification(title, container);
                        continue;
                    }
                }
            }
            else if (type == "insurance") {
                title = "Заявка на страхование";
                message = QString("Статус заявки на %1\nСтатус: %2")
                        .arg(additionalInfo)
                        .arg(status);

                // Добавляем кнопку скачивания договора для одобренных заявок
                if (status == "одобрено") {
                    // Получаем информацию об автомобиле
                    QSqlQuery carQuery;
                    QString carQueryStr = QString(
                        "SELECT c.* FROM cars c "
                        "INNER JOIN insurance_requests ir ON c.id = ir.car_id "
                        "WHERE ir.id = %1")
                        .arg(id);

                    if (carQuery.exec(carQueryStr) && carQuery.next()) {
                        ProductInfo product;
                        product.id_ = carQuery.value("id").toInt();
                        product.name_ = carQuery.value("name").toString();
                        product.color_ = carQuery.value("color").toString();
                        product.price_ = carQuery.value("price").toDouble();

                        QPushButton* downloadButton = new QPushButton("Скачать договор", this);
                        connect(downloadButton, &QPushButton::clicked, [this, product, additionalInfo]() {
                            product_card_->generateAndShowInsuranceContract(
                                product,
                                additionalInfo
                            );
                        });

                        message += "\n\n";
                        QWidget* container = new QWidget(this);
                        QVBoxLayout* layout = new QVBoxLayout(container);
                        layout->addWidget(new QLabel(message, container));
                        layout->addWidget(downloadButton);
                        AddNotification(title, container);
                        continue;
                    }
                }
            }
            else if (type == "loan") {
                title = "Заявка на кредит";
                message = QString("Статус заявки на сумму %1 руб.\nСтатус: %2")
                        .arg(FormatPrice(additionalInfo.toLongLong()))
                        .arg(status);

                // Добавляем кнопку скачивания договора для одобренных заявок
                if (status == "одобрено") {
                    // Получаем информацию об автомобиле и условиях кредита
                    QSqlQuery loanQuery;
                    QString loanQueryStr = QString(
                        "SELECT c.*, lr.loan_amount, lr.loan_term_months FROM cars c "
                        "INNER JOIN loan_requests lr ON c.id = lr.car_id "
                        "WHERE lr.id = %1")
                        .arg(id);

                    if (loanQuery.exec(loanQueryStr) && loanQuery.next()) {
                        ProductInfo product;
                        product.id_ = loanQuery.value("id").toInt();
                        product.name_ = loanQuery.value("name").toString();
                        product.color_ = loanQuery.value("color").toString();
                        product.price_ = loanQuery.value("price").toDouble();

                        QString loanAmount = loanQuery.value("loan_amount").toString();
                        QString loanTerm = loanQuery.value("loan_term_months").toString();

                        QPushButton* downloadButton = new QPushButton("Скачать договор", this);
                        connect(downloadButton, &QPushButton::clicked, [this, product, loanAmount, loanTerm]() {
                            product_card_->generateAndShowLoanContract(product, loanAmount, loanTerm);
                        });

                        message += "\n\n";
                        QWidget* container = new QWidget(this);
                        QVBoxLayout* layout = new QVBoxLayout(container);
                        layout->addWidget(new QLabel(message, container));
                        layout->addWidget(downloadButton);
                        AddNotification(title, container);
                        continue;
                    }
                }
            }
            else if (type == "rental") {
                title = "Заявка на аренду";
                message = QString("Статус заявки на аренду автомобиля на %1 дней\nДата начала: %2\nСтатус: %3")
                        .arg(additionalInfo)
                        .arg(dateInfo.toString("dd.MM.yyyy"))
                        .arg(status);

                // Добавляем кнопку скачивания договора для одобренных заявок
                if (status == "одобрено") {
                    // Получаем информацию об автомобиле
                    QSqlQuery carQuery;
                    QString carQueryStr = QString(
                        "SELECT c.* FROM cars c "
                        "INNER JOIN rental_requests rr ON c.id = rr.car_id "
                        "WHERE rr.id = %1")
                        .arg(id);

                    if (carQuery.exec(carQueryStr) && carQuery.next()) {
                        ProductInfo product;
                        product.id_ = carQuery.value("id").toInt();
                        product.name_ = carQuery.value("name").toString();
                        product.color_ = carQuery.value("color").toString();
                        product.price_ = carQuery.value("price").toDouble();

                        QPushButton* downloadButton = new QPushButton("Скачать договор", this);
                        connect(downloadButton, &QPushButton::clicked, [this, product, additionalInfo, dateInfo]() {
                            product_card_->generateAndShowRentalContract(
                                product,
                                additionalInfo,
                                dateInfo.toString("dd.MM.yyyy")
                            );
                        });

                        message += "\n\n";
                        QWidget* container = new QWidget(this);
                        QVBoxLayout* layout = new QVBoxLayout(container);
                        layout->addWidget(new QLabel(message, container));
                        layout->addWidget(downloadButton);
                        AddNotification(title, container);
                        continue;
                    }
                }
            }

            AddNotification(title, message);
        }
    }

    // После показа уведомлений помечаем их как прочитанные
    MarkNotificationsAsRead();
}

void MainWindow::AddNotification(const QString& title, const QString& message) {
    QWidget* container = new QWidget(ui->notifications_panel);
    QVBoxLayout* layout = new QVBoxLayout(container);
    
    // Set proper margins and spacing
    layout->setContentsMargins(20, 15, 20, 15);
    layout->setSpacing(10);

    QLabel* titleLabel = new QLabel(title, container);
    titleLabel->setStyleSheet("font: 700 16pt 'Open Sans'; color: #1d1b20;");
    titleLabel->setWordWrap(true);
    titleLabel->setMinimumHeight(30);
    layout->addWidget(titleLabel);

    QLabel* messageLabel = new QLabel(message, container);
    messageLabel->setStyleSheet("font: 14pt 'Open Sans'; color: #1d1b20;");
    messageLabel->setWordWrap(true);
    messageLabel->setMinimumHeight(25);
    layout->addWidget(messageLabel);

    container->setStyleSheet(
        "QWidget {"
        "   background-color: #ffffff;"
        "   margin-bottom: 10px;"
        "   margin-top: 10px;"
        "   margin-left: 5px;"
        "   margin-right: 5px;"
        "}"
    );
    container->setMinimumWidth(250);
    ui->verticalLayout_notifications_content->addWidget(container);
    
    // Add spacing item after the notification
    ui->verticalLayout_notifications_content->addSpacing(10);
}

void MainWindow::AddNotification(const QString& title, QWidget* content) {
    QWidget* container = new QWidget(ui->notifications_panel);
    QVBoxLayout* layout = new QVBoxLayout(container);
    
    // Set proper margins and spacing
    layout->setContentsMargins(20, 15, 20, 15);
    layout->setSpacing(10);

    QLabel* titleLabel = new QLabel(title, container);
    titleLabel->setStyleSheet("font: 700 16pt 'Open Sans'; color: #1d1b20;");
    titleLabel->setWordWrap(true);
    titleLabel->setMinimumHeight(30);
    layout->addWidget(titleLabel);

    content->setMinimumHeight(25);
    layout->addWidget(content);

    container->setStyleSheet(
        "QWidget {"
        "   background-color: #ffffff;"
        "   margin-bottom: 10px;"
        "   margin-top: 10px;"
        "   margin-left: 5px;"
        "   margin-right: 5px;"
        "}"
    );
    container->setMinimumWidth(250);
    ui->verticalLayout_notifications_content->addWidget(container);
    
    // Add spacing item after the notification
    ui->verticalLayout_notifications_content->addSpacing(10);
}

void MainWindow::ClearNotifications()
{
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(
        ui->scrollAreaWidgetContents_notifications->layout());
    if (!layout) {
        return;
    }

    // Удаляем все виджеты из layout
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void MainWindow::on_pushButton_settings_clicked()
{
    if (!user_) {
        QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для доступа к настройкам.");
        return;
    }

    // Проверяем, существует ли уже открытый диалог
    if (settings_dialog_) {
        settings_dialog_->activateWindow();
        return;
    }

    // Создаем диалог
    settings_dialog_ = std::make_unique<QDialog>(this);
    settings_dialog_->setWindowTitle("Настройки профиля");
    settings_dialog_->setFixedSize(450, 600);
    settings_dialog_->setStyleSheet(
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

    QVBoxLayout* dialogLayout = new QVBoxLayout(settings_dialog_.get());
    dialogLayout->setSpacing(10);
    dialogLayout->setContentsMargins(20, 20, 20, 20);

    // Заголовок
    QLabel* titleLabel = new QLabel("Редактирование профиля", settings_dialog_.get());
    titleLabel->setProperty("type", "header");
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    dialogLayout->addWidget(titleLabel);

    // Создаем поля ввода
    settings_first_name_ = std::make_unique<QLineEdit>(settings_dialog_.get());
    settings_last_name_ = std::make_unique<QLineEdit>(settings_dialog_.get());
    settings_email_ = std::make_unique<QLineEdit>(settings_dialog_.get());
    settings_phone_ = std::make_unique<QLineEdit>(settings_dialog_.get());
    settings_password_ = std::make_unique<QLineEdit>(settings_dialog_.get());
    settings_password_->setEchoMode(QLineEdit::Password);

    // Получаем информацию о пользователе из базы данных
    QSqlQuery query;
    QString queryStr = QString(
        "SELECT first_name, last_name, email, phone, password "
        "FROM clients WHERE id = %1")
        .arg(user_->GetId());

    if (query.exec(queryStr) && query.next()) {
        // Имя
        QLabel* firstNameLabel = new QLabel("Имя:", settings_dialog_.get());
        dialogLayout->addWidget(firstNameLabel);
        settings_first_name_->setText(query.value("first_name").toString());
        dialogLayout->addWidget(settings_first_name_.get());

        // Фамилия
        QLabel* lastNameLabel = new QLabel("Фамилия:", settings_dialog_.get());
        dialogLayout->addWidget(lastNameLabel);
        settings_last_name_->setText(query.value("last_name").toString());
        dialogLayout->addWidget(settings_last_name_.get());

        // Email
        QLabel* emailLabel = new QLabel("Email:", settings_dialog_.get());
        dialogLayout->addWidget(emailLabel);
        settings_email_->setText(query.value("email").toString());
        dialogLayout->addWidget(settings_email_.get());

        // Телефон
        QLabel* phoneLabel = new QLabel("Телефон:", settings_dialog_.get());
        dialogLayout->addWidget(phoneLabel);
        settings_phone_->setText(query.value("phone").toString());
        dialogLayout->addWidget(settings_phone_.get());

        // Пароль
        QLabel* passwordLabel = new QLabel("Пароль:", settings_dialog_.get());
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

    QPushButton* saveButton = new QPushButton("Сохранить", settings_dialog_.get());
    saveButton->setProperty("type", "primary");
    QPushButton* cancelButton = new QPushButton("Отмена", settings_dialog_.get());
    cancelButton->setProperty("type", "secondary");

    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(saveButton);
    dialogLayout->addLayout(buttonLayout);

    // Сохраняем указатель на диалог для использования в лямбда-функциях
    QDialog* dialogPtr = settings_dialog_.get();

    // Обработка нажатия кнопки "Сохранить"
    connect(saveButton, &QPushButton::clicked, dialogPtr, [this, dialogPtr]() {
        // Проверяем, что обязательные поля заполнены
        if (settings_first_name_->text().isEmpty() || settings_last_name_->text().isEmpty() ||
            settings_email_->text().isEmpty() || settings_phone_->text().isEmpty()) {
            QMessageBox::warning(dialogPtr, "Ошибка", "Все поля кроме пароля должны быть заполнены.");
            return;
        }

        // Сохраняем значения полей во временные переменные
        QString firstName = settings_first_name_->text();
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
        updateQuery += QString(" WHERE id = %1").arg(user_->GetId());

        if (db_manager_->ExecuteQuery(updateQuery)) {
            // Обновляем данные пользователя в памяти
            if (user_) {
                user_->SetName(firstName + " " + lastName);
                user_->SetEmail(email);
                // Обновляем отображаемое имя в интерфейсе
                if (ui->label_clientname) {
                    ui->label_clientname->setText(user_->GetName() + " — профиль");
                }
            }

            QMessageBox::information(dialogPtr, "Успех", "Данные профиля обновлены.");
            dialogPtr->accept();
        } else {
            QMessageBox::critical(dialogPtr, "Ошибка",
                "Не удалось обновить данные профиля: " + db_manager_->GetLastError());
        }
    });

    connect(cancelButton, &QPushButton::clicked, dialogPtr, &QDialog::reject);

    // Очистка указателей при закрытии диалога
    connect(dialogPtr, &QDialog::finished, this, [this]() {
        settings_first_name_.reset();
        settings_last_name_.reset();
        settings_email_.reset();
        settings_phone_.reset();
        settings_password_.reset();
        settings_dialog_.reset();
    });

    settings_dialog_->show();
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
            QMessageBox::information(&dialog, "Успех", 
                "Регистрация успешно завершена.\nТеперь вы можете войти в систему, используя email и пароль.");
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

void MainWindow::MarkNotificationsAsRead()
{
    if (!user_ || user_->GetRole() != Role::User) return;

    int userId = user_->GetId();
    QStringList tables = {"service_requests", "insurance_requests", "loan_requests", "rental_requests"};
    for (const QString& table : tables) {
        QString query = QString("UPDATE %1 SET notification_shown = true WHERE client_id = %2 AND notification_shown = false")
                        .arg(table)
                        .arg(userId);
        db_manager_->ExecuteQuery(query);
    }
}

