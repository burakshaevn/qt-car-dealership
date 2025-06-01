#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTimeEdit>
#include <QCalendarWidget>
#include <QScrollArea>
#include <QDir>

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

                    if (user.password_ == ui->lineEdit_password->text())
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
                    QMessageBox::critical(this, "Ошибка", "Пользователя с таким логином не существует.");
                }
            }
            else
            {
                QMessageBox::critical(this, "Ошибка в базе данных", queryResult.toString());
            }
        }
    }
    else
    {
        QMessageBox::critical(this, "Ошибка в базе данных", queryResult.toString());
    }
}

void MainWindow::on_pushButton_logout_clicked()
{
    if (!this->ui->stackedWidget)
    {
        return;
    }

    if (user_  && user_->GetRole() == Role::User)
    {
        // side_widget_.reset();
        // floating_menu_.reset();
        floating_widgets_.reset();

        products_.reset();
        cart_.reset();
        product_card_.reset();
        db_manager_.reset();

        if (ui->scrollArea_catalog)
        {
            QWidget* oldWidget = ui->scrollArea_catalog->takeWidget();
            if (oldWidget)
            {
                oldWidget->deleteLater();
            }
        }
        if (ui->scrollArea_purchased_cars)
        {
            QWidget* oldWidget = ui->scrollArea_purchased_cars->takeWidget();
            if (oldWidget)
            {
                oldWidget->deleteLater();
            }
        }
    }
    user_.reset();
    table_.reset();

    // Переключение на экран логина
    if (this->ui->stackedWidget && this->ui->login)
    {
        this->ui->stackedWidget->setCurrentWidget(this->ui->login);
    }
}

void MainWindow::SetupFloatingMenu() {
    floating_widgets_->BuildFloatingMenu(
        [this]() { this->MoreClicked(); }, // Обработка кнопки "More"
        [this]() // Обработка кнопки "Search"
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
                    QMessageBox::information(this, "Поиск", "Найдено " + QString::number(relevant_results) + " результатов по запросу «" + term + "».");
                }
                else
                {
                    QMessageBox::warning(this, "Поиск", "Отсутствуют релевантные результаты.");
                }
            }
        },
        [this]() { this->SortByColorClicked(); }, // Обработка кнопки "Сортировка по цветам"
        [this]() { this->ProfileClicked(); } // Обработка кнопки "User Profile"
    );
}

QList<Products::ProductKey> MainWindow::GetPurchasedProducts(int user_id) const {
    QList<Products::ProductKey> products_; // Список названий купленных инструментов

    QSqlQuery query;
    QString query_str = QString("SELECT * FROM CARS WHERE id IN (SELECT car_id FROM purchases WHERE client_id = %1);").arg(user_id);
    if (!query.exec(query_str)) {
        qWarning() << "Failed to execute query:" << query.lastError().text();
        return products_;
    }

    while (query.next()) {
        Products::ProductKey key = std::make_tuple(query.value(1).toString(), query.value(2).toString());
        products_.append(key);
    }

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

void MainWindow::MoreClicked()
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
    if (user_->GetRole() == Role::User)
    {
        floating_widgets_->GetSideMenu()->setVisible(false);

        product_card_->HideOldCards();
        product_card_->EnsureContainerInScrollArea(ui->scrollArea_purchased_cars);

        const QList<Products::ProductKey>& purchases_products_ = GetPurchasedProducts(user_->GetId());
        for (auto& key : purchases_products_)
        {
            const ProductInfo* found_product_struct = products_->FindProduct(key);
            if (!found_product_struct)
            {
                qDebug() << "found_product_struct is empty";
                break;
            }

            product_card_->DrawItem(*found_product_struct);
            auto to_cart_button = product_card_->FindProductCard(key)->findChild<QPushButton*>("to_cart_", Qt::FindChildrenRecursively);
            if (to_cart_button)
            {
                to_cart_button->hide();
                product_card_->hidden_to_cart_buttons_Push(key);
            }
        }
        product_card_->card_container_PerformAdjustSize();

        SetupServicesScrollArea();

        ui->label_clientname->setText(user_->GetName() + " — профиль");
        ui->stackedWidget->setCurrentWidget(ui->user_page);
    }
}
// void MainWindow::ProfileClicked()
// {
//     if (!user_ || user_->GetRole() != Role::User)
//     {
//         QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для доступа к профилю.");
//         return;
//     }

