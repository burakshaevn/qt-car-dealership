#include "WindowChrome.h"

#include <QColor>
#include <QWidget>

#import <AppKit/AppKit.h>

namespace WindowChrome {

void applyNative(QWidget* window, const QColor& background, const bool dark)
{
    auto* view = reinterpret_cast<NSView*>(window->winId());
    NSWindow* nsWindow = view.window;
    if (!nsWindow) {
        return;
    }
    // A transparent title bar shows the window background, so the traffic lights sit
    // directly on the page colour and no separate strip is drawn.
    nsWindow.titlebarAppearsTransparent = YES;
    nsWindow.titleVisibility = NSWindowTitleHidden;
    nsWindow.backgroundColor = [NSColor colorWithSRGBRed:background.redF()
                                                   green:background.greenF()
                                                    blue:background.blueF()
                                                   alpha:1.0];
    nsWindow.appearance = [NSAppearance appearanceNamed:dark ? NSAppearanceNameDarkAqua
                                                             : NSAppearanceNameAqua];
}

} // namespace WindowChrome
