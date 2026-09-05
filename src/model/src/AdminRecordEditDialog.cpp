#include "AdminRecordEditDialog.h"
#include "ThemeStyleProvider.h"

#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QScreen>
#include <QVBoxLayout>

AdminRecordEditDialog::AdminRecordEditDialog(const QSqlRecord& record, QWidget* parent)
    : QDialog(parent)
    , m_record(record)
{
    setWindowTitle(QStringLiteral("Редактирование записи"));
    applyThemeStyle(this, "DialogForm");

    // Основной макет
    auto* layout = new QVBoxLayout(this);

    // Создаём поля ввода для каждой колонки
    for (int i = 0; i < record.count(); ++i) {
        QLabel* label = new QLabel(record.fieldName(i), this);
        QLineEdit* editor = new QLineEdit(record.value(i).toString(), this);
        QString fieldName = record.fieldName(i);
        if (fieldName == "id") {
            editor->setReadOnly(true);
        }
        m_fields.append(editor);

        layout->addWidget(label);
        layout->addWidget(editor);
    }

    auto* buttonLayout = new QHBoxLayout();
    auto* saveButton = new QPushButton("Сохранить", this);
    auto* cancelButton = new QPushButton("Отмена", this);
    saveButton->setProperty("type", "primary");
    cancelButton->setProperty("type", "secondary");

    connect(saveButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    layout->addLayout(buttonLayout);
    setMinimumSize(300, 200);

    setSizeGripEnabled(true);
    adjustSize();

    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    move(screenGeometry.center() - rect().center());
}

QSqlRecord AdminRecordEditDialog::getUpdatedRecord() const
{
    QSqlRecord updatedRecord = m_record;

    for (int i = 0; i < m_fields.size(); ++i) {
        QString value = m_fields[i]->text();

        if (value.isEmpty()) {
            throw std::runtime_error(
                QString("Поле '%1' не может быть пустым.").arg(m_record.fieldName(i)).toStdString());
        }

        QString fieldName = m_record.fieldName(i);
        if (fieldName.toLower() == "email") {
            QRegularExpression emailRegex(R"((\w+)(\.\w+)*@(\w+)(\.\w{2,})+)");
            if (!emailRegex.match(value).hasMatch()) {
                throw std::invalid_argument(QString("Поле '%1' должно содержать действительный адрес электронной почты. Пример: example@site.com").arg(fieldName).toStdString());
            }
        }

        updatedRecord.setValue(i, value);
    }

    return updatedRecord;
}
