#include "ContractTemplates.h"
#include "ProductRepository.h"

#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QHash>
#include <QMessageBox>
#include <QPageSize>
#include <QPdfWriter>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextDocument>
#include <QTextStream>
#include <QUrl>

namespace ContractTemplates {
namespace {

using TemplateValues = QHash<QString, QString>;

QString LoadTemplate(const QString& code)
{
    QSqlQuery query;
    query.prepare(
        "SELECT title, body_template "
        "FROM contract_templates "
        "WHERE code = :code AND is_active = true "
        "ORDER BY version DESC LIMIT 1");
    query.bindValue(":code", code);

    if (!query.exec() || !query.next()) {
        qWarning() << "Contract template is unavailable:" << code << query.lastError().text();
        return {};
    }

    const QString title = query.value("title").toString().toHtmlEscaped();
    const QString body = query.value("body_template").toString();
    return QStringLiteral("<h1>%1</h1>%2").arg(title, body);
}

QString RenderTemplate(const QString& code, const TemplateValues& values)
{
    QString body = LoadTemplate(code);
    if (body.isEmpty()) {
        return {};
    }

    for (auto it = values.cbegin(); it != values.cend(); ++it) {
        body.replace(
            QStringLiteral("{{%1}}").arg(it.key()),
            it.value().toHtmlEscaped());
    }

    return QStringLiteral(R"(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <style>
        body { font-family: "Times New Roman", serif; color: #111; margin: 36px; line-height: 1.55; }
        h1 { font-size: 22px; text-align: center; margin-bottom: 28px; }
        h2 { font-size: 16px; margin-top: 22px; }
        p { font-size: 13px; text-align: justify; }
        table { width: 100%%; border-collapse: collapse; margin: 18px 0; }
        td { border: 1px solid #555; padding: 8px; font-size: 12px; }
        .signatures { margin-top: 52px; }
    </style>
</head>
<body>%1</body>
</html>)").arg(body);
}

TemplateValues BaseValues(const QString& currentDate,
                          const QString& carName,
                          const QString& carColor)
{
    return {
        {"current_date", currentDate},
        {"car_name", carName},
        {"car_color", carColor}
    };
}

} // namespace

void saveAsPdf(const QString& htmlContent, const QString& fileName)
{
    QPdfWriter writer(fileName);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(15, 15, 15, 15));
    writer.setResolution(300);

    QTextDocument document;
    document.setHtml(htmlContent);
    document.print(&writer);
}

void saveContract(const QString& content, const ProductInfo& product)
{
    if (content.isEmpty()) {
        QMessageBox::critical(
            nullptr,
            QStringLiteral("Ошибка"),
            QStringLiteral("Шаблон договора отсутствует в системной базе."));
        return;
    }

    const QString defaultFileName = QStringLiteral("Договор_%1_%2")
        .arg(QString(product.name_).replace(' ', '_'))
        .arg(QDateTime::currentDateTime().toString("dd_MM_yyyy"));

    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(
        nullptr,
        QStringLiteral("Сохранить договор"),
        defaultFileName,
        QStringLiteral("PDF (*.pdf);;HTML (*.html)"),
        &selectedFilter);

    if (fileName.isEmpty()) {
        return;
    }

    bool saved = false;
    if (selectedFilter.startsWith("PDF")) {
        if (!fileName.endsWith(".pdf", Qt::CaseInsensitive)) {
            fileName += ".pdf";
        }
        saveAsPdf(content, fileName);
        saved = QFile::exists(fileName);
    } else {
        if (!fileName.endsWith(".html", Qt::CaseInsensitive)) {
            fileName += ".html";
        }
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream.setEncoding(QStringConverter::Utf8);
            stream << content;
            saved = true;
        }
    }

    if (!saved) {
        QMessageBox::critical(nullptr, QStringLiteral("Ошибка"), QStringLiteral("Не удалось сохранить договор."));
        return;
    }

    if (QMessageBox::question(
            nullptr,
            QStringLiteral("Файл сохранён"),
            QStringLiteral("Договор сохранён. Открыть его?"),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
    }
}

QString getPurchaseContractHtml(const QString& currentDate,
                                const QString& carName,
                                const QString& carColor,
                                const QString& carPrice)
{
    auto values = BaseValues(currentDate, carName, carColor);
    values.insert("car_price", carPrice);
    return RenderTemplate("purchase", values);
}

QString getLoanContractHtml(const QString& currentDate,
                            const QString& carName,
                            const QString& carColor,
                            const QString& carPrice,
                            const QString& loanAmount,
                            const QString& loanTerm)
{
    auto values = BaseValues(currentDate, carName, carColor);
    values.insert("car_price", carPrice);
    values.insert("loan_amount", loanAmount);
    values.insert("loan_term", loanTerm);
    return RenderTemplate("loan", values);
}

QString getRentalContractHtml(const QString& currentDate,
                              const QString& carName,
                              const QString& carColor,
                              const QString& rentalDays,
                              const QString& startDate)
{
    auto values = BaseValues(currentDate, carName, carColor);
    values.insert("rental_days", rentalDays);
    values.insert("start_date", startDate);
    return RenderTemplate("rental", values);
}

QString getInsuranceContractHtml(const QString& currentDate,
                                 const QString& carName,
                                 const QString& carColor,
                                 const QString& insuranceType)
{
    auto values = BaseValues(currentDate, carName, carColor);
    values.insert("insurance_type", insuranceType);
    return RenderTemplate("insurance", values);
}

QString getServiceContractHtml(const QString& currentDate,
                               const QString& carName,
                               const QString& carColor,
                               const QString& serviceType,
                               const QString& scheduledDate)
{
    auto values = BaseValues(currentDate, carName, carColor);
    values.insert("service_type", serviceType);
    values.insert("scheduled_date", scheduledDate);
    return RenderTemplate("service", values);
}

QString getTestDriveContractHtml(const QString& currentDate,
                                 const QString& carName,
                                 const QString& carColor,
                                 const QString& scheduledDate)
{
    auto values = BaseValues(currentDate, carName, carColor);
    values.insert("scheduled_date", scheduledDate);
    return RenderTemplate("test_drive", values);
}

QString getOrderContractHtml(const QString& currentDate,
                             const QString& carName,
                             const QString& carColor,
                             const QString& carPrice,
                             const QString& trim)
{
    auto values = BaseValues(currentDate, carName, carColor);
    values.insert("car_price", carPrice);
    values.insert("trim", trim);
    return RenderTemplate("order", values);
}

} // namespace ContractTemplates
