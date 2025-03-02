#include "floating_widgets.h"

FloatingWidgets::FloatingWidgets(std::shared_ptr<DatabaseHandler> db_manager, QWidget* parent)
    : db_manager_(std::move(db_manager))
    , QWidget{parent}
{
    floating_menu_ = std::make_unique<QWidget>(parent);
    side_menu_ = std::make_unique<QWidget>(parent);
}

void FloatingWidgets::BuildFloatingMenu(
    const std::function<void()>& onMoreClicked,
    const std::function<void()>& onSearchClicked,
    // const std::function<void()>& onCartClicked,
    const std::function<void()>& onColorFilterClicked,
    const std::function<void()>& onUserProfileClicked
    ) {

    floating_menu_->setStyleSheet("background-color: #fafafa; border-radius: 39px;");
    floating_menu_->setFixedSize(88, 485);
    floating_menu_->setGeometry(0, 0, 88, 485);

    QVBoxLayout* menuLayout = new QVBoxLayout(floating_menu_.get());

    // Логотип
    QPushButton* logo_ = new QPushButton(floating_menu_.get());
    logo_->setIcon(QIcon(":/logo.svg"));
    logo_->setIconSize(QSize(41, 41));
    logo_->setStyleSheet("border: none; outline: none;");

    // Модели
    QPushButton* models_ = new QPushButton(floating_menu_.get());
    models_->setIcon(QIcon(":/directions_car.svg"));
    models_->setIconSize(QSize(41, 41));
    models_->setStyleSheet("border: none; outline: none;");
    connect(models_, &QPushButton::clicked, this, [onMoreClicked]() {
        if (onMoreClicked) onMoreClicked();
    });

    // Поиск
    QPushButton* searchButton = new QPushButton(floating_menu_.get());
    searchButton->setIcon(QIcon(":/search.svg"));
    searchButton->setIconSize(QSize(41, 41));
    searchButton->setStyleSheet("border: none; outline: none;");
    connect(searchButton, &QPushButton::clicked, this, [onSearchClicked]() {
        if (onSearchClicked) onSearchClicked();
    });

    // Сортировка по цветам
    QPushButton* sort_by_color_ = new QPushButton(floating_menu_.get());
    sort_by_color_->setIcon(QIcon(":/Color Swatch 02.svg"));
    sort_by_color_->setIconSize(QSize(41, 41));
    sort_by_color_->setStyleSheet("border: none; outline: none;");
    connect(sort_by_color_, &QPushButton::clicked, this, [onColorFilterClicked]() {
        if (onColorFilterClicked) onColorFilterClicked();
    });

    // Кнопка User Profile
    QPushButton* userButton = new QPushButton(floating_menu_.get());
    userButton->setIcon(QIcon(":/person.svg"));
    userButton->setIconSize(QSize(41, 41));
    userButton->setStyleSheet("border: none; outline: none;");
    connect(userButton, &QPushButton::clicked, this, [onUserProfileClicked]() {
        if (onUserProfileClicked) onUserProfileClicked();
    });

    // Добавляем кнопки в макет
    menuLayout->addWidget(logo_);
    menuLayout->addWidget(models_);
    menuLayout->addWidget(searchButton);
    menuLayout->addWidget(sort_by_color_);
    menuLayout->addWidget(userButton);

    // Устанавливаем позицию и показываем меню
    floating_menu_->move(37, 37);
    floating_menu_->show();
}

// void FloatingWidgets::BuildSideMenu(
//     const std::function<void(const QString&)>& onItemClicked,
//     const std::function<void()>& onViewAllClicked
//     ) {
//     // side_menu_ = std::make_unique<QWidget>(this);
//     side_menu_->setStyleSheet("background-color: #fafafa;");
//     side_menu_->setGeometry(0, 0, 224, 560);

//     QLabel* title = new QLabel("Каталог", side_menu_.get());
//     title->setStyleSheet("background-color: #fafafa; color: #140f10; font: 700 14pt 'Open Sans';");
//     title->setGeometry(28, 87, 172, 19);

//     side_list_ = new QListWidget(side_menu_.get());
//     side_list_->setStyleSheet("background-color: #fafafa; color: #140f10; font: 12pt 'Open Sans'; border: 0px;");
//     side_list_->setGeometry(22, 111, 224, 449);
//     side_list_->setSpacing(3);

//     // Заполнение данных из базы данных
//     QSqlQuery query("SELECT name FROM instrument_types");
//     while (query.next()) {
//         QString instrumentTypeName = query.value("name").toString();
//         side_list_->addItem(instrumentTypeName);
//     }

//     // Добавляем элемент "Смотреть всё"
//     QListWidgetItem* viewAllItem = new QListWidgetItem("Смотреть всё", side_list_);
//     viewAllItem->setTextAlignment(Qt::AlignLeft);
//     viewAllItem->setFont(QFont("Open Sans", 12));
//     viewAllItem->setForeground(QColor("#9b9c9c"));

//     // Обработка сигналов кликов
//     connect(side_list_, &QListWidget::itemClicked, this, [onItemClicked, onViewAllClicked](QListWidgetItem* item) {
//         if (item->text() == "Смотреть всё") {
//             if (onViewAllClicked) onViewAllClicked();
//         } else {
//             if (onItemClicked) onItemClicked(item->text());
//         }
//     });

//     side_menu_->show();
// }

void FloatingWidgets::BuildSideMenu(
    const std::function<void(const QString&)>& onItemClicked,
    const std::function<void()>& onViewAllClicked
    ) {
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
        nullptr, // Родительский виджет (nullptr для отдельного окна)
        "Выберите тип", // Заголовок окна
        "Тип:", // Текст перед выпадающим списком
        types, // Список элементов
        0, // Индекс выбранного элемента по умолчанию
        false, // Редактируемый ли список
        &ok // Успешно ли выбрано значение
        );

    // Обработка выбора
    if (ok && !selectedType.isEmpty()) {
        if (selectedType == "Смотреть всё") {
            if (onViewAllClicked) onViewAllClicked();
        } else {
            if (onItemClicked) onItemClicked(selectedType);
        }
    }
}
