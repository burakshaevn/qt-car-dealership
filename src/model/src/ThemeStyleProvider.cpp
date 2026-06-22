#include "ThemeStyleProvider.h"

#include <QApplication>
#include <QAbstractButton>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QStringList>
#include <QWidget>

namespace {

QString ThemeFolder(ThemeMode mode)
{
    return mode == ThemeMode::Dark ? "dark" : "light";
}

QString ResolveIconPath(const QString& iconName, ThemeMode mode)
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
        QString(":/icons/%1/%2").arg(ThemeFolder(mode), iconName),
        QString(":/icons/light/%1").arg(iconName),
        QString(":/icons/common/%1").arg(iconName),
        QString(":/%1").arg(iconName),
        QDir::cleanPath(appDir + "/resources/icons/" + ThemeFolder(mode) + "/" + iconName),
        QDir::cleanPath(appDir + "/resources/icons/light/" + iconName),
        QDir::cleanPath(appDir + "/../../resources/icons/" + ThemeFolder(mode) + "/" + iconName),
        QDir::cleanPath(appDir + "/../../resources/icons/light/" + iconName)
    };

    for (const QString& path : candidates) {
        if (QFile::exists(path)) {
            return path;
        }
    }
    return QString();
}

} // namespace

ThemeMode GetCurrentThemeMode()
{
    if (qApp) {
        const QVariant prop = qApp->property("app_theme");
        if (prop.isValid()) {
            return prop.toString().trimmed().compare("dark", Qt::CaseInsensitive) == 0
                ? ThemeMode::Dark
                : ThemeMode::Light;
        }
    }

    return qEnvironmentVariable("APP_THEME").trimmed().compare("dark", Qt::CaseInsensitive) == 0
        ? ThemeMode::Dark
        : ThemeMode::Light;
}

QString LoadThemeStyle(const QString& token, ThemeMode mode)
{
    const QString path = QString(":/styles/%1/%2.qss").arg(ThemeFolder(mode), token);
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    return QString::fromUtf8(file.readAll());
}

void ApplyThemeStyle(QWidget* widget, const QString& token, ThemeMode mode)
{
    if (!widget) {
        return;
    }
    widget->setProperty("theme_token", token);
    widget->setStyleSheet(LoadThemeStyle(token, mode));
}

void ReapplyThemeStyles(QWidget* root, ThemeMode mode)
{
    if (!root) {
        return;
    }

    const auto applyToken = [mode](QWidget* widget) {
        if (!widget) {
            return;
        }
        const QVariant token = widget->property("theme_token");
        if (token.isValid() && !token.toString().isEmpty()) {
            widget->setStyleSheet(LoadThemeStyle(token.toString(), mode));
        }
    };

    applyToken(root);
    const auto children = root->findChildren<QWidget*>();
    for (QWidget* child : children) {
        applyToken(child);
    }
}

QIcon LoadThemeIcon(const QString& iconName, ThemeMode mode)
{
    const QString path = ResolveIconPath(iconName, mode);
    return path.isEmpty() ? QIcon() : QIcon(path);
}

void ApplyThemeIcon(QAbstractButton* button, const QString& iconName, ThemeMode mode)
{
    if (!button) {
        return;
    }
    button->setProperty("theme_icon_name", iconName);
    button->setIcon(LoadThemeIcon(iconName, mode));
}

void ReapplyThemeIcons(QWidget* root, ThemeMode mode)
{
    if (!root) {
        return;
    }

    const auto applyForButton = [mode](QAbstractButton* button) {
        if (!button) {
            return;
        }
        const QVariant iconName = button->property("theme_icon_name");
        if (iconName.isValid() && !iconName.toString().isEmpty()) {
            button->setIcon(LoadThemeIcon(iconName.toString(), mode));
        }
    };

    const auto rootButton = qobject_cast<QAbstractButton*>(root);
    if (rootButton) {
        applyForButton(rootButton);
    }

    const auto buttons = root->findChildren<QAbstractButton*>();
    for (QAbstractButton* button : buttons) {
        applyForButton(button);
    }
}
