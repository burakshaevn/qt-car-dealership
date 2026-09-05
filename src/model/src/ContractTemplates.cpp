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

namespace contract_templates {
namespace {

using TemplateValues = QHash<QString, QString>;

QString loadTemplate(const QString& code)
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

    const QString kTitle = query.value("title").toString().toHtmlEscaped();
    const QString kBody = query.value("body_template").toString();
    return QStringLiteral("<h1>%1</h1>%2").arg(kTitle, kBody);
}

QString renderTemplate(const QString& code, const TemplateValues& values)
{
    QString body = loadTemplate(code);
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

TemplateValues baseValues(const QString& currentDate,
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

    const QString kDefaultFileName = QStringLiteral("Договор_%1_%2")
                                         .arg(QString(product.Name).replace(' ', '_'),
                                              QDateTime::currentDateTime().toString("dd_MM_yyyy"));

    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(
        nullptr,
        QStringLiteral("Сохранить договор"),
        kDefaultFileName,
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
    auto values = baseValues(currentDate, carName, carColor);
    values.insert("car_price", carPrice);
    return renderTemplate("purchase", values);
}

QString getLoanContractHtml(const QString& currentDate,
                            const QString& carName,
                            const QString& carColor,
                            const QString& carPrice,
                            const QString& loanAmount,
                            const QString& loanTerm)
{
    auto values = baseValues(currentDate, carName, carColor);
    values.insert("car_price", carPrice);
    values.insert("loan_amount", loanAmount);
    values.insert("loan_term", loanTerm);
    return renderTemplate("loan", values);
}

QString getRentalContractHtml(const QString& currentDate,
                              const QString& carName,
                              const QString& carColor,
                              const QString& rentalDays,
                              const QString& startDate)
{
    auto values = baseValues(currentDate, carName, carColor);
    values.insert("rental_days", rentalDays);
    values.insert("start_date", startDate);
    return renderTemplate("rental", values);
}

QString getInsuranceContractHtml(const QString& currentDate,
                                 const QString& carName,
                                 const QString& carColor,
                                 const QString& insuranceType)
{
    auto values = baseValues(currentDate, carName, carColor);
    values.insert("insurance_type", insuranceType);
    return renderTemplate("insurance", values);
}

QString getServiceContractHtml(const QString& currentDate,
                               const QString& carName,
                               const QString& carColor,
                               const QString& serviceType,
                               const QString& scheduledDate)
{
    auto values = baseValues(currentDate, carName, carColor);
    values.insert("service_type", serviceType);
    values.insert("scheduled_date", scheduledDate);
    return renderTemplate("service", values);
}

QString getTestDriveContractHtml(const QString& currentDate,
                                 const QString& carName,
                                 const QString& carColor,
                                 const QString& scheduledDate)
{
    auto values = baseValues(currentDate, carName, carColor);
    values.insert("scheduled_date", scheduledDate);
    return renderTemplate("test_drive", values);
}

QString getOrderContractHtml(const QString& currentDate,
                             const QString& carName,
                             const QString& carColor,
                             const QString& carPrice,
                             const QString& trim)
{
    auto values = baseValues(currentDate, carName, carColor);
    values.insert("car_price", carPrice);
    values.insert("trim", trim);
    return renderTemplate("order", values);
}

} // namespace contract_templates