//     if (ui->stackedWidget->currentWidget() != ui->user_page)
//     {
//         QList<Products::ProductKey> products = user_->GetProducts();
//         for (auto& product : products) {
//             product_card_->DrawRelevantProducts(ui->scrollArea_purchased_cars, std::get<0>(product));
//         }

//         // if (!product_card_->hidden_to_cart_buttons_IsEmpty())
//         // {
//         //     product_card_->RestoreHiddenToCartButtons();
//         // }

//         // // Hide the side menu first
//         // if (floating_widgets_ && floating_widgets_->GetSideMenu()) {
//         //     floating_widgets_->GetSideMenu()->setVisible(false);
//         // }

//         // // Clear existing content before adding new
//         // if (product_card_) {
//         //     product_card_->HideOldCards();
//         // }
        
//         // // Important: Clear the container before adding new content
//         // if (ui->scrollArea_purchased_cars && ui->scrollArea_purchased_cars->widget()) {
//         //     QWidget* oldWidget = ui->scrollArea_purchased_cars->takeWidget();
//         //     if (oldWidget) {
//         //         oldWidget->deleteLater();
//         //     }
//         // }
        
//         // if (product_card_) {
//         //     product_card_->EnsureContainerInScrollArea(ui->scrollArea_purchased_cars);

//         //     const QList<Products::ProductKey>& purchases_products_ = GetPurchasedProducts(user_->GetId());
//         //     for (auto& key : purchases_products_)
//         //     {
//         //         const ProductInfo* found_product_struct = products_->FindProduct(key);
//         //         if (!found_product_struct)
//         //         {
//         //             qDebug() << "found_product_struct is empty";
//         //             continue;
//         //         }

//         //         product_card_->DrawItem(*found_product_struct);
//         //         QWidget* card = product_card_->FindProductCard(key);
//         //         if (card) {
//         //             auto to_cart_button = card->findChild<QPushButton*>("to_cart_", Qt::FindChildrenRecursively);
//         //             if (to_cart_button)
//         //             {
//         //                 to_cart_button->hide();
//         //                 product_card_->hidden_to_cart_buttons_Push(key);
//         //             }
//         //         }
//         //     }
//         //     product_card_->card_container_PerformAdjustSize();
//         // }

//         // // Clear existing services scroll area content
//         // if (ui->scrollArea_services && ui->scrollArea_services->widget()) {
//         //     QWidget* oldWidget = ui->scrollArea_services->takeWidget();
//         //     if (oldWidget) {
//         //         oldWidget->deleteLater();
//         //     }
//         // }

//         // // Setup services scroll area with proper cleanup
//         // SetupServicesScrollArea();

//         // if (ui->label_clientname) {
//         //     ui->label_clientname->setText(user_->GetName() + " — профиль");
//         // }
        
//         // Switch to user page
//         ui->stackedWidget->setCurrentWidget(ui->user_page);
//     }
// }

