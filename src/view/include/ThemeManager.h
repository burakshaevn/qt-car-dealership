#pragma once

#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <QColor>
#include <QFont>
#include <QHash>
#include <QIcon>
#include <QObject>
#include <QSize>
#include <QString>

class QAbstractButton;
class QApplication;

/*!
 * \brief Application-wide theming.
 *
 * - Palettes live in :/themes/<name>.json (colour tokens only);
 * - the single stylesheet :/themes/app.qss references tokens as `@token`;
 * - widgets opt into variants through dynamic properties
 *   (e.g. `type="primary"`), never through inline style sheets.
 *
 * Custom-painted widgets (delegates) read colours via color(); they repaint on
 * themeChanged(). The chosen theme is persisted with QSettings.
 */
class ThemeManager final : public QObject
{
    Q_OBJECT
public:
    static ThemeManager& instance();

    /// Registers bundled fonts, restores the saved theme and applies it.
    void initialize(QApplication& app);

    [[nodiscard]] QStringList availableThemes() const;
    [[nodiscard]] QString theme() const;
    [[nodiscard]] bool isDark() const;
    void setTheme(const QString& name);

    [[nodiscard]] QColor color(const QString& token) const;
    [[nodiscard]] QString fontFamily() const;
    /// Serif family used for headings, model names and prices.
    [[nodiscard]] QString displayFamily() const;
    [[nodiscard]] QFont displayFont(qreal pointSize, QFont::Weight weight = QFont::Normal) const;

    /// Icon from the theme's icon set.
    [[nodiscard]] QIcon icon(const QString& name) const;
    /// Monochrome icon recoloured with \a color, rasterised at \a size (logical pixels).
    [[nodiscard]] QIcon tintedIcon(const QString& name, const QColor& color, QSize size = {48, 48}) const;

    /// Human-readable theme title from the palette file.
    [[nodiscard]] QString themeTitle(const QString& name) const;

    /*!
     * Sets a themed icon on \a button and keeps it in sync on theme changes.
     * \a colorToken tints a monochrome icon; \a checkedToken (optional) is used
     * for the checked state of checkable buttons.
     */
    void bindIcon(QAbstractButton* button,
                  const QString& name,
                  const QString& colorToken = {},
                  const QString& checkedToken = {});

    /// Forces a widget to re-evaluate property-based selectors after a property change.
    static void repolish(QWidget* widget);

signals:
    void themeChanged();

private:
    ThemeManager() = default;

    bool loadPalette(const QString& name);
    void apply();
    void refreshBoundIcons();

    QString m_theme;
    QString m_iconSet;
    QHash<QString, QColor> m_colors;
    QString m_fontFamily;
    QString m_displayFamily;
    QApplication* m_app = nullptr;
};

#endif // THEME_MANAGER_H
