#pragma once

#ifndef FLOATING_WIDGET_H
#define FLOATING_WIDGET_H

#include <QWidget>

class FloatingWidget : public QWidget
{
    Q_OBJECT
public:

    /*!
     * \brief FloatingWidget - Конструктор плавающего виджета
     * \param parent Родительский виджет (необязательный параметр)
     */
    explicit FloatingWidget(QWidget* parent = nullptr);

    /*!
     * \brief BuildFloatingMenu - Создает и настраивает плавающее боковое меню
     * \param x — Координата X для расположения меню (в пикселях)
     * \param y — Координата Y для расположения меню (в пикселях)
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
    void BuildFloatingMenu(
        const size_t x,
        const size_t y,
        const std::function<void()>& onMoreClicked,
        const std::function<void()>& onSearchClicked,
        const std::function<void()>& onColorFilterClicked,
        const std::function<void()>& onUserProfileClicked
    );
};

#endif // FLOATING_WIDGET_H
