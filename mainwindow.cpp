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

void MainWindow::UpdateUser(const UserInfo& user, QWidget* parent){
    user_ = std::make_unique<User>(user, parent);
}

void MainWindow::on_pushButton_login_clicked() {
    auto queryResult = db_manager_->ExecuteSelectQuery(QString("SELECT * FROM public.admins WHERE username = '%1';").arg(ui->lineEdit_login->text()));

    if (queryResult.canConvert<QSqlQuery>()) {
        QSqlQuery query = queryResult.value<QSqlQuery>();
        if (query.next()) {
            UserInfo user;
            user.id_ = query.value("id").toInt();
            user.password_ = query.value("password").toString();
            user.role_ = Role::Admin;
            if (user.password_ == ui->lineEdit_password->text()) {
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
            else{
                QMessageBox::critical(this, "Авторизация", "Неверный логин или пароль.");
            }
        }
        else {
            auto queryResult = db_manager_->ExecuteSelectQuery(QString("SELECT * FROM public.clients WHERE email = '%1';").arg(ui->lineEdit_login->text()));

            if (queryResult.canConvert<QSqlQuery>()) {
                QSqlQuery query = queryResult.value<QSqlQuery>();
                if (query.next()) {
                    UserInfo user;
                    user.id_ = query.value("id").toInt();
                    user.full_name_ = query.value("first_name").toString();
                    user.full_name_ += " " + query.value("last_name").toString();
                    user.email_ = query.value("email").toString();
                    user.password_ = query.value("password").toString();
                    user.role_ = Role::User;
                    user.products_ = GetPurchasedProducts(user.id_);

                    if (user.password_ == ui->lineEdit_password->text()) {
                        ui->lineEdit_login->clear();
                        ui->lineEdit_password->clear();
                        QMessageBox::information(this, "Авторизация", "Выполнена авторизация как пользователь.");

                        BuildDependencies();
                        UpdateUser(user, this);

                        products_->PullProducts();

                        product_card_->UpdateProductsWidget(ui->scrollArea, "Смотреть всё", "Белый");
                        ui->stackedWidget->setCurrentWidget(ui->main);
                    }
                    else{
                        QMessageBox::critical(this, "Авторизация", "Неверный логин или пароль.");
                    }
                }
                else {
                    QMessageBox::critical(this, "Ошибка", "Пользователя с таким логином не существует.");
                }
            }
            else {
                QMessageBox::critical(this, "Ошибка в базе данных", queryResult.toString());
            }
        }
    }
    else {
        QMessageBox::critical(this, "Ошибка в базе данных", queryResult.toString());
    }
}

void MainWindow::on_pushButton_logout_clicked() {
    if (!this->ui->stackedWidget) {
        return;
    }

    if (user_  && user_->GetRole() == Role::User) {
        // side_widget_.reset();
        // floating_menu_.reset();
        floating_widgets_.reset();

        products_.reset();
        cart_.reset();
        product_card_.reset();
        db_manager_.reset();


        if (ui->scrollArea) {
            QWidget* oldWidget = ui->scrollArea->takeWidget();
            if (oldWidget) {
                oldWidget->deleteLater();
            }
        }
        if (ui->scrollArea_2) {
            QWidget* oldWidget = ui->scrollArea_2->takeWidget();
            if (oldWidget) {
                oldWidget->deleteLater();
            }
        }
    }
    user_.reset();
    table_.reset();

    // Переключение на экран логина
    if (this->ui->stackedWidget && this->ui->login) {
        this->ui->stackedWidget->setCurrentWidget(this->ui->login);
    }
}

void MainWindow::SetupFloatingMenu() {
    floating_widgets_->BuildFloatingMenu(
        // Обработка кнопки "More"
        [this]() { this->MoreClicked(); },

        // Обработка кнопки "Search"
        [this]() {
            bool ok;
            QString term = QInputDialog::getText(
                this,
                "Поиск",
                "Укажите поисковый запрос:",
                QLineEdit::Normal,
                "",
                &ok
                );

            if (ok && !term.isEmpty()) {
                ProductInfo product;
                product.name_ = term;

                int relevant_results = product_card_->DrawRelevantProducts(ui->scrollArea, product.name_);
                if (relevant_results > 0) {
                    QMessageBox::information(this, "Поиск", "Найдено " + QString::number(relevant_results) + " результатов по запросу «" + term + "».");
                    MoreClicked();
                } else {
                    QMessageBox::warning(this, "Поиск", "Отсутствуют релевантные результаты.");
                }
            } else {
                QMessageBox::warning(
                    this,
                    "Предупреждение",
                    "Запрос не может быть пустым."
                    );
            }
        },

        // Обработка кнопки "Сортировка по цветам"
        [this]() { this->SortByColorClicked(); },

        // Обработка кнопки "User Profile"
        [this]() { this->ProfileClicked(); }
        );
}

QList<Products::ProductKey> MainWindow::GetPurchasedProducts(int user_id) const {
    QList<Products::ProductKey> products_; // Список названий купленных инструментов

    QSqlQuery query;
    QString query_str = QString("select * from cars where id in (select car_id from purchases where client_id = %1);").arg(user_id);
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
            products_->PullAvailableColorsForProduct(product);
            ShowProductOnPersonalPage(product);
        });

        // connect(products_.get(), &Products::CartUpdated, this, [this]{
        //     ui->label_cart_total->setText("Корзина — " + FormatPrice(cart_->GetTotalCost()) + " руб.");
        // });

        // Устанавливаем связь между ProductCard и Instruments
        product_card_->SetProductsPtr(products_);
    }
}

