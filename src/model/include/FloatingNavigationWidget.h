#pragma once

#ifndef FLOATING_NAVIGATION_WIDGET_H
#define FLOATING_NAVIGATION_WIDGET_H

#include <QWidget>

#include <cstddef>
#include <functional>

class FloatingNavigationWidget final : public QWidget
{
    Q_OBJECT
public:

    /*!
     * \brief Конструктор плавающей навигации
     * \param parent Родительский виджет (необязательный параметр)
     */
    explicit FloatingNavigationWidget(QWidget* parent = nullptr);

    /*!
     * \brief BuildFloatingMenu - Создает и настраивает плавающее боковое меню
     * \param parent_height — Высота родительского окна в пикселях (чтобы отобразить плавающее меню по центру по высоте)
     * \param onMoreClicked — Колбэк, вызываемый при клике на кнопку "Еще" (должен быть std::function<void()> или лямбда-функцией)
     * \param onSearchClicked — Колбэк, вызываемый при клике на кнопку поиска
     * \param onColorFilterClicked — Колбэк, вызываемый при клике на фильтр по цвету
     * \param onUserProfileClicked — Колбэк, вызываемый при клике на профиль пользователя
     *
     * \details Метод создает вертикальное меню с иконками, позиционирует его
     * в указанных координатах и связывает кнопки с переданными обработчиками событий
     *
     * \note Все колбэки являются опциональными - можно передавать nullptr,
     * если обработка какого-то события не требуется
     *
     * \warning Координаты x и y должны быть в пределах видимой области родительского виджета
     */
    void buildFloatingMenu(
        const size_t kX,
        const size_t kParentHeight,
        const std::function<void()>& onMoreClicked,
        const std::function<void()>& onSearchClicked,
        const std::function<void()>& onColorFilterClicked,
        const std::function<void()>& onUserProfileClicked
    );
};

#endif // FLOATING_NAVIGATION_WIDGET_H
