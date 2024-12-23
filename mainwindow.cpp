#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(1300, 760);

    // Настраиваем автоматическое подключение к базе данных
    // при запуске приложения и открываем базу данных
    QString hostname = "localhost";
    int port = 5432;
    QString dbname = "car-dealership";
    QString username = "postgres";
    QString password = "89274800234Nn";
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
                        user.full_name_ = query.value("name").toString();
                        user.email_ = query.value("email").toString();
                        user.password_ = query.value("password").toString();
                        user.role_ = Role::User;

                        if (user.password_ == ui->lineEdit_password->text()) {
                            ui->lineEdit_login->clear();
                            ui->lineEdit_password->clear();
                            QMessageBox::information(this, "Авторизация", "Выполнена авторизация как пользователь.");
                            UpdateUser(user, this);

                            table_ = std::make_unique<Table>(&db_manager_, user_.get(), nullptr);
                            connect(table_.get(), &Table::Logout, this, &MainWindow::on_pushButton_logout_clicked);
                            ui->stackedWidget->addWidget(table_.get());
                            ui->stackedWidget->setCurrentWidget(table_.get());

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
    this->ui->stackedWidget->setCurrentWidget(this->ui->login);
}

void MainWindow::SetupFloatingMenu(){
    QWidget* floatingMenu = new QWidget(this);
    floatingMenu->setStyleSheet("background-color: #fafafa; border-radius: 39px;");
    floatingMenu->setFixedSize(353, 96);

    // Горизонтальное размещение кнопок в меню
    QHBoxLayout* menuLayout = new QHBoxLayout(floatingMenu);
    menuLayout->setContentsMargins(20, 10, 20, 10);
    menuLayout->setSpacing(20);

    QPushButton* carButton = new QPushButton(floatingMenu);
    carButton->setIcon(QIcon("://directions_car.svg"));
    carButton->setIconSize(QSize(45, 45));
    carButton->setStyleSheet("QPushButton { border: none; outline: none; }");
    connect(carButton, &QPushButton::clicked, this, &MainWindow::on_pushButton_cars_clicked);

    QPushButton* searchButton = new QPushButton(floatingMenu);
    searchButton->setIcon(QIcon("://search.svg"));
    searchButton->setIconSize(QSize(45, 45));
    searchButton->setStyleSheet("QPushButton { border: none; outline: none; }");

    QPushButton* userButton = new QPushButton(floatingMenu);
    userButton->setIcon(QIcon("://person.svg"));
    userButton->setIconSize(QSize(45, 45));
    userButton->setStyleSheet("QPushButton { border: none; outline: none; }");
    connect(userButton, &QPushButton::clicked, this, &MainWindow::on_pushButton_profile_clicked);

    menuLayout->addWidget(carButton);
    menuLayout->addWidget(searchButton);
    menuLayout->addWidget(userButton);

    // Установим позицию меню (по центру внизу)
    floatingMenu->move((this->width() - floatingMenu->width()) / 2, this->height() - floatingMenu->height() - 20);
    floatingMenu->show();
}

