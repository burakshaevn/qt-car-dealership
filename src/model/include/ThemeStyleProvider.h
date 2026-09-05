#pragma once

#ifndef THEME_STYLE_PROVIDER_H
#define THEME_STYLE_PROVIDER_H

#include <QString>

class QWidget;
class QAbstractButton;
class QIcon;

enum class ThemeMode
{
    Light,
    Dark
};

ThemeMode getCurrentThemeMode();
QString loadThemeStyle(const QString& token, ThemeMode mode = getCurrentThemeMode());
void applyThemeStyle(QWidget* widget, const QString& token, ThemeMode mode = getCurrentThemeMode());
void reapplyThemeStyles(QWidget* root, ThemeMode mode = getCurrentThemeMode());
QIcon loadThemeIcon(const QString& iconName, ThemeMode mode = getCurrentThemeMode());
void applyThemeIcon(QAbstractButton* button, const QString& iconName, ThemeMode mode = getCurrentThemeMode());
void reapplyThemeIcons(QWidget* root, ThemeMode mode = getCurrentThemeMode());

#endif // THEME_STYLE_PROVIDER_H
