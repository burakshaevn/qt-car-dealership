#include "UiKit.h"

#include "ThemeManager.h"

#include <QEvent>
#include <QFontMetrics>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>

namespace UiKit {

namespace {

/// Keeps a follower label's baseline on the reference label's baseline.
class BaselineAligner final : public QObject
{
public:
    BaselineAligner(QLabel* reference, QLabel* follower)
        : QObject(follower)
        , m_reference(reference)
        , m_follower(follower)
    {
        reference->installEventFilter(this);
        follower->installEventFilter(this);
        update();
    }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        switch (event->type()) {
        case QEvent::Polish:
        case QEvent::FontChange:
        case QEvent::StyleChange:
        case QEvent::Show:
            update();
            break;
        default:
            break;
        }
        return QObject::eventFilter(watched, event);
    }

private:
    void update()
    {
        // Both labels sit on the row's bottom edge, so their baselines differ by
        // the difference of their descents; lift the follower by that amount.
        const int kLift = QFontMetrics(m_reference->font()).descent() - QFontMetrics(m_follower->font()).descent();
        const QMargins kMargins(0, 0, 0, qMax(0, kLift));
        if (m_follower->contentsMargins() != kMargins) {
            m_follower->setContentsMargins(kMargins);
        }
    }

    QLabel* m_reference;
    QLabel* m_follower;
};

} // namespace

void alignBaseline(QLabel* reference, QLabel* follower)
{
    if (reference && follower) {
        new BaselineAligner(reference, follower);
    }
}

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
    result->setObjectName(QStringLiteral("rule"));
    result->setFrameShape(QFrame::NoFrame);
    return result;
}

void setTone(QLabel* badge, const Tone tone)
{
    static const char* const kTones[] = {"neutral", "accent", "success", "warning", "danger"};
    badge->setProperty("tone", QString::fromLatin1(kTones[static_cast<int>(tone)]));
    ThemeManager::repolish(badge);
}

void makeOverline(QLabel* label)
{
    label->setText(label->text().toUpper());
    QFont font = label->font();
    font.setLetterSpacing(QFont::AbsoluteSpacing, 1.1);
    label->setFont(font);
}

QLabel* overline(const QString& text, QWidget* parent)
{
    auto* result = label(text, "overline", parent);
    makeOverline(result);
    return result;
}

QLabel* badge(const QString& text, const Tone tone, QWidget* parent)
{
    auto* result = label(text, "status", parent);
    makeOverline(result);
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

QString plural(const qint64 count, const QString& one, const QString& few, const QString& many)
{
    const qint64 kMod100 = count % 100;
    const qint64 kMod10 = count % 10;
    if (kMod100 >= 11 && kMod100 <= 14) {
        return many;
    }
    if (kMod10 == 1) {
        return one;
    }
    if (kMod10 >= 2 && kMod10 <= 4) {
        return few;
    }
    return many;
}

QIcon swatchIcon(const QColor& color, const int size)
{
    if (!color.isValid()) {
        return {};
    }
    QIcon icon;
    for (const qreal kScale : {1.0, 2.0}) {
        QPixmap pixmap(QSize(size, size) * kScale);
        pixmap.setDevicePixelRatio(kScale);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(QPen(color.lightnessF() > 0.85 ? QColor(0, 0, 0, 60) : color.darker(120), 1));
        painter.setBrush(color);
        painter.drawRect(QRectF(0.5, 0.5, size - 1, size - 1));
        painter.end();
        icon.addPixmap(pixmap);
    }
    return icon;
}

} // namespace UiKit

FormDialog::FormDialog(const QString& title, const QString& subtitle, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(title);
    setModal(true);
    setMinimumWidth(460);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(32, 28, 32, 26);
    root->setSpacing(0);

    root->addWidget(UiKit::label(title, "h2", this));
    if (!subtitle.isEmpty()) {
        root->addSpacing(4);
        auto* sub = UiKit::label(subtitle, "muted", this);
        sub->setWordWrap(true);
        root->addWidget(sub);
    }
    root->addSpacing(18);
    root->addWidget(UiKit::divider(this));
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
    footer->setSpacing(18);
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
