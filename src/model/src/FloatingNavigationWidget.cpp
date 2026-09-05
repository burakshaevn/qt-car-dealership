#include "FloatingNavigationWidget.h"
#include "ThemeStyleProvider.h"
#include <QVBoxLayout>
#include <QPushButton>

FloatingNavigationWidget::FloatingNavigationWidget(QWidget* parent)
    : QWidget{parent}
{}

void FloatingNavigationWidget::buildFloatingMenu(const size_t kX,
                                                 const size_t kParentHeight,
                                                 const std::function<void()>& onMoreClicked,
                                                 const std::function<void()>& onSearchClicked,
                                                 const std::function<void()>& onColorFilterClicked,
                                                 const std::function<void()>& onUserProfileClicked)
{
    // Создаём основной контейнер для меню
    this->setFixedSize(88, 485);

    // Создаём виджет для фона с эффектом блюра
    QWidget* backgroundWidget = new QWidget(this);
    backgroundWidget->setFixedSize(88, 485);
    applyThemeStyle(backgroundWidget, "FloatingWidget");

    // Создаём layout для кнопок
    QVBoxLayout* menuLayout = new QVBoxLayout(this);
    menuLayout->setContentsMargins(0, 0, 0, 0);
    menuLayout->setSpacing(0);

    // Логотип
    QPushButton* logo = new QPushButton(this);
    applyThemeIcon(logo, "logo.svg");
    logo->setIconSize(QSize(41, 41));
    applyThemeStyle(logo, "IconButton");

    // Модели
    QPushButton* models = new QPushButton(this);
    applyThemeIcon(models, "directions_car.svg");
    models->setIconSize(QSize(41, 41));
    applyThemeStyle(models, "IconButton");
    connect(models, &QPushButton::clicked, this, [onMoreClicked]() {
        if (onMoreClicked) onMoreClicked();
    });

    // Поиск
    QPushButton* searchButton = new QPushButton(this);
    applyThemeIcon(searchButton, "search.svg");
    searchButton->setIconSize(QSize(41, 41));
    applyThemeStyle(searchButton, "IconButton");
    connect(searchButton, &QPushButton::clicked, this, [onSearchClicked]() {
        if (onSearchClicked) onSearchClicked();
    });

    // Сортировка по цветам
    QPushButton* sortByColor = new QPushButton(this);
    applyThemeIcon(sortByColor, "color_swatch_02.svg");
    sortByColor->setIconSize(QSize(41, 41));
    applyThemeStyle(sortByColor, "IconButton");
    connect(sortByColor, &QPushButton::clicked, this, [onColorFilterClicked]() {
        if (onColorFilterClicked) onColorFilterClicked();
    });

    // Кнопка User Profile
    QPushButton* userButton = new QPushButton(this);
    applyThemeIcon(userButton, "person.svg");
    userButton->setIconSize(QSize(41, 41));
    applyThemeStyle(userButton, "IconButton");
    connect(userButton, &QPushButton::clicked, this, [onUserProfileClicked]() {
        if (onUserProfileClicked) onUserProfileClicked();
    });

    // Добавляем кнопки в макет
    menuLayout->addWidget(logo);
    menuLayout->addWidget(models);
    menuLayout->addWidget(searchButton);
    menuLayout->addWidget(sortByColor);
    menuLayout->addWidget(userButton);

    this->move(kX, (kParentHeight - this->height()) / 2);
    this->show();
}