void MainWindow::DrawCars(const QString& condition) {
    QWidget* oldWidget = ui->scrollArea->takeWidget();
    if (oldWidget) {
        oldWidget->deleteLater(); // Удаляем виджет асинхронно
    }

    // Создаем виджет-контейнер для карточек
    QWidget* container = new QWidget(ui->scrollArea);
    QVBoxLayout* layout = new QVBoxLayout(container);

    // Запрос к базе данных для получения автомобилей
    // auto queryResult = db_manager_.ExecuteSelectQuery(QString("SELECT name, color, price, image_url FROM cars;"));
    // auto queryResult = db_manager_.ExecuteSelectQuery(QString("SELECT name, color, price, image_url FROM cars WHERE color = 'Белый'"));
    auto queryResult = db_manager_.ExecuteSelectQuery(condition);

    if (queryResult.canConvert<QSqlQuery>()) {
        QSqlQuery query = queryResult.value<QSqlQuery>();
        while (query.next()) {
            QString name = query.value("name").toString();
            QString color = query.value("color").toString();
            double price = query.value("price").toDouble();

            QString relativePath = "/../../resources/";
            QString imagePath = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/" + relativePath + query.value("image_url").toString().replace("\\", "/"));

            QPixmap pixmap(imagePath);
            if (!pixmap.isNull()) {
                // Создаем карточку
                QWidget* carCard = new QWidget(container);
                carCard->setStyleSheet("background-color: #fafafa; border-radius: 50px;");
                carCard->setFixedSize(869, 207);

                // Загружаем изображение
                QPixmap originalPixmap(imagePath);

                // Проверяем, удалось ли загрузить изображение
                if (!originalPixmap.isNull()) {
                    // Фиксированная ширина для всех изображений
                    int fixedWidth = 271;

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
                    carImage->move(49, imageY);
                }

                // Добавляем название автомобиля
                QLabel* carName = new QLabel("<b>" + name + "</b>", carCard);
                carName->setStyleSheet("font: 22pt 'JetBrains Mono'; color: #1d1b20;");
                carName->setAlignment(Qt::AlignLeft);
                carName->setFixedSize(428, 34);
                carName->move(368, 40);

                // Добавляем цвет автомобиля
                QLabel* carColor = new QLabel(color, carCard);
                carColor->setStyleSheet("font: 16pt 'JetBrains Mono'; color: #555555;");
                carColor->setAlignment(Qt::AlignLeft);
                carColor->setFixedSize(400, 25);
                carColor->move(368, 83);

                // Функция для форматирования цены
                auto FormatPrice = [](int price) {
                    QString formattedPrice = QString::number(price); // Преобразуем в строку
                    int len = formattedPrice.length();
                    for (int i = len - 3; i > 0; i -= 3) {
                        formattedPrice.insert(i, ' '); // Вставляем пробел каждые три цифры
                    }
                    return formattedPrice;
                };

                // Использование функции FormatPrice
                QLabel* carPrice = new QLabel(FormatPrice(price) + " руб.", carCard);
                carPrice->setStyleSheet("font: 700 22pt 'JetBrains Mono'; color: #1d1b20;");
                carPrice->setAlignment(Qt::AlignRight);
                carPrice->setFixedSize(456, 34);
                carPrice->move(368, 133);

                // Добавляем карточку в контейнер
                layout->addWidget(carCard);

                connect(carCard, &QWidget::mousePressEvent, [=](QMouseEvent*) {
                    emit carSelected(name, color, price, imagePath);
                });
            } else {
                qDebug() << "Не удалось загрузить изображение: " << imagePath;
            }
        }
    }
    container->setLayout(layout);
    ui->scrollArea->setWidget(container);
}

