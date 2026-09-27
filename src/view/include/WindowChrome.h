#pragma once

#ifndef WINDOW_CHROME_H
#define WINDOW_CHROME_H

class QWidget;

/*!
 * \brief Makes the native title bar blend into the window instead of sitting on top
 *        of it as a separate grey strip.
 *
 * - macOS: transparent title bar filled with the theme background, title text hidden
 *   (the way Apple's own apps look).
 * - Windows 11: caption and border are painted with the theme background, dark mode
 *   follows the theme. Older Windows versions silently keep the default frame.
 * - Other platforms: the window manager owns the frame, nothing is changed.
 *
 * The chrome is re-applied whenever the theme changes.
 */
namespace WindowChrome {

void attach(QWidget* window);

} // namespace WindowChrome

#endif // WINDOW_CHROME_H
