#include "edit_dialog.h"

EditDialog::EditDialog(const QSqlRecord& record, QWidget* parent)
    : QDialog(parent)
    , record_(record)
{
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
        fields_.append(editor);

        layout->addWidget(label);
        layout->addWidget(editor);
    }

    auto* buttonLayout = new QHBoxLayout();
    auto* saveButton = new QPushButton("Сохранить", this);
    auto* cancelButton = new QPushButton("Отмена", this);

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

QSqlRecord EditDialog::GetUpdatedRecord() const {
    QSqlRecord updatedRecord = record_;

    for (int i = 0; i < fields_.size(); ++i) {
        QString value = fields_[i]->text();

        if (value.isEmpty()) {
            throw std::runtime_error(
                QString("Поле '%1' не может быть пустым.").arg(record_.fieldName(i)).toStdString()
                );
        }

        QString fieldName = record_.fieldName(i);
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
