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

QString themeFolder(ThemeMode mode)
{
    return mode == ThemeMode::Dark ? "dark" : "light";
}

QString resolveIconPath(const QString& iconName, ThemeMode mode)
{
    const QString kAppDir = QCoreApplication::applicationDirPath();
    const QStringList kCandidates
        = {QString(":/icons/%1/%2").arg(themeFolder(mode), iconName),
           QString(":/icons/light/%1").arg(iconName),
           QString(":/icons/common/%1").arg(iconName),
           QString(":/%1").arg(iconName),
           QDir::cleanPath(kAppDir + "/resources/icons/" + themeFolder(mode) + "/" + iconName),
           QDir::cleanPath(kAppDir + "/resources/icons/light/" + iconName),
           QDir::cleanPath(kAppDir + "/../../resources/icons/" + themeFolder(mode) + "/" + iconName),
           QDir::cleanPath(kAppDir + "/../../resources/icons/light/" + iconName)};

    for (const QString& path : kCandidates) {
        if (QFile::exists(path)) {
            return path;
        }
    }
    return QString();
}

} // namespace

ThemeMode getCurrentThemeMode()
{
    if (qApp) {
        const QVariant kProp = qApp->property("app_theme");
        if (kProp.isValid()) {
            return kProp.toString().trimmed().compare("dark", Qt::CaseInsensitive) == 0
                       ? ThemeMode::Dark
                       : ThemeMode::Light;
        }
    }

    return qEnvironmentVariable("APP_THEME").trimmed().compare("dark", Qt::CaseInsensitive) == 0
        ? ThemeMode::Dark
        : ThemeMode::Light;
}

QString loadThemeStyle(const QString& token, ThemeMode mode)
{
    const QString kPath = QString(":/styles/%1/%2.qss").arg(themeFolder(mode), token);
    QFile file(kPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    return QString::fromUtf8(file.readAll());
}

void applyThemeStyle(QWidget* widget, const QString& token, ThemeMode mode)
{
    if (!widget) {
        return;
    }
    widget->setProperty("theme_token", token);
    widget->setStyleSheet(loadThemeStyle(token, mode));
}

void reapplyThemeStyles(QWidget* root, ThemeMode mode)
{
    if (!root) {
        return;
    }

    const auto kApplyToken = [mode](QWidget* widget) {
        if (!widget) {
            return;
        }
        const QVariant kToken = widget->property("theme_token");
        if (kToken.isValid() && !kToken.toString().isEmpty()) {
            widget->setStyleSheet(loadThemeStyle(kToken.toString(), mode));
        }
    };

    kApplyToken(root);
    const auto kChildren = root->findChildren<QWidget*>();
    for (QWidget* child : kChildren) {
        kApplyToken(child);
    }
}

QIcon loadThemeIcon(const QString& iconName, ThemeMode mode)
{
    const QString kPath = resolveIconPath(iconName, mode);
    return kPath.isEmpty() ? QIcon() : QIcon(kPath);
}

void applyThemeIcon(QAbstractButton* button, const QString& iconName, ThemeMode mode)
{
    if (!button) {
        return;
    }
    button->setProperty("theme_icon_name", iconName);
    button->setIcon(loadThemeIcon(iconName, mode));
}

void reapplyThemeIcons(QWidget* root, ThemeMode mode)
{
    if (!root) {
        return;
    }

    const auto kApplyForButton = [mode](QAbstractButton* button) {
        if (!button) {
            return;
        }
        const QVariant kIconName = button->property("theme_icon_name");
        if (kIconName.isValid() && !kIconName.toString().isEmpty()) {
            button->setIcon(loadThemeIcon(kIconName.toString(), mode));
        }
    };

    const auto kRootButton = qobject_cast<QAbstractButton*>(root);
    if (kRootButton) {
        kApplyForButton(kRootButton);
    }

    const auto kButtons = root->findChildren<QAbstractButton*>();
    for (QAbstractButton* button : kButtons) {
        kApplyForButton(button);
    }
}
