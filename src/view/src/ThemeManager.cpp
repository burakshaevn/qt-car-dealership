#include "ThemeManager.h"

#include <QAbstractButton>
#include <QApplication>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QPainter>
#include <QPalette>
#include <QRegularExpression>
#include <QSettings>
#include <QStyle>
#include <algorithm>

Q_LOGGING_CATEGORY(lcTheme, "dealership.theme")

namespace {
const QString kThemesRoot = QStringLiteral(":/themes/");
const QString kFontsRoot = QStringLiteral(":/fonts/");
const QString kStyleSheet = QStringLiteral(":/themes/app.qss");
const QString kSettingsKey = QStringLiteral("ui/theme");
const QString kDefaultTheme = QStringLiteral("light");
const char* const kIconProperty = "themeIconName";
const char* const kIconColorProperty = "themeIconColor";
const char* const kIconCheckedProperty = "themeIconCheckedColor";
} // namespace

ThemeManager& ThemeManager::instance()
{
    static ThemeManager manager;
    return manager;
}

void ThemeManager::initialize(QApplication& app)
{
    m_app = &app;

    QDirIterator fonts(kFontsRoot, {QStringLiteral("*.ttf"), QStringLiteral("*.otf")}, QDir::Files);
    while (fonts.hasNext()) {
        const QString kPath = fonts.next();
        if (QFontDatabase::addApplicationFont(kPath) < 0) {
            qCWarning(lcTheme) << "Cannot load font" << kPath;
        }
    }

    const QString kSaved = QSettings().value(kSettingsKey, kDefaultTheme).toString();
    const QString kFromEnv = qEnvironmentVariable("APP_THEME");
    setTheme(kFromEnv.isEmpty() ? kSaved : kFromEnv);
}

QStringList ThemeManager::availableThemes() const
{
    QStringList names;
    QDirIterator it(kThemesRoot, {QStringLiteral("*.json")}, QDir::Files);
    while (it.hasNext()) {
        names << QFileInfo(it.next()).completeBaseName();
    }
    names.sort();
    return names;
}

QString ThemeManager::theme() const
{
    return m_theme;
}

bool ThemeManager::isDark() const
{
    return color(QStringLiteral("window")).lightnessF() < 0.5;
}

QString ThemeManager::fontFamily() const
{
    return m_fontFamily;
}

QString ThemeManager::displayFamily() const
{
    return m_displayFamily;
}

QFont ThemeManager::displayFont(const qreal pointSize, const QFont::Weight weight) const
{
    QFont font(m_displayFamily.isEmpty() ? QApplication::font().family() : m_displayFamily);
    font.setPointSizeF(pointSize);
    font.setWeight(weight);
    return font;
}

void ThemeManager::setTheme(const QString& name)
{
    if (name == m_theme && !m_colors.isEmpty()) {
        return;
    }
    if (!loadPalette(name) && !loadPalette(kDefaultTheme)) {
        qCCritical(lcTheme) << "No usable theme palette found";
        return;
    }
    QSettings().setValue(kSettingsKey, m_theme);
    apply();
    emit themeChanged();
}

bool ThemeManager::loadPalette(const QString& name)
{
    QFile file(kThemesRoot + name + QStringLiteral(".json"));
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(lcTheme) << "Theme not found:" << name;
        return false;
    }
    const QJsonObject kRoot = QJsonDocument::fromJson(file.readAll()).object();
    const QJsonObject kColors = kRoot.value(QStringLiteral("colors")).toObject();
    if (kColors.isEmpty()) {
        return false;
    }

    m_colors.clear();
    for (auto it = kColors.begin(); it != kColors.end(); ++it) {
        m_colors.insert(it.key(), QColor::fromString(it.value().toString()));
    }
    m_theme = name;
    m_iconSet = kRoot.value(QStringLiteral("icons")).toString(name);

    // Font families are part of the theme; fall back to the platform font when unavailable.
    const QJsonObject kFonts = kRoot.value(QStringLiteral("fonts")).toObject();
    const auto resolve = [](const QString& family) {
        return QFontDatabase::hasFamily(family) ? family : QString();
    };
    m_fontFamily = resolve(kFonts.value(QStringLiteral("ui")).toString());
    m_displayFamily = resolve(kFonts.value(QStringLiteral("display")).toString());
    if (m_displayFamily.isEmpty()) {
        m_displayFamily = m_fontFamily;
    }
    return true;
}

QColor ThemeManager::color(const QString& token) const
{
    const auto it = m_colors.constFind(token);
    if (it == m_colors.constEnd()) {
        qCWarning(lcTheme) << "Unknown colour token:" << token;
        return Qt::magenta;
    }
    return it.value();
}

