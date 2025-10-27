#include "floating_widget.h"
#include <QVBoxLayout>
#include <QPushButton>

FloatingWidget::FloatingWidget(QWidget* parent)
    : QWidget{parent}
{}

void FloatingWidget::BuildFloatingMenu(
    const size_t x,
    const size_t parent_height,
    const std::function<void()>& onMoreClicked,
    const std::function<void()>& onSearchClicked,
    const std::function<void()>& onColorFilterClicked,
    const std::function<void()>& onUserProfileClicked
    ) {

    // Создаём основной контейнер для меню
    this->setFixedSize(88, 485);

    // Создаём виджет для фона с эффектом блюра
    QWidget* background_widget = new QWidget(this);
    background_widget->setFixedSize(88, 485);
    background_widget->setStyleSheet("background-color: #ffffff; border-radius: 39px;");

    // Создаём layout для кнопок
    QVBoxLayout* menu_layout = new QVBoxLayout(this);
    menu_layout->setContentsMargins(0, 0, 0, 0);
    menu_layout->setSpacing(0);

    // Логотип
    QPushButton* logo = new QPushButton(this);
    logo->setIcon(QIcon(":/logo.svg"));
    logo->setIconSize(QSize(41, 41));
    logo->setStyleSheet("border: none; outline: none; background: transparent;");

    // Модели
    QPushButton* models = new QPushButton(this);
    models->setIcon(QIcon(":/directions_car.svg"));
    models->setIconSize(QSize(41, 41));
    models->setStyleSheet("border: none; outline: none; background: transparent;");
    connect(models, &QPushButton::clicked, this, [onMoreClicked]() {
        if (onMoreClicked) onMoreClicked();
    });

    // Поиск
    QPushButton* search_button = new QPushButton(this);
    search_button->setIcon(QIcon(":/search.svg"));
    search_button->setIconSize(QSize(41, 41));
    search_button->setStyleSheet("border: none; outline: none; background: transparent;");
    connect(search_button, &QPushButton::clicked, this, [onSearchClicked]() {
        if (onSearchClicked) onSearchClicked();
    });

    // Сортировка по цветам
    QPushButton* sort_by_color = new QPushButton(this);
    sort_by_color->setIcon(QIcon(":/Color Swatch 02.svg"));
    sort_by_color->setIconSize(QSize(41, 41));
    sort_by_color->setStyleSheet("border: none; outline: none; background: transparent;");
    connect(sort_by_color, &QPushButton::clicked, this, [onColorFilterClicked]() {
        if (onColorFilterClicked) onColorFilterClicked();
    });

    // Кнопка User Profile
    QPushButton* user_button = new QPushButton(this);
    user_button->setIcon(QIcon(":/person.svg"));
    user_button->setIconSize(QSize(41, 41));
    user_button->setStyleSheet("border: none; outline: none; background: transparent;");
    connect(user_button, &QPushButton::clicked, this, [onUserProfileClicked]() {
        if (onUserProfileClicked) onUserProfileClicked();
    });

    // Добавляем кнопки в макет
    menu_layout->addWidget(logo);
    menu_layout->addWidget(models);
    menu_layout->addWidget(search_button);
    menu_layout->addWidget(sort_by_color);
    menu_layout->addWidget(user_button);

    this->move(x, (parent_height - this->height()) / 2);
    this->show();
}
