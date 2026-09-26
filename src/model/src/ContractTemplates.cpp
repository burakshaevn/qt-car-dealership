#include "ContractTemplates.h"

#include "DatabaseHandler.h"

#include <QDate>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QLocale>
#include <QMessageBox>
#include <QPageSize>
#include <QPdfWriter>
#include <QTextDocument>
#include <QUrl>

namespace {
const QString kDocumentResource = QStringLiteral(":/contracts/document.html");

QString placeholder(const QString& key)
{
    return QStringLiteral("{{%1}}").arg(key);
}

QString loadDocumentSkeleton()
{
    QFile file(kDocumentResource);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return placeholder(QStringLiteral("body"));
    }
    return QString::fromUtf8(file.readAll());
}
} // namespace

ContractRenderer::ContractRenderer(QSharedPointer<DatabaseHandler> database)
    : m_database(std::move(database))
{}

QString ContractRenderer::render(const QString& code, const Values& values) const
{
    if (!m_database) {
        return {};
    }
    const auto kRows = m_database->rows(SqlQuery::Contracts::kSelectActiveTemplate, {{"code", code}});
    if (kRows.isEmpty()) {
        return {};
    }

    Values resolved = values;
    if (!resolved.contains(QStringLiteral("current_date"))) {
        resolved.insert(QStringLiteral("current_date"), QLocale().toString(QDate::currentDate(), QLocale::ShortFormat));
    }

    QString body = kRows.first().value("body_template").toString();
    for (auto it = resolved.cbegin(); it != resolved.cend(); ++it) {
        body.replace(placeholder(it.key()), it.value().toHtmlEscaped());
    }

    QString document = loadDocumentSkeleton();
    document.replace(placeholder(QStringLiteral("title")), kRows.first().value("title").toString().toHtmlEscaped());
    document.replace(placeholder(QStringLiteral("body")), body);
    return document;
}

namespace ContractExporter {

bool saveAsPdf(const QString& html, const QString& fileName)
{
    QPdfWriter writer(fileName);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(15, 15, 15, 15));
    writer.setResolution(300);

    QTextDocument document;
    document.setHtml(html);
    document.print(&writer);
    return QFile::exists(fileName);
}

bool saveAsHtml(const QString& html, const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }
    return file.write(html.toUtf8()) >= 0;
}

void exportWithDialog(QWidget* parent, const QString& html, const QString& suggestedName)
{
    if (html.isEmpty()) {
        QMessageBox::critical(parent, QObject::tr("Договор"),
                              QObject::tr("Шаблон договора отсутствует в базе данных."));
        return;
    }

    const QString kPdfFilter = QObject::tr("PDF (*.pdf)");
    const QString kHtmlFilter = QObject::tr("HTML (*.html)");
    QString selectedFilter = kPdfFilter;
    QString fileName = QFileDialog::getSaveFileName(
        parent, QObject::tr("Сохранить договор"),
        QStringLiteral("%1_%2").arg(suggestedName, QDate::currentDate().toString(Qt::ISODate)),
        kPdfFilter + QStringLiteral(";;") + kHtmlFilter, &selectedFilter);
    if (fileName.isEmpty()) {
        return;
    }

    const bool kPdf = selectedFilter == kPdfFilter;
    const QString kSuffix = kPdf ? QStringLiteral(".pdf") : QStringLiteral(".html");
    if (!fileName.endsWith(kSuffix, Qt::CaseInsensitive)) {
        fileName += kSuffix;
    }

    const bool kSaved = kPdf ? saveAsPdf(html, fileName) : saveAsHtml(html, fileName);
    if (!kSaved) {
        QMessageBox::critical(parent, QObject::tr("Договор"), QObject::tr("Не удалось сохранить файл."));
        return;
    }

    if (QMessageBox::question(parent, QObject::tr("Договор сохранён"), QObject::tr("Открыть файл?"))
        == QMessageBox::Yes) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
    }
}

} // namespace ContractExporter