void MainWindow::SetupServicesScrollArea() {
    // Prevent recursive calls
    static bool isSettingUp = false;
    if (isSettingUp) {
        return;
    }
    isSettingUp = true;

    // Create a new container widget
    QWidget* container = new QWidget(ui->scrollArea_services);
    
    // Configure scroll area
    ui->scrollArea_services->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->scrollArea_services->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea_services->setWidgetResizable(false);
    ui->scrollArea_services->setFocusPolicy(Qt::WheelFocus);

    // Set up layout
    QHBoxLayout* layout = new QHBoxLayout(container);
    layout->setAlignment(Qt::AlignLeft);
    layout->setSpacing(21);
    layout->setContentsMargins(0, 0, 0, 0);

    // List of service names
    QStringList services = { "Обслуживание", "Аренда", "Кредитование", "Страхование" };

    // Create service cards
    for (const QString& service : services) {
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
            if (!user_) {
                QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для доступа к этому разделу.");
                return;
            }

            if (service == "Обслуживание") {
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
            else if (service == "Аренда") {
                QMessageBox::information(this, "Информация", "Для оформления заявки на аренду необходимо выбрать автомобиль.");
            }
            else if (service == "Кредитование") {
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

    // Set container size
    int containerWidth = (360 + layout->spacing()) * services.size();
    container->setMinimumWidth(containerWidth);
    container->setFixedHeight(175);

    // Set scroll area stylesheet
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
        "    background: transparent;"
        "    margin: 3px 0px 3px 0px;"
        "    border-radius: 7px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "    background: #9b9c9c;"
        "    min-width: 20px;"
        "    border-radius: 7px;"
        "}"
        "QScrollBar::add-line:horizontal {"
        "    border: none;"
        "    background: none;"
        "    width: 0px;"
        "}"
        "QScrollBar::sub-line:horizontal {"
        "    border: none;"
        "    background: none;"
        "    width: 0px;"
        "}"
        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {"
        "    background: none;"
        "}"
        "QScrollBar:horizontal {"
        "    opacity: 0;"
        "}"
        "QScrollArea:hover QScrollBar:horizontal {"
        "    opacity: 1;"
        "}"
    );

    // Set the container as the scroll area widget
    ui->scrollArea_services->setWidget(container);
    
    isSettingUp = false;
}

void MainWindow::SortByColorClicked()
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

    // Clean up any existing content in the scroll areas
    if (ui->scrollArea_catalog && ui->scrollArea_catalog->widget()) {
        QWidget* container = ui->scrollArea_catalog->widget();
        qDeleteAll(container->children());
    }

    if (ui->scrollArea_purchased_cars && ui->scrollArea_purchased_cars->widget()) {
        QWidget* container = ui->scrollArea_purchased_cars->widget();
        qDeleteAll(container->children());
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
        product_card_->UpdateProductsWidget(ui->scrollArea_catalog, QString("Смотреть всё"), QString("Белый"));
    }

    // Switch to main page
    ui->stackedWidget->setCurrentWidget(ui->main);
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


#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTimeEdit>
#include <QCalendarWidget>
#include <QMessageBox>

    void MainWindow::on_pushButton_test_drive_clicked()
{
    if (current_product_.name_.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Не выбран автомобиль для записи на тест-драйв.");
        return;
    }

    // Create a dialog for date and time selection
    QDialog dialog(this);
    dialog.setWindowTitle("Запись на тест-драйв");
    dialog.setFixedSize(400, 400);

    // Layout for the dialog
    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    // Time edit for time selection
    QTimeEdit* timeEdit = new QTimeEdit(&dialog);
    timeEdit->setDisplayFormat("HH:mm");
    timeEdit->setTime(QTime(9, 0)); // Default to 9:00 AM
    layout->addWidget(timeEdit);

    // Calendar widget for date selection
    QCalendarWidget* calendar = new QCalendarWidget(&dialog);
    calendar->setMinimumDate(QDate::currentDate()); // Prevent past dates
    layout->addWidget(calendar);

    // Buttons for confirmation
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton("OK", &dialog);
    QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    // Connect buttons
    bool accepted = false;
    connect(okButton, &QPushButton::clicked, [&]() {
        accepted = true;
        dialog.accept();
    });
    connect(cancelButton, &QPushButton::clicked, [&]() {
        dialog.reject();
    });

    // Show dialog and wait for user input
    if (dialog.exec() == QDialog::Accepted && accepted) {
        // Get selected date and time
        QDate selectedDate = calendar->selectedDate();
        QTime selectedTime = timeEdit->time();

        // Combine date and time into a QString
        QString dateTime = selectedDate.toString("yyyy-MM-dd") + " " + selectedTime.toString("HH:mm");

        // Insert into test_drives table
        QSqlQuery query;
        QString queryStr = QString("INSERT INTO test_drives (client_id, car_id, date_time) VALUES (%1, %2, '%3');")
                               .arg(user_->GetId())
                               .arg(current_product_.id_)
                               .arg(dateTime);

        if (query.exec(queryStr)) {
            QMessageBox::information(this, "Успех", QString("Вы записаны на тест-драйв автомобиля %1 на %2.").arg(current_product_.name_, dateTime));
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось записаться на тест-драйв: " + query.lastError().text());
        }
    }
}

// void MainWindow::on_pushButton_test_drive_clicked()
// {
//     if (current_product_.name_.isEmpty()) {
//         QMessageBox::warning(this, "Ошибка", "Не выбран автомобиль для записи на тест-драйв.");
//         return;
//     }

//     // Запрашиваем у пользователя дату и время для тест-драйва
//     bool ok;
//     QString dateTime = QInputDialog::getText(
//         this,
//         "Запись на тест-драйв",
//         "Введите дату и время (например, 2025-05-15 14:00):",
//         QLineEdit::Normal,
//         "",
//         &ok
//         );

//     if (ok && !dateTime.isEmpty()) {
//         // Здесь можно добавить запись в базу данных (например, в таблицу test_drives)
//         QSqlQuery query;
//         QString queryStr = QString("INSERT INTO test_drives (client_id, car_id, date_time) VALUES (%1, %2, '%3');")
//                                .arg(user_->GetId())
//                                .arg(current_product_.id_)
//                                .arg(dateTime);

//         if (query.exec(queryStr)) {
//             QMessageBox::information(this, "Успех", QString("Вы записаны на тест-драйв автомобиля %1 на %2.").arg(current_product_.name_, dateTime));
//         } else {
//             QMessageBox::critical(this, "Ошибка", "Не удалось записаться на тест-драйв: " + query.lastError().text());
//         }
//     } else if (ok) {
//         QMessageBox::warning(this, "Ошибка", "Не указана дата и время тест-драйва.");
//     }
// }

#include <QSpinBox>

void MainWindow::on_pushButton_to_pay_clicked()
{
    if (!user_) {
        QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для подачи заявки.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Заявка на продажу автомобиля");
    dialog.setFixedSize(400, 300);
    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    QLineEdit* makeEdit = new QLineEdit(&dialog);
    makeEdit->setPlaceholderText("Марка автомобиля");
    QLineEdit* modelEdit = new QLineEdit(&dialog);
    modelEdit->setPlaceholderText("Модель автомобиля");
    QSpinBox* yearEdit = new QSpinBox(&dialog);
    yearEdit->setRange(1900, QDate::currentDate().year());
    QDoubleSpinBox* priceEdit = new QDoubleSpinBox(&dialog);
    priceEdit->setRange(0, 100000000);
    priceEdit->setSuffix(" руб.");

    layout->addWidget(makeEdit);
    layout->addWidget(modelEdit);
    layout->addWidget(yearEdit);
    layout->addWidget(priceEdit);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton("OK", &dialog);
    QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    bool accepted = false;
    connect(okButton, &QPushButton::clicked, [&]() {
        if (makeEdit->text().isEmpty() || modelEdit->text().isEmpty()) {
            QMessageBox::warning(&dialog, "Ошибка", "Укажите марку и модель.");
            return;
        }
        accepted = true;
        dialog.accept();
    });
    connect(cancelButton, &QPushButton::clicked, [&]() { dialog.reject(); });

    if (dialog.exec() == QDialog::Accepted && accepted) {
        QSqlQuery query;
        QString queryStr = QString(
                               "INSERT INTO sell_requests (client_id, car_make, car_model, car_year, proposed_price, status) "
                               "VALUES (%1, '%2', '%3', %4, %5, 'не обработано');")
                               .arg(user_->GetId())
                               .arg(makeEdit->text())
                               .arg(modelEdit->text())
                               .arg(yearEdit->value())
                               .arg(priceEdit->value() * 1000); // Convert to numeric(15,0)

        if (query.exec(queryStr)) {
            QMessageBox::information(this, "Успех", "Заявка на продажу подана.");
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось подать заявку: " + query.lastError().text());
        }
    }
}

void MainWindow::on_pushButton_notifications_clicked()
{
    bool isVisible = ui->notifications_panel->isVisible();
    ui->notifications_panel->setVisible(!isVisible);
    
    if (!isVisible) {
        // Проверяем наличие новых уведомлений
        CheckNotifications();
        
        // Обновляем уведомления
        UpdateNotifications();
        
        // Поднимаем панель поверх других виджетов
        ui->notifications_panel->raise();
        
        // Устанавливаем фокус на панель, чтобы она оставалась поверх
        ui->notifications_panel->setFocus();
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

void MainWindow::CheckNotifications()
{
    if (!user_ || user_->GetRole() != Role::User) {
        return;
    }

    // Получаем количество необработанных уведомлений
    QVariant result = db_manager_->ExecuteSelectQuery(
        QString("SELECT COUNT(*) as count FROM ("
                "    SELECT id FROM service_requests "
                "    WHERE client_id = %1 AND status != 'не обработано' AND (notification_shown = false OR notification_shown IS NULL) "
                "    UNION ALL "
                "    SELECT id FROM insurance_requests "
                "    WHERE client_id = %1 AND status != 'не обработано' AND (notification_shown = false OR notification_shown IS NULL) "
                "    UNION ALL "
                "    SELECT id FROM loan_requests "
                "    WHERE client_id = %1 AND status != 'не обработано' AND (notification_shown = false OR notification_shown IS NULL) "
                "    UNION ALL "
                ") as notifications")
            .arg(user_->GetId()));

    if (result.canConvert<QSqlQuery>()) {
        QSqlQuery query = result.value<QSqlQuery>();
        if (query.next()) {
            int count = query.value("count").toInt();
            if (count > 0) {
                // Есть новые уведомления - меняем иконку
                ui->pushButton_notifications->setStyleSheet(
                    "QPushButton {"
                    "   background-color: #fafafa;"
                    "   border: 2px solid #ff4081;"  // Розовая рамка для индикации
                    "   border-radius: 17px;"
                    "}"
                );
                
                // Показываем всплывающее уведомление
                QMessageBox::information(this, "Новые уведомления", 
                    QString("У вас есть %1 новых уведомлений о статусе заявок").arg(count));
            }
        }
    }
}

void MainWindow::UpdateNotifications()
{
    if (!user_) {
        return;
    }

    // Очищаем текущие уведомления
    ClearNotifications();

    // Получаем все заявки пользователя с измененным статусом
    QString debugQuery = QString("WITH updated_notifications AS ("
            "    (SELECT id, 'service' as request_type, service_type as details, status, created_at, scheduled_date "
            "    FROM public.service_requests "
            "    WHERE client_id = %1 AND status != 'не обработано')"
            "    UNION ALL "
            "    (SELECT id, 'insurance' as request_type, insurance_type as details, status, created_at, NULL as scheduled_date "
            "    FROM public.insurance_requests "
            "    WHERE client_id = %1 AND status != 'не обработано')"
            "    UNION ALL "
            "    (SELECT id, 'loan' as request_type, CAST(loan_amount AS TEXT) as details, status, created_at, NULL as scheduled_date "
            "    FROM public.loan_requests "
            "    WHERE client_id = %1 AND status != 'не обработано')"
            ") "
            "SELECT * FROM updated_notifications "
            "ORDER BY created_at DESC LIMIT 10")
        .arg(user_->GetId());

    qDebug() << "Executing query:" << debugQuery;

    QVariant result = db_manager_->ExecuteSelectQuery(debugQuery);

    if (!result.canConvert<QSqlQuery>()) {
        qDebug() << "Query failed to convert to QSqlQuery";
        qDebug() << "Result:" << result.toString();
        return;
    }

    QSqlQuery query = result.value<QSqlQuery>();
    if (!query.isActive()) {
        qDebug() << "Query is not active";
        qDebug() << "Last error:" << query.lastError().text();
        return;
    }

    int notificationCount = 0;
    while (query.next()) {
        notificationCount++;
        QString type = query.value("request_type").toString();
        QString details = query.value("details").toString();
        QString status = query.value("status").toString();
        QDateTime createdAt = query.value("created_at").toDateTime();
        int id = query.value("id").toInt();
        
        qDebug() << "Processing notification:" << type << details << status;
        
        QString title;
        QString message;
        bool updateSuccess = false;

        // Формируем сообщение в зависимости от типа заявки
        if (type == "service") {
            title = "Заявка на обслуживание";
            QDateTime scheduledDate = query.value("scheduled_date").toDateTime();
            message = QString("Статус заявки на %1\nЗапланировано на: %2\nСтатус: %3")
                         .arg(details)
                         .arg(scheduledDate.toString("dd.MM.yyyy HH:mm"))
                         .arg(status);
            
            // Обновляем статус уведомления
            updateSuccess = db_manager_->ExecuteQuery(
                QString("UPDATE service_requests SET notification_shown = true WHERE id = %1 AND client_id = %2")
                    .arg(id)
                    .arg(user_->GetId()));
        } 
        else if (type == "insurance") {
            title = "Заявка на страхование";
            message = QString("Статус заявки на %1\nСтатус: %2")
                         .arg(details)
                         .arg(status);
            
            updateSuccess = db_manager_->ExecuteQuery(
                QString("UPDATE insurance_requests SET notification_shown = true WHERE id = %1 AND client_id = %2")
                    .arg(id)
                    .arg(user_->GetId()));
        } 
        else if (type == "loan") {
            title = "Заявка на кредит";
            message = QString("Статус заявки на сумму %1 руб.\nСтатус: %2")
                         .arg(FormatPrice(details.toLongLong()))
                         .arg(status);
            
            updateSuccess = db_manager_->ExecuteQuery(
                QString("UPDATE loan_requests SET notification_shown = true WHERE id = %1 AND client_id = %2")
                    .arg(id)
                    .arg(user_->GetId()));
        } 
        else if (type == "sell") {
            title = "Заявка на продажу";
            message = QString("Статус заявки на продажу %1\nСтатус: %2")
                         .arg(details)
                         .arg(status);
            
            updateSuccess = db_manager_->ExecuteQuery(
                QString("UPDATE sell_requests SET notification_shown = true WHERE id = %1 AND client_id = %2")
                    .arg(id)
                    .arg(user_->GetId()));
        }

        if (!updateSuccess) {
            qDebug() << "Failed to update notification_shown status for" << type << "request with id" << id;
            qDebug() << "Error:" << db_manager_->GetLastError();
        }

        message += QString("\nДата создания: %1").arg(createdAt.toString("dd.MM.yyyy HH:mm"));
        AddNotification(title, message);
    }

    qDebug() << "Total notifications found:" << notificationCount;

    // Возвращаем обычный стиль кнопки уведомлений
    ui->pushButton_notifications->setStyleSheet(
        "QPushButton {"
        "   background-color: #fafafa;"
        "   border: 0px;"
        "   border-radius: 17px;"
        "}"
    );
}

void MainWindow::AddNotification(const QString& title, const QString& message)
{
    qDebug() << "Adding notification:" << title << message;

    // Создаем виджет для уведомления
    QWidget* notification = new QWidget();
    notification->setStyleSheet(
        "QWidget { "
        "   background-color: white;"
        "   border-radius: 10px;"
        "   border: 1px solid #e0e0e0;"
        "   margin: 5px;"
        "   padding: 10px;"
        "}"
    );
    
    QVBoxLayout* layout = new QVBoxLayout(notification);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(5);

    // Заголовок уведомления
    QLabel* titleLabel = new QLabel(title);
    titleLabel->setStyleSheet(
        "QLabel { "
        "   color: #1d1b20;"
        "   font: 700 12pt 'JetBrains Mono';"
        "   background: transparent;"
        "}"
    );
    titleLabel->setWordWrap(true);
    layout->addWidget(titleLabel);

    // Текст уведомления
    QLabel* messageLabel = new QLabel(message);
    messageLabel->setStyleSheet(
        "QLabel { "
        "   color: #1d1b20;"
        "   font: 11pt 'Open Sans';"
        "   background: transparent;"
        "}"
    );
    messageLabel->setWordWrap(true);
    layout->addWidget(messageLabel);

    // Проверяем, что у нас есть контейнер для уведомлений
    QVBoxLayout* notificationsLayout = qobject_cast<QVBoxLayout*>(
        ui->scrollAreaWidgetContents_notifications->layout());
    
    if (!notificationsLayout) {
        qDebug() << "Creating new notifications layout";
        notificationsLayout = new QVBoxLayout(ui->scrollAreaWidgetContents_notifications);
        notificationsLayout->setAlignment(Qt::AlignTop);
        notificationsLayout->setSpacing(10);
        notificationsLayout->setContentsMargins(10, 10, 10, 10);
    }

    qDebug() << "Adding notification widget to layout";
    notificationsLayout->addWidget(notification);
    
    // Устанавливаем минимальную ширину для контейнера уведомлений
    ui->scrollAreaWidgetContents_notifications->setMinimumWidth(
        ui->scrollArea_notifications->width() - 30); // 30 пикселей для скроллбара
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
        settings_password_->setText(query.value("password").toString());
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
        // Проверяем, что все поля заполнены
        if (settings_first_name_->text().isEmpty() || settings_last_name_->text().isEmpty() ||
            settings_email_->text().isEmpty() || settings_phone_->text().isEmpty() ||
            settings_password_->text().isEmpty()) {
            QMessageBox::warning(dialogPtr, "Ошибка", "Все поля должны быть заполнены.");
            return;
        }

        // Сохраняем значения полей во временные переменные
        QString firstName = settings_first_name_->text();
        QString lastName = settings_last_name_->text();
        QString email = settings_email_->text();
        QString phone = settings_phone_->text();
        QString password = settings_password_->text();

        // Обновляем данные в базе
        QString updateQuery = QString(
            "UPDATE clients SET "
            "first_name = '%1', "
            "last_name = '%2', "
            "email = '%3', "
            "phone = '%4', "
            "password = '%5' "
            "WHERE id = %6")
            .arg(firstName)
            .arg(lastName)
            .arg(email)
            .arg(phone)
            .arg(password)
            .arg(user_->GetId());

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

