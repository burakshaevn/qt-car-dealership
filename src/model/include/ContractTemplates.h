#pragma once

#ifndef CONTRACT_TEMPLATES_H
#define CONTRACT_TEMPLATES_H

#include <QHash>
#include <QSharedPointer>
#include <QString>

class DatabaseHandler;
class QWidget;

/*!
 * \brief Renders contracts from templates stored in the `contract_templates` table.
 *
 * The document skeleton (styles, layout) lives in :/contracts/document.html;
 * the body of each contract kind is versioned data in the database. Values are
 * substituted into `{{placeholder}}` markers and HTML-escaped.
 */
class ContractRenderer final
{
public:
    using Values = QHash<QString, QString>;

    explicit ContractRenderer(QSharedPointer<DatabaseHandler> database);

    /// Returns a complete HTML document or an empty string if the template is missing.
    [[nodiscard]] QString render(const QString& code, const Values& values) const;

private:
    QSharedPointer<DatabaseHandler> m_database;
};

/*!
 * \brief Asks the user for a destination and saves the contract as PDF or HTML.
 */
namespace ContractExporter {

bool saveAsPdf(const QString& html, const QString& fileName);
bool saveAsHtml(const QString& html, const QString& fileName);
void exportWithDialog(QWidget* parent, const QString& html, const QString& suggestedName);

} // namespace ContractExporter

#endif // CONTRACT_TEMPLATES_H