void ThemeManager::apply()
{
    if (!m_app) {
        return;
    }

    if (!m_fontFamily.isEmpty()) {
        QFont font(m_fontFamily);
        font.setPointSizeF(10.5);
        font.setHintingPreference(QFont::PreferNoHinting);
        m_app->setFont(font);
    }

    QPalette palette = m_app->palette();
    palette.setColor(QPalette::Window, color(QStringLiteral("window")));
    palette.setColor(QPalette::WindowText, color(QStringLiteral("text")));
    palette.setColor(QPalette::Base, color(QStringLiteral("surface")));
    palette.setColor(QPalette::AlternateBase, color(QStringLiteral("surfaceAlt")));
    palette.setColor(QPalette::Text, color(QStringLiteral("text")));
    palette.setColor(QPalette::Button, color(QStringLiteral("surface")));
    palette.setColor(QPalette::ButtonText, color(QStringLiteral("text")));
    palette.setColor(QPalette::Highlight, color(QStringLiteral("accent")));
    palette.setColor(QPalette::HighlightedText, color(QStringLiteral("onAccent")));
    palette.setColor(QPalette::PlaceholderText, color(QStringLiteral("textMuted")));
    palette.setColor(QPalette::ToolTipBase, color(QStringLiteral("surface")));
    palette.setColor(QPalette::ToolTipText, color(QStringLiteral("text")));
    m_app->setPalette(palette);

    QFile file(kStyleSheet);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCCritical(lcTheme) << "Stylesheet is missing:" << kStyleSheet;
        return;
    }
    QString qss = QString::fromUtf8(file.readAll());
    static const QRegularExpression kComments(QStringLiteral(R"(/\*.*?\*/)"),
                                              QRegularExpression::DotMatchesEverythingOption);
    qss.remove(kComments);

    // Longest tokens first, so that @accentHover is not clobbered by @accent.
    QStringList tokens = m_colors.keys();
    std::sort(tokens.begin(), tokens.end(),
              [](const QString& a, const QString& b) { return a.size() > b.size(); });
    for (const QString& token : tokens) {
        const QColor kColor = m_colors.value(token);
        const QString kValue = kColor.alpha() == 255
                                   ? kColor.name(QColor::HexRgb)
                                   : QStringLiteral("rgba(%1, %2, %3, %4)")
                                         .arg(kColor.red())
                                         .arg(kColor.green())
                                         .arg(kColor.blue())
                                         .arg(kColor.alpha());
        qss.replace(QLatin1Char('@') + token, kValue);
    }
    qss.replace(QStringLiteral("$icons"), m_iconSet);
    qss.replace(QStringLiteral("$display"), m_displayFamily.isEmpty() ? m_app->font().family() : m_displayFamily);
    qss.replace(QStringLiteral("$font"), m_fontFamily.isEmpty() ? m_app->font().family() : m_fontFamily);

    static const QRegularExpression kUnresolved(QStringLiteral("@[A-Za-z]+"));
    if (const auto kMatch = kUnresolved.match(qss); kMatch.hasMatch()) {
        qCWarning(lcTheme) << "Unresolved token in stylesheet:" << kMatch.captured(0);
    }

    m_app->setStyleSheet(qss);
    refreshBoundIcons();
}

QIcon ThemeManager::icon(const QString& name) const
{
    // Theme-specific artwork first, then the shared monochrome line icons (:/icons/ui).
    const QString kFile = name.endsWith(QStringLiteral(".svg")) ? name : name + QStringLiteral(".svg");
    for (const QString& set : {m_iconSet, QStringLiteral("ui")}) {
        const QString kPath = QStringLiteral(":/icons/%1/%2").arg(set, kFile);
        if (QFile::exists(kPath)) {
            return QIcon(kPath);
        }
    }
    qCWarning(lcTheme) << "Icon not found:" << name;
    return {};
}

QIcon ThemeManager::tintedIcon(const QString& name, const QColor& tint, const QSize size) const
{
    const QIcon kSource = icon(name);
    if (kSource.isNull()) {
        return {};
    }
    QIcon result;
    for (const qreal kScale : {1.0, 2.0}) {
        QPixmap pixmap = kSource.pixmap(size * kScale);
        {
            QPainter painter(&pixmap);
            painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            painter.fillRect(pixmap.rect(), tint);
        }
        pixmap.setDevicePixelRatio(kScale);
        result.addPixmap(pixmap);
    }
    return result;
}

QString ThemeManager::themeTitle(const QString& name) const
{
    QFile file(kThemesRoot + name + QStringLiteral(".json"));
    if (!file.open(QIODevice::ReadOnly)) {
        return name;
    }
    return QJsonDocument::fromJson(file.readAll()).object().value(QStringLiteral("title")).toString(name);
}

void ThemeManager::bindIcon(QAbstractButton* button,
                            const QString& name,
                            const QString& colorToken,
                            const QString& checkedToken)
{
    if (!button) {
        return;
    }
    button->setProperty(kIconProperty, name);
    button->setProperty(kIconColorProperty, colorToken);
    button->setProperty(kIconCheckedProperty, checkedToken);

    const QSize kSize = button->iconSize().isValid() ? button->iconSize() : QSize(20, 20);
    if (colorToken.isEmpty()) {
        button->setIcon(icon(name));
        return;
    }
    QIcon result = tintedIcon(name, color(colorToken), kSize);
    if (!checkedToken.isEmpty()) {
        const QIcon kChecked = tintedIcon(name, color(checkedToken), kSize);
        for (const qreal kScale : {1.0, 2.0}) {
            QPixmap pixmap = kChecked.pixmap(kSize, kScale, QIcon::Normal, QIcon::Off);
            result.addPixmap(pixmap, QIcon::Normal, QIcon::On);
            result.addPixmap(pixmap, QIcon::Active, QIcon::On);
        }
    }
    button->setIcon(result);
}

void ThemeManager::refreshBoundIcons()
{
    if (!m_app) {
        return;
    }
    const auto kWidgets = QApplication::allWidgets();
    for (QWidget* widget : kWidgets) {
        auto* button = qobject_cast<QAbstractButton*>(widget);
        if (!button) {
            continue;
        }
        const QString kName = button->property(kIconProperty).toString();
        if (!kName.isEmpty()) {
            bindIcon(button,
                     kName,
                     button->property(kIconColorProperty).toString(),
                     button->property(kIconCheckedProperty).toString());
        }
    }
}

void ThemeManager::repolish(QWidget* widget)
{
    if (!widget) {
        return;
    }
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}
