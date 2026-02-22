#include "floating_widget.h"
#include "ThemeStyleProvider.h"
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
    ApplyThemeStyle(background_widget, "FloatingWidget");

    // Создаём layout для кнопок
    QVBoxLayout* menu_layout = new QVBoxLayout(this);
    menu_layout->setContentsMargins(0, 0, 0, 0);
    menu_layout->setSpacing(0);

    // Логотип
    QPushButton* logo = new QPushButton(this);
    ApplyThemeIcon(logo, "logo.svg");
    logo->setIconSize(QSize(41, 41));
    ApplyThemeStyle(logo, "IconButton");

    // Модели
    QPushButton* models = new QPushButton(this);
    ApplyThemeIcon(models, "directions_car.svg");
    models->setIconSize(QSize(41, 41));
    ApplyThemeStyle(models, "IconButton");
    connect(models, &QPushButton::clicked, this, [onMoreClicked]() {
        if (onMoreClicked) onMoreClicked();
    });

    // Поиск
    QPushButton* search_button = new QPushButton(this);
    ApplyThemeIcon(search_button, "search.svg");
    search_button->setIconSize(QSize(41, 41));
    ApplyThemeStyle(search_button, "IconButton");
    connect(search_button, &QPushButton::clicked, this, [onSearchClicked]() {
        if (onSearchClicked) onSearchClicked();
    });

    // Сортировка по цветам
    QPushButton* sort_by_color = new QPushButton(this);
    ApplyThemeIcon(sort_by_color, "color_swatch_02.svg");
    sort_by_color->setIconSize(QSize(41, 41));
    ApplyThemeStyle(sort_by_color, "IconButton");
    connect(sort_by_color, &QPushButton::clicked, this, [onColorFilterClicked]() {
        if (onColorFilterClicked) onColorFilterClicked();
    });

    // Кнопка User Profile
    QPushButton* user_button = new QPushButton(this);
    ApplyThemeIcon(user_button, "person.svg");
    user_button->setIconSize(QSize(41, 41));
    ApplyThemeStyle(user_button, "IconButton");
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