void MainWindow::SetupSideMenu() {
    // Создаём боковое меню как QWidget
    side_widget_ = std::make_unique<QWidget>(this);
    side_widget_->setStyleSheet("background-color: #fafafa;");
    side_widget_->setGeometry(0, 0, 305, 760);

    // Добавляем логотип
    QLabel* logo = new QLabel(side_widget_.get());
    logo->setPixmap(QPixmap("://logo.svg").scaled(25, 25, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logo->setGeometry(26, 51, 25, 25);

    QLabel* logo_words = new QLabel(side_widget_.get());
    logo_words->setPixmap(QPixmap("://mercedez-benz.svg").scaled(218, 25, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logo_words->setGeometry(61, 51, 218, 25);

    // Добавляем заголовок
    QLabel* title = new QLabel("Модели", side_widget_.get());
    title->setStyleSheet("background-color: #fafafa; color: #140f10; font: 700 16pt 'JetBrains Mono';");
    title->setGeometry(30, 125, 234, 25);

    // Создаём QListWidget
    side_list_ = new QListWidget(side_widget_.get());
    side_list_->setStyleSheet("background-color: #fafafa; color: #140f10; font: 16pt 'JetBrains Mono'; border: 0px;");
    side_list_->setGeometry(30, 168, 275, 592);

    // Наполняем список моделей
    QSqlQuery query("SELECT name FROM car_types");
    while (query.next()) {
        QString carTypeName = query.value("name").toString();
        side_list_->addItem(carTypeName);
    }

    // Добавляем кнопку "Смотреть все" как элемент списка
    QListWidgetItem* viewAllItem = new QListWidgetItem("Смотреть все модели", side_list_);
    viewAllItem->setTextAlignment(Qt::AlignLeft); // Выравнивание текста по центру
    viewAllItem->setFont(QFont("JetBrains Mono", 16));
    viewAllItem->setForeground(QColor("#140f10"));

    // Обработка кликов по элементам меню
    connect(side_list_, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (item->text() == "Смотреть все модели") {
            // Логика для кнопки "Смотреть все"
            DrawCars("SELECT * FROM cars WHERE color = 'Белый'");
        } else {
            // Логика для остальных пунктов списка
            DrawCars(QString(R"(SELECT * FROM cars WHERE type_id =
                            (SELECT id FROM car_types WHERE name = '%1') AND color = '%2')")
                         .arg(item->text())
                         .arg("Белый"));
        }
        ui->stackedWidget->setCurrentWidget(ui->main);
    });

    // Делаем виджет видимым
    side_widget_->show();
}

void MainWindow::on_pushButton_profile_clicked()
{
    if (ui->stackedWidget->currentWidget() == ui->login || ui->stackedWidget->currentWidget() == table_.get()) {
        QMessageBox::information(this, "Информация", "Текущая страница — главная.");
    }
    else{
        if (user_->GetRole() == Role::User){
            // table_->LoadAppointments(user_->GetId());
            ui->stackedWidget->setCurrentWidget(table_.get());
        }
    }
}

// void MainWindow::on_pushButton_masters_clicked()
// {
//     if (user_.get()){
//         if (user_->GetRole() == Role::User){
//             ShowMasters();
//             // ui->stackedWidget->setCurrentWidget(ui->masters);
//             return;
//         }
//     }
//     QMessageBox::warning(this, "Ошибка", "Чтобы переключаться по остальным разделам, необходимо авторизоваться как пользователь.");
// }

void MainWindow::on_pushButton_cars_clicked()
{
    if (user_.get()){
        if (user_->GetRole() == Role::User) {
            // DrawCars("SELECT name, color, price, image_url FROM cars WHERE color = 'Белый'");
            ui->stackedWidget->setCurrentWidget(ui->main);
            return;
        }
    }
    QMessageBox::warning(this, "Ошибка", "Чтобы переключаться по остальным разделам, необходимо авторизоваться как пользователь.");
}

void MainWindow::on_pushButton_back_clicked()
{
    if (user_.get()){
        if (user_->GetRole() == Role::User) {
            // DrawCars("SELECT name, color, price, image_url FROM cars WHERE color = 'Белый'");
            ui->stackedWidget->setCurrentWidget(ui->main);
            return;
        }
    }
    QMessageBox::warning(this, "Ошибка", "Чтобы переключаться по остальным разделам, необходимо авторизоваться как пользователь.");
}

void MainWindow::onCarSelected(const QString& name, const QString& color, double price, const QString& imagePath) {
    // Устанавливаем данные на странице ui->personal (предполагается, что такие элементы есть в форме)
    ui->personalCarName->setText(name);
    ui->personalCarColor->setText(color);
    ui->personalCarPrice->setText(QString::number(price) + " руб.");

    QPixmap pixmap(imagePath);
    if (!pixmap.isNull()) {
        ui->personalCarImage->setPixmap(pixmap); // Устанавливаем изображение
    }

    // Переходим на страницу "personal"
    ui->stackedWidget->setCurrentWidget(ui->personal);
}
