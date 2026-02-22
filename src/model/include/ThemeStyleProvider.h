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

ThemeMode GetCurrentThemeMode();
QString LoadThemeStyle(const QString& token, ThemeMode mode = GetCurrentThemeMode());
void ApplyThemeStyle(QWidget* widget, const QString& token, ThemeMode mode = GetCurrentThemeMode());
void ReapplyThemeStyles(QWidget* root, ThemeMode mode = GetCurrentThemeMode());
QIcon LoadThemeIcon(const QString& iconName, ThemeMode mode = GetCurrentThemeMode());
void ApplyThemeIcon(QAbstractButton* button, const QString& iconName, ThemeMode mode = GetCurrentThemeMode());
void ReapplyThemeIcons(QWidget* root, ThemeMode mode = GetCurrentThemeMode());

#endif // THEME_STYLE_PROVIDER_H
