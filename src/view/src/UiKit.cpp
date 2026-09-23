#include "UiKit.h"

#include "ThemeManager.h"

#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace UiKit {

void setRole(QWidget* widget, const char* role)
{
    if (widget && role) {
        widget->setProperty("role", QString::fromLatin1(role));
        ThemeManager::repolish(widget);
    }
}

void setType(QWidget* widget, const char* type)
{
    if (widget && type) {
        widget->setProperty("type", QString::fromLatin1(type));
        ThemeManager::repolish(widget);
    }
}

QLabel* label(const QString& text, const char* role, QWidget* parent)
{
    auto* result = new QLabel(text, parent);
    setRole(result, role);
    return result;
}

QPushButton* button(const QString& text, const char* type, QWidget* parent)
{
    auto* result = new QPushButton(text, parent);
    result->setCursor(Qt::PointingHandCursor);
    setType(result, type);
    return result;
}

QFrame* card(QWidget* parent)
{
    auto* result = new QFrame(parent);
    result->setProperty("card", true);
    return result;
}

QFrame* divider(QWidget* parent)
{
    auto* result = new QFrame(parent);
    result->setObjectName(QStringLiteral("divider"));
    result->setFrameShape(QFrame::NoFrame);
    return result;
}

void setTone(QLabel* badge, const Tone tone)
{
    static const char* const kTones[] = {"neutral", "accent", "success", "warning", "danger"};
    badge->setProperty("tone", QString::fromLatin1(kTones[static_cast<int>(tone)]));
    ThemeManager::repolish(badge);
}

QLabel* badge(const QString& text, const Tone tone, QWidget* parent)
{
    auto* result = label(text, "badge", parent);
    result->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    setTone(result, tone);
    return result;
}

QWidget* field(const QString& caption, QWidget* input, QWidget* parent)
{
    auto* container = new QWidget(parent);
    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);
    layout->addWidget(label(caption, "caption", container));
    input->setParent(container);
    layout->addWidget(input);
    return container;
}

void elevate(QWidget* widget, const int blurRadius, const int offsetY)
{
    auto* shadow = new QGraphicsDropShadowEffect(widget);
    shadow->setBlurRadius(blurRadius);
    shadow->setOffset(0, offsetY);
    shadow->setColor(ThemeManager::instance().color(QStringLiteral("shadow")));
    widget->setGraphicsEffect(shadow);
}

QString initials(const QString& fullName)
{
    QString result;
    const QStringList kParts = fullName.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    for (const QString& part : kParts) {
        result += part.front().toUpper();
        if (result.size() == 2) {
            break;
        }
    }
    return result.isEmpty() ? QStringLiteral("?") : result;
}

} // namespace UiKit

FormDialog::FormDialog(const QString& title, const QString& subtitle, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(title);
    setModal(true);
    setMinimumWidth(460);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 26, 28, 24);
    root->setSpacing(0);

    root->addWidget(UiKit::label(title, "h2", this));
    if (!subtitle.isEmpty()) {
        root->addSpacing(4);
        auto* sub = UiKit::label(subtitle, "muted", this);
        sub->setWordWrap(true);
        root->addWidget(sub);
    }
    root->addSpacing(20);

    m_body = new QVBoxLayout;
    m_body->setSpacing(14);
    root->addLayout(m_body);

    m_error = UiKit::label({}, "error", this);
    m_error->setWordWrap(true);
    m_error->hide();
    root->addSpacing(12);
    root->addWidget(m_error);
    root->addStretch(1);
    root->addSpacing(12);

    auto* footer = new QHBoxLayout;
    footer->setSpacing(10);
    footer->addStretch(1);
    m_cancel = UiKit::button(tr("Отмена"), "ghost", this);
    m_accept = UiKit::button(tr("Сохранить"), "primary", this);
    m_accept->setDefault(true);
    footer->addWidget(m_cancel);
    footer->addWidget(m_accept);
    root->addLayout(footer);

    connect(m_cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_accept, &QPushButton::clicked, this, &FormDialog::accept);
}

void FormDialog::addField(const QString& caption, QWidget* input)
{
    m_body->addWidget(UiKit::field(caption, input, this));
}

void FormDialog::addWidget(QWidget* widget)
{
    m_body->addWidget(widget);
}

void FormDialog::addSpacing(const int size)
{
    m_body->addSpacing(size);
}

void FormDialog::setAcceptText(const QString& text)
{
    m_accept->setText(text);
}

void FormDialog::setValidator(Validator validator)
{
    m_validator = std::move(validator);
}

void FormDialog::showError(const QString& message)
{
    m_error->setText(message);
    m_error->setVisible(!message.isEmpty());
}

void FormDialog::accept()
{
    if (m_validator) {
        const QString kError = m_validator();
        if (!kError.isEmpty()) {
            showError(kError);
            return;
        }
    }
    showError({});
    QDialog::accept();
}
