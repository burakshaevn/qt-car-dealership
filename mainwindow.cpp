#include "mainwindow.h"
#include "./ui_mainwindow.h"

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

#include <QDoubleSpinBox>
#include <QTimeEdit>

void MainWindow::SetupServicesScrollArea() {
    // Настройка QScrollArea
    ui->scrollArea_services->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea_services->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea_services->setWidgetResizable(false);
    ui->scrollArea_services->setFocusPolicy(Qt::WheelFocus);

    // Стили для QScrollArea
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
        "    height: 0px;"
        "}"
        );

    // Создаём контейнер для карточек
    QWidget* container = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(container);
    layout->setAlignment(Qt::AlignLeft); // Set alignment correctly
    layout->setSpacing(21);
    layout->setContentsMargins(0, 0, 0, 0);

    // Список названий карточек
    QStringList services = { "Обслуживание", "Аренда", "Продажа", "Кредитование", "Страхование" };

    // Создаём карточки
    for (const QString& service : services) {
        QPushButton* card = new QPushButton(container);
        card->setFixedSize(360, 175);
        card->setStyleSheet(
            "QPushButton {"
            "    background-color: #fafafa;"
            "    border-radius: 39px;"
            "    border: none;"
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
        connect(card, &QPushButton::clicked, this, [=]() {
            if (!user_) {
                QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для подачи заявки.");
                return;
            }

            if (service == "Обслуживание") {
                QDialog dialog(this);
                dialog.setWindowTitle("Заявка на сервисное обслуживание");
                dialog.setFixedSize(400, 400);
                QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);

                // Populate QComboBox with unique car names
                QComboBox* carCombo = new QComboBox(&dialog);
                QSqlQuery carQuery;
                // Select the minimum id for each unique car name
                QString queryStr = "SELECT MIN(id) AS id, name FROM cars GROUP BY name ORDER BY name";
                if (!carQuery.exec(queryStr)) {
                    QMessageBox::critical(this, "Ошибка", "Не удалось загрузить список автомобилей: " + carQuery.lastError().text());
                    return;
                }
                while (carQuery.next()) {
                    carCombo->addItem(carQuery.value("name").toString(), carQuery.value("id").toInt());
                }
                if (carCombo->count() == 0) {
                    QMessageBox::warning(this, "Ошибка", "Нет доступных автомобилей для обслуживания.");
                    return;
                }

                QLineEdit* serviceTypeEdit = new QLineEdit(&dialog);
                serviceTypeEdit->setPlaceholderText("Тип обслуживания (напр., замена масла)");

                QCalendarWidget* calendar = new QCalendarWidget(&dialog);
                calendar->setMinimumDate(QDate::currentDate());

                QTimeEdit* timeEdit = new QTimeEdit(&dialog);
                timeEdit->setDisplayFormat("HH:mm");
                timeEdit->setTime(QTime(9, 0));
                timeEdit->setMinimumTime(QTime(9, 0));
                timeEdit->setMaximumTime(QTime(18, 0));

                dialogLayout->addWidget(carCombo);
                dialogLayout->addWidget(serviceTypeEdit);
                dialogLayout->addWidget(timeEdit);
                dialogLayout->addWidget(calendar);

                QHBoxLayout* buttonLayout = new QHBoxLayout();
                QPushButton* okButton = new QPushButton("OK", &dialog);
                QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
                buttonLayout->addWidget(okButton);
                buttonLayout->addWidget(cancelButton);
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
                QMessageBox::information(this, "Информация", "Для оформления заявки на аренду необходимо выбрать автомобиль.");
            }
            else if (service == "Продажа")
            {
                QDialog dialog(this);
                dialog.setWindowTitle("Заявка на продажу автомобиля");
                dialog.setFixedSize(400, 300);
                QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);

                QLineEdit* makeEdit = new QLineEdit(&dialog);
                makeEdit->setPlaceholderText("Марка автомобиля");
                QLineEdit* modelEdit = new QLineEdit(&dialog);
                modelEdit->setPlaceholderText("Модель автомобиля");
                QSpinBox* yearEdit = new QSpinBox(&dialog);
                yearEdit->setRange(1900, QDate::currentDate().year());
                QDoubleSpinBox* priceEdit = new QDoubleSpinBox(&dialog);
                priceEdit->setRange(0, 100000000);
                priceEdit->setSuffix(" руб.");

                dialogLayout->addWidget(makeEdit);
                dialogLayout->addWidget(modelEdit);
                dialogLayout->addWidget(yearEdit);
                dialogLayout->addWidget(priceEdit);

                QHBoxLayout* buttonLayout = new QHBoxLayout();
                QPushButton* okButton = new QPushButton("OK", &dialog);
                QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
                buttonLayout->addWidget(okButton);
                buttonLayout->addWidget(cancelButton);
                dialogLayout->addLayout(buttonLayout);

                bool accepted = false;
                connect(okButton, &QPushButton::clicked, [&]() {
                    if (makeEdit->text().isEmpty() || modelEdit->text().isEmpty())
                    {
                        QMessageBox::warning(&dialog, "Ошибка", "Укажите марку и модель.");
                        return;
                    }
                    accepted = true;
                    dialog.accept();
                });
                connect(cancelButton, &QPushButton::clicked, [&]() { dialog.reject(); });

                if (dialog.exec() == QDialog::Accepted && accepted)
                {
                    QSqlQuery query;
                    QString queryStr = QString(
                                           "INSERT INTO sell_requests (client_id, car_make, car_model, car_year, proposed_price, status) "
                                           "VALUES (%1, '%2', '%3', %4, %5, 'не обработано');")
                                           .arg(user_->GetId())
                                           .arg(makeEdit->text())
                                           .arg(modelEdit->text())
                                           .arg(yearEdit->value())
                                           .arg(priceEdit->value() * 1000);

                    if (query.exec(queryStr)) {
                        QMessageBox::information(this, "Успех", "Заявка на продажу подана.");
                    } else {
                        QMessageBox::critical(this, "Ошибка", "Не удалось подать заявку: " + query.lastError().text());
                    }
                }
            }
            else if (service == "Кредитование")
            {
                QDialog dialog(this);
                dialog.setWindowTitle("Заявка на кредитование");
                dialog.setFixedSize(400, 300);
                QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);

                QComboBox* carCombo = new QComboBox(&dialog);
                QSqlQuery carQuery("SELECT MIN(id) AS id, name FROM cars GROUP BY name ORDER BY name");
                while (carQuery.next())
                {
                    carCombo->addItem(carQuery.value("name").toString(), carQuery.value("id").toInt());
                }
                QDoubleSpinBox* amountEdit = new QDoubleSpinBox(&dialog);
                amountEdit->setRange(100000, 50000000);
                amountEdit->setSuffix(" руб.");
                QSpinBox* termEdit = new QSpinBox(&dialog);
                termEdit->setRange(1, 360);
                termEdit->setSuffix(" мес.");

                dialogLayout->addWidget(carCombo);
                dialogLayout->addWidget(amountEdit);
                dialogLayout->addWidget(termEdit);

                QHBoxLayout* buttonLayout = new QHBoxLayout();
                QPushButton* okButton = new QPushButton("OK", &dialog);
                QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
                buttonLayout->addWidget(okButton);
                buttonLayout->addWidget(cancelButton);
                dialogLayout->addLayout(buttonLayout);

                bool accepted = false;
                connect(okButton, &QPushButton::clicked, [&]() {
                    accepted = true;
                    dialog.accept();
                });
                connect(cancelButton, &QPushButton::clicked, [&]() { dialog.reject(); });

                if (dialog.exec() == QDialog::Accepted && accepted)
                {
                    QSqlQuery query;
                    QString queryStr = QString(
                                           "INSERT INTO loan_requests (client_id, car_id, loan_amount, loan_term_months, status) "
                                           "VALUES (%1, %2, %3, %4, 'не обработано');")
                                           .arg(user_->GetId())
                                           .arg(carCombo->currentData().toInt())
                                           .arg(amountEdit->value() * 1000)
                                           .arg(termEdit->value());

                    if (query.exec(queryStr))
                    {
                        QMessageBox::information(this, "Успех", "Заявка на кредитование подана.");
                    } else
                    {
                        QMessageBox::critical(this, "Ошибка", "Не удалось подать заявку: " + query.lastError().text());
                    }
                }
            }
            else if (service == "Страхование")
            {
                QDialog dialog(this);
                dialog.setWindowTitle("Заявка на страхование");
                dialog.setFixedSize(400, 300);
                QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);

                QComboBox* carCombo = new QComboBox(&dialog);
                QSqlQuery carQuery("SELECT MIN(id) AS id, name FROM cars GROUP BY name ORDER BY name");
                while (carQuery.next())
                {
                    carCombo->addItem(carQuery.value("name").toString(), carQuery.value("id").toInt());
                }
                QComboBox* insuranceTypeCombo = new QComboBox(&dialog);
                insuranceTypeCombo->addItems({"ОСАГО", "КАСКО", "Комплекс"});

                dialogLayout->addWidget(carCombo);
                dialogLayout->addWidget(insuranceTypeCombo);

                QHBoxLayout* buttonLayout = new QHBoxLayout();
                QPushButton* okButton = new QPushButton("OK", &dialog);
                QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
                buttonLayout->addWidget(okButton);
                buttonLayout->addWidget(cancelButton);
                dialogLayout->addLayout(buttonLayout);

                bool accepted = false;
                connect(okButton, &QPushButton::clicked, [&]() {
                    accepted = true;
                    dialog.accept();
                });
                connect(cancelButton, &QPushButton::clicked, [&]() { dialog.reject(); });

                if (dialog.exec() == QDialog::Accepted && accepted)
                {
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

    // Устанавливаем минимальный размер контейнера
    container->setMinimumWidth((360 + 21) * services.size());
    container->setFixedHeight(175);

    // Устанавливаем контейнер в QScrollArea
    ui->scrollArea_services->setWidget(container);

    // Убедимся, что QScrollArea может принимать фокус
    ui->scrollArea_services->setFocus();
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
    if (user_->GetRole() == Role::User) {
        ui->stackedWidget->setCurrentWidget(ui->main);
        current_product_ = ProductInfo();
        current_color_index_ = 0;
    }
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

