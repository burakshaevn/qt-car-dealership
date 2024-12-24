#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(1100, 560);

    // Настраиваем автоматическое подключение к базе данных
    // при запуске приложения и открываем базу данных
    QString hostname = "localhost";
    int port = 5432;
    QString dbname = "car-dealership";
    QString username = "postgres";
    QString password = "password";
    db_manager_.UpdateConnection(hostname, port, dbname, username, password);
    db_manager_.Open();

    // После запуска переносим пользователя на окно входа
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
    auto queryResult = db_manager_.ExecuteSelectQuery(QString("SELECT * FROM public.admins WHERE username = '%1';").arg(ui->lineEdit_login->text()));

    if (queryResult.canConvert<QSqlQuery>()) {
            QSqlQuery query = queryResult.value<QSqlQuery>();
            if (query.next()) {
                UserInfo user;
                user.id_ = query.value("id").toInt();
                user.full_name_ = query.value("full_name").toString();
                user.email_ = query.value("email").toString();
                user.password_ = query.value("password").toString();
                user.role_ = StringToRole(query.value("role").toString());
                if (user.password_ == ui->lineEdit_password->text()) {
                    ui->lineEdit_login->clear();
                    ui->lineEdit_password->clear();
                    QMessageBox::information(this, "Авторизация", "Выполнена авторизация как администратор.");
                    UpdateUser(user, this);

                    table_ = std::make_unique<Table>(&db_manager_, user_.get(), nullptr);
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
                auto queryResult = db_manager_.ExecuteSelectQuery(QString("SELECT * FROM public.clients WHERE email = '%1';").arg(ui->lineEdit_login->text()));

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
                        user.purchased_cars_ = GetCars(user.id_);

                        if (user.password_ == ui->lineEdit_password->text()) {
                            ui->lineEdit_login->clear();
                            ui->lineEdit_password->clear();
                            QMessageBox::information(this, "Авторизация", "Выполнена авторизация как пользователь.");

                            UpdateUser(user, this);
                            ui->stackedWidget->setCurrentWidget(ui->main);

                            SetupFloatingMenu();
                            SetupSideMenu();
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

    user_.reset();
    table_.reset();
    // table_services_.reset();
    side_widget_.reset();
    floating_menu_.reset();

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

    // Очистка карты с автомобилями
    carCardsMap_.clear();
    carColors_.clear();
    currentColorIndex_ = 0;

    // Переключение на экран логина
    this->ui->stackedWidget->setCurrentWidget(this->ui->login);
}

void MainWindow::UpdateColorDropdown() {
    if (!colorDropdown_) return;

    colorDropdown_->clear(); // Очищаем список

    QList<QString> colors = db_manager_.GetDistinctColors();

    if (colors.isEmpty()) {
        qDebug() << "No colors retrieved from the database.";
        return;
    }

    // Добавляем опцию "Все" для сброса фильтра
    colorDropdown_->addItem("По умолч.");

    for (const auto& color : colors) {
        colorDropdown_->addItem(color);
    }

    // Подключаем сигнал для изменения выбора
    connect(colorDropdown_.get(), &QComboBox::currentTextChanged, this, [this](const QString& selectedColor) {
        if (selectedColor == "По умолч.") {
            DrawCars(ui->scrollArea, "select * from cars where color = 'Белый'");
        } else {
            DrawCars(ui->scrollArea, QString("select * from cars where color = '%1'").arg(selectedColor));
        }
    });
}

void MainWindow::SetupFloatingMenu() {
    floating_menu_ = std::make_unique<QWidget>(this);
    floating_menu_->setStyleSheet("background-color: #fafafa; border-radius: 29px;");
    floating_menu_->setFixedSize(349, 74);

    QHBoxLayout* menuLayout = new QHBoxLayout(floating_menu_.get());
    menuLayout->setContentsMargins(20, 10, 20, 10);
    menuLayout->setSpacing(20);

    QPushButton* carButton = new QPushButton(floating_menu_.get());
    carButton->setIcon(QIcon("://directions_car.svg"));
    carButton->setIconSize(QSize(35, 35));
    carButton->setStyleSheet("QPushButton { border: none; outline: none; }");
    connect(carButton, &QPushButton::clicked, this, &MainWindow::on_pushButton_cars_clicked);

    QPushButton* searchButton = new QPushButton(floating_menu_.get());
    searchButton->setIcon(QIcon("://search.svg"));
    searchButton->setIconSize(QSize(35, 35));
    searchButton->setStyleSheet("QPushButton { border: none; outline: none; }");
    connect(searchButton, &QPushButton::clicked, this, [this]() {
        bool ok;
        Car car;
        car.name_= QInputDialog::getText(
            this,
            "Поиск автомобиля",
            "Введите название автомобиля:",
            QLineEdit::Normal,
            "",
            &ok
            );

        if (ok && !car.name_.isEmpty()) {
            onCarSelected(car);
        }
    });

    QPushButton* sort_by_color = new QPushButton(floating_menu_.get());
    sort_by_color->setIcon(QIcon("://Color Swatch 02.svg"));
    sort_by_color->setIconSize(QSize(32, 32));
    sort_by_color->setStyleSheet("QPushButton { border: none; outline: none; }");

    QPushButton* userButton = new QPushButton(floating_menu_.get());
    userButton->setIcon(QIcon("://person.svg"));
    userButton->setIconSize(QSize(35, 35));
    userButton->setStyleSheet("QPushButton { border: none; outline: none; }");
    connect(userButton, &QPushButton::clicked, this, &MainWindow::on_pushButton_profile_clicked);

    menuLayout->addWidget(carButton);
    menuLayout->addWidget(searchButton);
    menuLayout->addWidget(sort_by_color);
    menuLayout->addWidget(userButton);

    floating_menu_->move((this->width() - floating_menu_->width()) / 2, this->height() - floating_menu_->height() - 20);
    floating_menu_->show();

    // Создаем выпадающий список для цветов
    colorDropdown_ = std::make_unique<QComboBox>(this);
    colorDropdown_->setStyleSheet(R"(
    QComboBox {
        background-color: #fafafa;
        border-radius: 9px;
        border: 1px solid #303436;
        padding: 5px;
    }
    QComboBox::drop-down {
        border: none;
    }
    QComboBox::down-arrow {
        width: 10px;
        height: 10px;
    }
    QComboBox QAbstractItemView {
        border: 1px solid #303436;
        border-radius: 9px;
        background-color: #fafafa;
    }
    )");
    colorDropdown_->hide(); // Скрываем выпадающий список изначально

    connect(sort_by_color, &QPushButton::clicked, this, [this]() {
        if (colorDropdown_->isVisible()) {
            colorDropdown_->hide();
        } else {
            UpdateColorDropdown();
            QPoint buttonPos = floating_menu_->geometry().bottomLeft();
            colorDropdown_->setFixedSize(246, 35);
            colorDropdown_->move(464, 442);
            colorDropdown_->show();
        }
    });

    UpdateColorDropdown();
}

void MainWindow::DrawCars(QScrollArea* scrollArea, const QString& condition) {
    QWidget* oldWidget = scrollArea->takeWidget();
    if (oldWidget) {
        oldWidget->deleteLater(); // Удаляем виджет асинхронно
    }

    // Создаем виджет-контейнер для карточек
    QWidget* container = new QWidget(scrollArea);
    QVBoxLayout* layout = new QVBoxLayout(container);

    // Запрос к базе данных для получения автомобилей
    auto queryResult = db_manager_.ExecuteSelectQuery(condition);

    if (queryResult.canConvert<QSqlQuery>()) {
        QSqlQuery query = queryResult.value<QSqlQuery>();
        while (query.next()) {
            int id = query.value("id").toInt();
            QString name = query.value("name").toString();
            QString color = query.value("color").toString();
            double price = query.value("price").toDouble();
            QString description = query.value("description").toString();

            QString relativePath = "/../../resources/";
            QString imagePath = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/" + relativePath + query.value("image_url").toString().replace("\\", "/"));

            QPixmap pixmap(imagePath);
            if (!pixmap.isNull()) {
                // Создаем карточку
                QWidget* carCard = new QWidget(container);
                carCard->setStyleSheet("background-color: #fafafa; border-radius: 50px;");
                carCard->setFixedSize(640, 152);

                // Загружаем изображение
                QPixmap originalPixmap(imagePath);

                // Проверяем, удалось ли загрузить изображение
                if (!originalPixmap.isNull()) {
                    // Фиксированная ширина для всех изображений
                    int fixedWidth = 200;

                    // Масштабируем изображение с сохранением пропорций
                    QPixmap scaledPixmap = originalPixmap.scaled(fixedWidth, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                    // Получаем итоговую высоту изображения после масштабирования
                    int imageHeight = scaledPixmap.height();

                    // Рассчитываем координату Y для центрирования изображения
                    int imageY = (carCard->height() - imageHeight) / 2;

                    // Создаем QLabel для изображения
                    QLabel* carImage = new QLabel(carCard);
                    carImage->setPixmap(scaledPixmap);
                    carImage->setFixedSize(fixedWidth, imageHeight);
                    carImage->move(36, imageY);
                }

                // Добавляем название автомобиля
                QLabel* carName = new QLabel("<b>" + name + "</b>", carCard);
                carName->setStyleSheet("font: 16pt 'JetBrains Mono'; color: #1d1b20;");
                carName->setAlignment(Qt::AlignLeft);
                carName->setFixedSize(315, 25);
                carName->move(271, 25);

                // Добавляем цвет автомобиля
                QLabel* carColor = new QLabel(color, carCard);
                carColor->setStyleSheet("font: 12pt 'JetBrains Mono'; color: #555555;");
                carColor->setAlignment(Qt::AlignLeft);
                carColor->setFixedSize(294, 19);
                carColor->move(271, 57);

                // Использование функции FormatPrice
                QLabel* carPrice = new QLabel(FormatPrice(price) + " руб.", carCard);
                carPrice->setStyleSheet("font: 700 16pt 'JetBrains Mono'; color: #1d1b20;");
                carPrice->setAlignment(Qt::AlignRight);
                carPrice->setFixedSize(336, 25);
                carPrice->move(271, 102);

                // Добавляем карточку в контейнер
                layout->addWidget(carCard);

                Car selectedCar(id, name, color, price, description, imagePath);
                carCardsMap_[carCard] = selectedCar;
                carCard->installEventFilter(this);
            }
            else {
                qDebug() << "Не удалось загрузить изображение: " << imagePath;
            }
        }
    }
    container->setLayout(layout);
    scrollArea->setWidget(container);
}

QString MainWindow::FormatPrice(int price) {
    QString formattedPrice = QString::number(price);
    int len = formattedPrice.length();
    for (int i = len - 3; i > 0; i -= 3) {
        formattedPrice.insert(i, ' ');
    }
    return formattedPrice;
};

QList<Car> MainWindow::GetCars(int user_id) const {
    QList<Car> cars;
    QSqlQuery query;
    QString query_str = QString("select * from cars where id in (select car_id from purchases where client_id = %1); ").arg(user_id);

    if (!query.exec(query_str)) {
        qWarning() << "Failed to execute query:" << query.lastError().text();
        return cars;
    }

    while (query.next()) {
        cars.append(Car{
            query.value(0).toInt(),       // id
            query.value(1).toString(),   // name
            query.value(2).toString(),   // color
            query.value(3).toInt(),      // price
            query.value(4).toString(),   // description
            query.value(5).toString()    // path_to_image
        });
    }

    return cars;
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::MouseButtonPress) {
        auto it = carCardsMap_.find(static_cast<QWidget*>(obj));
        if (it != carCardsMap_.end()) {
            current_car_ = it.value();
            emit onCarSelected(current_car_);
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::SetupSideMenu() {
    // Создаём боковое меню как QWidget
    side_widget_ = std::make_unique<QWidget>(this);
    side_widget_->setStyleSheet("background-color: #fafafa;");

    side_widget_->setGeometry(0, 0, 224, 560);

    // Добавляем логотип
    QLabel* logo = new QLabel(side_widget_.get());
    logo->setPixmap(QPixmap("://logo.svg").scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logo->setGeometry(19, 38, 18, 18);

    QLabel* logo_words = new QLabel(side_widget_.get());
    logo_words->setPixmap(QPixmap("://mercedez-benz.svg").scaled(160, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logo_words->setGeometry(45, 38, 160, 18);

    // Добавляем заголовок
    QLabel* title = new QLabel("Модели", side_widget_.get());
    title->setStyleSheet("background-color: #fafafa; color: #140f10; font: 700 12pt 'JetBrains Mono';");
    title->setGeometry(30, 125, 234, 25);

    // Создаём QListWidget
    side_list_ = new QListWidget(side_widget_.get());
    side_list_->setStyleSheet("background-color: #fafafa; color: #140f10; font: 12pt 'JetBrains Mono'; border: 0px;");
    side_list_->setGeometry(22, 111, 224, 449);

    // Наполняем список моделей
    QSqlQuery query("SELECT name FROM car_types");
    while (query.next()) {
        QString carTypeName = query.value("name").toString();
        side_list_->addItem(carTypeName);
    }

    // Добавляем кнопку "Смотреть все" как элемент списка
    QListWidgetItem* viewAllItem = new QListWidgetItem("Все модели", side_list_);
    viewAllItem->setTextAlignment(Qt::AlignLeft); // Выравнивание текста по центру
    viewAllItem->setFont(QFont("JetBrains Mono", 12));
    viewAllItem->setForeground(QColor("#9b9c9c"));

    // Обработка кликов по элементам меню
    connect(side_list_, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (item->text() == "Все модели") {
            // Логика для кнопки "Смотреть все"
            DrawCars(ui->scrollArea, "SELECT * FROM cars WHERE color = 'Белый'");
        } else {
            // Логика для остальных пунктов списка
            DrawCars(ui->scrollArea, QString(R"(SELECT * FROM cars WHERE type_id =
                            (SELECT id FROM car_types WHERE name = '%1') AND color = '%2')")
                         .arg(item->text())
                         .arg("Белый"));
        }
        ui->stackedWidget->setCurrentWidget(ui->main);
    });

    // Делаем виджет видимым
    side_widget_->show();
}

void MainWindow::on_pushButton_cars_clicked()
{
    if (user_.get()){
        if (user_->GetRole() == Role::User) {
            side_widget_->setVisible(true);
            side_list_->setVisible(true);
            ui->stackedWidget->setCurrentWidget(ui->main);
            return;
        }
    }
    QMessageBox::warning(this, "Ошибка", "Чтобы переключаться по остальным разделам, необходимо авторизоваться как пользователь.");
}

void MainWindow::on_pushButton_search_clicked() {
    if (ui->stackedWidget->currentWidget() == ui->login ||
        ui->stackedWidget->currentWidget() == table_.get()) {
        QMessageBox::information(this, "Информация", "Текущая страница — главная.");
    } else {
        if (user_->GetRole() == Role::User) {
            QSqlRecord record;
            EditDialog dialog(record, this);

            if (dialog.exec() == QDialog::Accepted) {
                try {
                    QSqlRecord updatedRecord = dialog.GetUpdatedRecord();
                    QString carName = updatedRecord.value("name").toString();

                    if (!carName.isEmpty()) {

                    }
                } catch (const std::exception& e) {
                    QMessageBox::critical(this, "Ошибка", e.what());
                }
            }
        }
    }
}

void MainWindow::on_pushButton_profile_clicked()
{
    if (ui->stackedWidget->currentWidget() == ui->login || ui->stackedWidget->currentWidget() == table_.get()) {
        QMessageBox::information(this, "Информация", "Текущая страница — главная.");
    }
    else{
        if (user_->GetRole() == Role::User){
            side_widget_->setVisible(false);
            side_list_->setVisible(false);
            DrawCars(ui->scrollArea_2, QString(R"(select * from cars
            where id IN (select car_id from purchases where client_id = %1);)").arg(user_->GetId()));
            ui->label_clientname->setText(user_->GetName());
            ui->stackedWidget->setCurrentWidget(ui->user_page);
        }
    }
}

void MainWindow::on_pushButton_back_clicked()
{
    if (user_.get()){
        if (user_->GetRole() == Role::User) {
            side_widget_->setVisible(true);
            side_list_->setVisible(true);
            ui->stackedWidget->setCurrentWidget(ui->main);
            return;
        }
    }
    QMessageBox::warning(this, "Ошибка", "Чтобы переключаться по остальным разделам, необходимо авторизоваться как пользователь.");
}

void MainWindow::onCarSelected(const Car& car) {
    side_widget_->setVisible(false);
    side_list_->setVisible(false);

    // Выполняем запрос в базу данных для получения всех цветов
    carColors_.clear();
    currentColorIndex_ = 0;

    QSqlQuery query;
    query.prepare("SELECT * FROM cars WHERE name = :name");
    query.bindValue(":name", car.name_);
    if (query.exec()) {
        while (query.next()) {
            Car colorVariant;
            colorVariant.id_ = query.value("id").toInt();
            colorVariant.name_ = query.value("name").toString();
            colorVariant.color_ = query.value("color").toString();
            colorVariant.price_ = query.value("price").toInt();
            colorVariant.description_= query.value("description").toString();
            colorVariant.path_to_image_ = query.value("image_url").toString();
            carColors_.append(colorVariant);
        }
    }
    else{
        QMessageBox::warning(this, "Результаты поиска", "Автомобиль не найден.");
    }

    if (!carColors_.isEmpty()) {
        showCarColor(carColors_.at(currentColorIndex_));

    }

    ui->stackedWidget->setCurrentWidget(ui->personal);
}

void MainWindow::showCarColor(const Car& car) {
    ui->label_name->setText(car.name_);
    ui->label_price->setText(FormatPrice(car.price_) + " руб.");
    ui->label_color_index->setText(QString::number(currentColorIndex_ + 1) + "/" + QString::number(carColors_.size()));

    QString imagePath = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../resources/" + car.path_to_image_);
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
}

void MainWindow::on_pushButton_next_left_clicked() {
    if (!carColors_.isEmpty()) {
        currentColorIndex_ = (currentColorIndex_ - 1 + carColors_.size()) % carColors_.size();
        current_car_.id_ = carColors_.at(currentColorIndex_).id_;
        current_car_.color_ = carColors_.at(currentColorIndex_).color_;
        current_car_.path_to_image_ = carColors_.at(currentColorIndex_).path_to_image_;
        showCarColor(carColors_.at(currentColorIndex_));
    }
}

void MainWindow::on_pushButton_next_right_clicked() {
    if (!carColors_.isEmpty()) {
        currentColorIndex_ = (currentColorIndex_ + 1) % carColors_.size();
        current_car_.id_ = carColors_.at(currentColorIndex_).id_;
        current_car_.color_ = carColors_.at(currentColorIndex_).color_;
        current_car_.path_to_image_ = carColors_.at(currentColorIndex_).path_to_image_;
        showCarColor(carColors_.at(currentColorIndex_));
    }
}

void MainWindow::on_pushButton_to_pay_clicked()
{
    int newId = db_manager_.GetMaxOrMinValueFromTable("MAX", "id", "purchases") + 1;
    if (db_manager_.ExecuteQuery(QString("INSERT INTO public.purchases(id, car_id, client_id) VALUES (%1, %2, %3);").arg(newId).arg(current_car_.id_).arg(user_->GetId()))){
        QMessageBox::information(this, "Выполнение операции", "Произведена оплата. Купленный автомобиль добавлен в Профиль.");
    }
    else{
        QMessageBox::critical(this, "", db_manager_.GetLastError());
    }
}

void MainWindow::onSortByColorClicked() {
    if (!colorDropdown_) return;

    if (colorDropdown_->isVisible()) {
        colorDropdown_->setVisible(false);
    } else {
        UpdateColorDropdown(); // Обновляем данные
        colorDropdown_->setVisible(true);
    }
}
