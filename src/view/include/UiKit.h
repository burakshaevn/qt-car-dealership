#pragma once

#ifndef UI_KIT_H
#define UI_KIT_H

#include <QDialog>
#include <QString>
#include <functional>

class QFrame;
class QLabel;
class QPushButton;
class QVBoxLayout;
class QWidget;

/*!
 * \brief Small set of UI primitives shared by all screens.
 *
 * Visual appearance is defined exclusively by the application stylesheet;
 * these helpers only assign semantic properties (role/type/tone) to widgets.
 */
namespace UiKit {

enum class Tone { Neutral, Accent, Success, Warning, Danger };

QLabel* label(const QString& text, const char* role = nullptr, QWidget* parent = nullptr);
QPushButton* button(const QString& text, const char* type = nullptr, QWidget* parent = nullptr);
QFrame* card(QWidget* parent = nullptr);
QFrame* divider(QWidget* parent = nullptr);
QLabel* badge(const QString& text, Tone tone, QWidget* parent = nullptr);

void setRole(QWidget* widget, const char* role);
void setType(QWidget* widget, const char* type);
void setTone(QLabel* badge, Tone tone);

/// Caption + input stacked vertically.
QWidget* field(const QString& caption, QWidget* input, QWidget* parent = nullptr);

/// Adds a drop shadow suitable for floating surfaces.
void elevate(QWidget* widget, int blurRadius = 32, int offsetY = 8);

/// Two-letter monogram for avatars.
QString initials(const QString& fullName);

} // namespace UiKit

/*!
 * \brief Standard modal form: title, optional subtitle, fields, inline error, footer buttons.
 *
 * A validator can be installed; it runs on accept and returns an error text
 * (shown inline) or an empty string to accept the dialog.
 */
class FormDialog : public QDialog
{
    Q_OBJECT
public:
    using Validator = std::function<QString()>;

    FormDialog(const QString& title, const QString& subtitle = {}, QWidget* parent = nullptr);

    void addField(const QString& caption, QWidget* input);
    void addWidget(QWidget* widget);
    void addSpacing(int size);

    void setAcceptText(const QString& text);
    void setValidator(Validator validator);
    void showError(const QString& message);

    [[nodiscard]] QPushButton* acceptButton() const { return m_accept; }
    [[nodiscard]] QPushButton* cancelButton() const { return m_cancel; }
    [[nodiscard]] QVBoxLayout* body() const { return m_body; }

    void accept() override;

private:
    QVBoxLayout* m_body = nullptr;
    QLabel* m_error = nullptr;
    QPushButton* m_accept = nullptr;
    QPushButton* m_cancel = nullptr;
    Validator m_validator;
};

#endif // UI_KIT_H
