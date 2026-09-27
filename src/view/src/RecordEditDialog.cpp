#include "RecordEditDialog.h"

#include <QLineEdit>
#include <QScrollArea>
#include <QSqlField>
#include <QVBoxLayout>

RecordEditDialog::RecordEditDialog(const QString& title,
                                   const QSqlRecord& record,
                                   const QStringList& readOnlyFields,
                                   const QHash<QString, QString>& labels,
                                   QWidget* parent)
    : FormDialog(title, tr("Пустые поля сохраняются как NULL."), parent)
    , m_record(record)
{
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto* host = new QWidget(scroll);
    auto* layout = new QVBoxLayout(host);
    layout->setContentsMargins(0, 0, 8, 0);
    layout->setSpacing(14);

    for (int i = 0; i < record.count(); ++i) {
        const QSqlField kField = record.field(i);
        auto* input = new QLineEdit(host);
        input->setText(kField.value().toString());
        const bool kReadOnly = readOnlyFields.contains(kField.name());
        input->setReadOnly(kReadOnly);
        if (kReadOnly && kField.value().isNull()) {
            input->setPlaceholderText(tr("назначается автоматически"));
        }
        m_inputs.append(input);
        layout->addWidget(UiKit::field(labels.value(kField.name(), kField.name()), input, host));
    }
    layout->addStretch(1);
    scroll->setWidget(host);
    scroll->setMinimumHeight(qMin(520, 74 * static_cast<int>(record.count())));
    addWidget(scroll);
    resize(520, sizeHint().height());
}

QSqlRecord RecordEditDialog::record() const
{
    QSqlRecord result = m_record;
    for (int i = 0; i < m_inputs.size(); ++i) {
        const QString kText = m_inputs.at(i)->text().trimmed();
        result.setValue(i, kText.isEmpty() ? QVariant() : QVariant(kText));
    }
    return result;
}