void MainWindow::MoreClicked() {
    if (user_.get()) {
        if (!product_card_->hidden_to_cart_buttons_IsEmpty()) {
            product_card_->RestoreHiddenToCartButtons();
        }
        if (user_->GetRole() == Role::User) {
            // Создаем список типов для выпадающего списка
            QStringList types;
            QSqlQuery query("SELECT name FROM car_types");
            while (query.next()) {
                types << query.value("name").toString();
            }
            types << "Смотреть всё"; // Добавляем пункт "Смотреть всё"

            // Создаем модальное окно с выпадающим списком
            bool ok;
            QString selectedType = QInputDialog::getItem(
                this, // Родительский виджет (this, чтобы окно было модальным относительно MainWindow)
                "Выберите тип", // Заголовок окна
                "Тип:", // Текст перед выпадающим списком
                types, // Список элементов
                0, // Индекс выбранного элемента по умолчанию
                false, // Редактируемый ли список
                &ok // Успешно ли выбрано значение
                );
            qDebug() << selectedType;
            // Обработка выбора
            if (ok && !selectedType.isEmpty()) {
                if (selectedType == "Смотреть всё") {
                    product_card_->UpdateProductsWidget(ui->scrollArea, "Смотреть всё", "Белый");
                    ui->stackedWidget->setCurrentWidget(ui->main);
                    qDebug() << "Выбрано: Смотреть всё";
                } else {
                    product_card_->UpdateProductsWidget(ui->scrollArea, selectedType, "Белый");
                    ui->stackedWidget->setCurrentWidget(ui->main);
                    qDebug() << "Выбран тип:" << selectedType;
                }
            }

            return;
        }
    }
    QMessageBox::warning(this, "Ошибка", "Чтобы переключаться по остальным разделам, необходимо авторизоваться как пользователь.");
}

void MainWindow::ProfileClicked() {
    if (user_->GetRole() == Role::User) {
        floating_widgets_->GetSideMenu()->setVisible(false);

        product_card_->HideOldCards();
        product_card_->EnsureContainerInScrollArea(ui->scrollArea_2);

        const QList<Products::ProductKey>& purchases_products_ = GetPurchasedProducts(user_->GetId());
        for (auto& key : purchases_products_) {
            qDebug() << purchases_products_.size();
            const ProductInfo* found_product_struct = products_->FindProduct(key);
            if (!found_product_struct){
                qDebug() << "found_product_struct is empty";
                break;
            }

            product_card_->DrawItem(*found_product_struct);
            auto to_cart_button = product_card_->FindProductCard(key)->findChild<QPushButton*>("to_cart_", Qt::FindChildrenRecursively);
            if (to_cart_button) {
                to_cart_button->hide();
                product_card_->hidden_to_cart_buttons_Push(key);
                qDebug() << "Добавлена карточка: " << found_product_struct->name_;
            }
        }
        product_card_->card_container_PerformAdjustSize();

        ui->label_clientname->setText(user_->GetName() + " — профиль");
        ui->stackedWidget->setCurrentWidget(ui->user_page);
    }
}

void MainWindow::SortByColorClicked(){
    if (user_.get()) {
        if (!product_card_->hidden_to_cart_buttons_IsEmpty()) {
            product_card_->RestoreHiddenToCartButtons();
        }
        if (user_->GetRole() == Role::User) {
            // Создаем модальное окно с выпадающим списком
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
            qDebug() << selectedColor;
            // Обработка выбора
            if (ok && !selectedColor.isEmpty()) {
                product_card_->UpdateProductsWidget(ui->scrollArea, "Смотреть всё", selectedColor);
                ui->stackedWidget->setCurrentWidget(ui->main);
                qDebug() << "Выбран цвет:" << selectedColor;
            }
            return;
        }
    }
    QMessageBox::warning(this, "Ошибка", "Чтобы переключаться по остальным разделам, необходимо авторизоваться как пользователь.");
}

