#include "WindowChrome.h"

#include <QtGlobal>

#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)

#include "ThemeManager.h"

#include <QColor>
#include <QEvent>
#include <QObject>
#include <QPointer>
#include <QWidget>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <dwmapi.h>
#endif

namespace WindowChrome {

#ifdef Q_OS_MACOS
// Implemented in WindowChrome_mac.mm.
void applyNative(QWidget* window, const QColor& background, bool dark);
#endif

namespace {

#ifdef Q_OS_WIN
void applyNative(QWidget* window, const QColor& background, const bool dark)
{
    // Attribute ids from the Windows 11 SDK; defined here so older SDKs still build.
    // On Windows 10 the calls simply fail and the default frame is kept.
    constexpr DWORD kUseImmersiveDarkMode = 20;
    constexpr DWORD kBorderColor = 34;
    constexpr DWORD kCaptionColor = 35;
    constexpr DWORD kTextColor = 36;

    const auto toColorRef = [](const QColor& color) {
        return static_cast<COLORREF>(RGB(color.red(), color.green(), color.blue()));
    };
    const auto kHwnd = reinterpret_cast<HWND>(window->winId());
    const BOOL kDark = dark ? TRUE : FALSE;
    const COLORREF kCaption = toColorRef(background);
    const COLORREF kText = toColorRef(ThemeManager::instance().color(QStringLiteral("text")));
    DwmSetWindowAttribute(kHwnd, kUseImmersiveDarkMode, &kDark, sizeof(kDark));
    DwmSetWindowAttribute(kHwnd, kCaptionColor, &kCaption, sizeof(kCaption));
    DwmSetWindowAttribute(kHwnd, kBorderColor, &kCaption, sizeof(kCaption));
    DwmSetWindowAttribute(kHwnd, kTextColor, &kText, sizeof(kText));
}
#endif

void apply(QWidget* window)
{
    const ThemeManager& theme = ThemeManager::instance();
    applyNative(window, theme.color(QStringLiteral("window")), theme.isDark());
}

/// Re-applies the chrome whenever the native window is shown or recreated.
class ChromeFilter final : public QObject
{
public:
    using QObject::QObject;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        if (event->type() == QEvent::Show || event->type() == QEvent::WinIdChange) {
            if (auto* widget = qobject_cast<QWidget*>(watched); widget && widget->isWindow()) {
                apply(widget);
            }
        }
        return QObject::eventFilter(watched, event);
    }
};

} // namespace

void attach(QWidget* window)
{
    if (!window) {
        return;
    }
    window->installEventFilter(new ChromeFilter(window));
    QObject::connect(&ThemeManager::instance(), &ThemeManager::themeChanged, window,
                     [window = QPointer<QWidget>(window)] {
                         if (window && window->testAttribute(Qt::WA_WState_Created)) {
                             apply(window);
                         }
                     });
}

} // namespace WindowChrome

#else

namespace WindowChrome {

void attach(QWidget* /*window*/)
{
    // X11/Wayland: the frame belongs to the window manager.
}

} // namespace WindowChrome

#endif