void MainWindow::CleanCart() {
    for (auto& [name_, card_] : cart_->GetCart()) {
        if (card_) {
            auto to_cart_button = card_->findChild<QPushButton*>("to_cart_", Qt::FindChildrenRecursively);
            if (to_cart_button) {
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

    for (auto& [name_, card_] : cart_->GetCart()) {
        auto instrument = products_->FindProduct(name_);
        if (!db_manager_->ExecuteQuery(QString("INSERT INTO public.purchases(id, client_id, instrument_id) VALUES (%1, %2, %3);")
                                          .arg(new_id).arg(user_->GetId()).arg(instrument->id_)))
        {
            success = false;
            QMessageBox::critical(this, "Ошибка", "Не удалось выполнить операцию: " + db_manager_->GetLastError());
            break; // Прерываем цикл при ошибке
        }
        ++new_id;
    }

    // Если все операции успешны, фиксируем транзакцию
    if (success) {
        if (!db.commit()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось зафиксировать транзакцию: " + db.lastError().text());
        }
        else {
            // All succsess
        }
    }
    else {
        // Если есть ошибки, откатываем транзакцию
        if (!db.rollback()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось откатить транзакцию: " + db.lastError().text());
        }
    }
}

void MainWindow::on_pushButton_clean_cart_clicked()
{
    if (cart_->CartIsEmpty()){
        QMessageBox::warning(this, "Предупреждение", "Невозможно очистить корзину: отсутствует содержимое.");
        return;
    }
    CleanCart();
    QMessageBox::information(this, "", "Корзина очищена.");
}

void MainWindow::on_pushButton_submit_cart_clicked()
{
    if (cart_->CartIsEmpty()){
        QMessageBox::warning(this, "Предупреждение", "Невозможно выполить оплату: отсутствует содержимое.");
        return;
    }
    ToPayCart();
    CleanCart();
    QMessageBox::information(this, "", "Произведена оплата. Купленные инструменты добавлены в профиль.");
}


void MainWindow::ShowProductOnPersonalPage(const ProductInfo& product) {
    ui->label_name->setText(product.name_);
    ui->label_price->setText(FormatPrice(product.price_) + " руб.");
    auto current_product_colors_ = products_->PullAvailableColorsForProduct(current_product_); // Доступные варианты цветов для current_product
    // ui->label_color_index->setText(QString::number(current_color_index_ + 1) + "/" + QString::number(current_product_colors_.size()));

    QString imagePath = QDir::cleanPath(product.image_path_);
    QPixmap originalPixmap(imagePath);

    if (!originalPixmap.isNull()) {
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

void MainWindow::on_pushButton_next_left_clicked() {
    // Доступные варианты цветов для current_product
    auto current_product_colors_ = products_->PullAvailableColorsForProduct(current_product_);
    if (!current_product_colors_.isEmpty()) {
        current_color_index_ = (current_color_index_ - 1 + current_product_colors_.size()) % current_product_colors_.size();
        current_product_.id_ = current_product_colors_.at(current_color_index_).id_;
        current_product_.color_ = current_product_colors_.at(current_color_index_).color_;
        current_product_.image_path_ = current_product_colors_.at(current_color_index_).image_path_;
        ShowProductOnPersonalPage(current_product_colors_.at(current_color_index_));
    }
}

void MainWindow::on_pushButton_next_right_clicked() {
    // Доступные варианты цветов для current_product
    auto current_product_colors_ = products_->PullAvailableColorsForProduct(current_product_);
    if (!current_product_colors_.isEmpty()) {
        current_color_index_ = (current_color_index_ + 1) % current_product_colors_.size();
        current_product_.id_ = current_product_colors_.at(current_color_index_).id_;
        current_product_.color_ = current_product_colors_.at(current_color_index_).color_;
        current_product_.image_path_ = current_product_colors_.at(current_color_index_).image_path_;
        ShowProductOnPersonalPage(current_product_colors_.at(current_color_index_));
    }
}

void MainWindow::on_pushButton_back_clicked()
{
    if (user_->GetRole() == Role::User) {
        ui->stackedWidget->setCurrentWidget(ui->main);
    }
}

