#include "AdminController.h"

#include "AdminTableModel.h"
#include "AppServices.h"
#include "PriceFormatter.h"
#include "RecordEditDialog.h"
#include "pages/AdminPage.h"

#include <QMessageBox>
#include <QSqlIndex>

AdminController::AdminController(AppServices& services, AdminPage* page, QObject* parent)
    : QObject(parent)
    , m_services(services)
    , m_page(page)
    , m_model(new AdminTableModel(services.database()))
    , m_tables(services.admin().tables())
{
    m_page->setModel(m_model.get());
    connect(m_page, &AdminPage::approveRequested, this, [this] { setStatus(true); });
    connect(m_page, &AdminPage::rejectRequested, this, [this] { setStatus(false); });
    connect(m_page, &AdminPage::addRequested, this, &AdminController::addRecord);
    connect(m_page, &AdminPage::editRequested, this, &AdminController::editRecord);
    connect(m_page, &AdminPage::deleteRequested, this, &AdminController::deleteRecord);
}

AdminController::~AdminController() = default;

void AdminController::open(const QString& tableName)
{
    const auto kIt = std::find_if(m_tables.cbegin(), m_tables.cend(),
                                  [&](const AdminTableInfo& info) { return info.TableName == tableName; });
    if (kIt == m_tables.cend()) {
        return;
    }
    if (!m_model->load(*kIt)) {
        QMessageBox::critical(m_page, tr("Ошибка"), m_services.database()->userMessage(m_model->lastError()));
        return;
    }
    m_labels = m_services.admin().columnLabels(kIt->TableName);
    for (int column = 0; column < m_model->columnCount(); ++column) {
        const QString kName = m_model->record().fieldName(column);
        if (const auto kLabel = m_labels.constFind(kName); kLabel != m_labels.cend()) {
            m_model->setHeaderData(column, Qt::Horizontal, *kLabel);
        }
    }
    reload();
}

void AdminController::reload()
{
    const AdminTableInfo& info = m_model->info();
    QString summary;
    if (info.TableName == QLatin1String("purchases")) {
        summary = tr("%1 ₽").arg(formatPrice(m_services.admin().salesTotal()));
    }
    m_model->select();
    m_page->setHeader(info.DisplayName, info.Description, summary);
    m_page->setRequestMode(info.IsRequest);
    m_page->setEditable(info.isEditable());
    m_page->setModel(m_model.get());
}

int AdminController::selectedRow(const QString& action)
{
    const int kRow = m_page->currentSourceRow();
    if (kRow < 0) {
        QMessageBox::information(m_page, action, tr("Выберите строку в таблице."));
    }
    return kRow;
}

QStringList AdminController::primaryKeyFields() const
{
    QStringList result;
    const QSqlIndex kPrimary = m_model->primaryKey();
    for (int i = 0; i < kPrimary.count(); ++i) {
        result << kPrimary.fieldName(i);
    }
    return result;
}

void AdminController::setStatus(const bool approve)
{
    const AdminTableInfo& info = m_model->info();
    const int kRow = selectedRow(approve ? tr("Одобрить") : tr("Отклонить"));
    if (kRow < 0 || !info.IsRequest) {
        return;
    }
    const QString kStatus = approve ? info.ApproveStatus : info.RejectStatus;
    QString error;
    if (!m_services.requests().updateStatus({info.TableName, m_model->keyAt(kRow).toInt()}, kStatus, &error)) {
        QMessageBox::warning(m_page, tr("Не удалось изменить статус"), error);
        return;
    }
    reload();
}

void AdminController::addRecord()
{
    RecordEditDialog dialog(tr("Новая запись"), m_model->blankRecord(), {}, m_labels, m_page);
    dialog.setAcceptText(tr("Добавить"));
    dialog.setValidator([&]() -> QString {
        QString error;
        return m_model->insert(dialog.record(), &error) ? QString() : error;
    });
    if (dialog.exec() == QDialog::Accepted) {
        reload();
    }
}

void AdminController::editRecord()
{
    const int kRow = selectedRow(tr("Изменить"));
    if (kRow < 0) {
        return;
    }
    RecordEditDialog dialog(tr("Изменение записи"), m_model->record(kRow), primaryKeyFields(), m_labels, m_page);
    dialog.setValidator([&]() -> QString {
        QString error;
        return m_model->update(kRow, dialog.record(), &error) ? QString() : error;
    });
    if (dialog.exec() == QDialog::Accepted) {
        reload();
    }
}

void AdminController::deleteRecord()
{
    const int kRow = selectedRow(tr("Удалить"));
    if (kRow < 0) {
        return;
    }
    const AdminTableInfo& info = m_model->info();
    QString message = tr("Удалить запись №%1 из «%2»?").arg(m_model->keyAt(kRow).toString(), info.DisplayName);
    const QStringList kDependants = m_services.admin().referencingTables(info.TableName);
    if (!kDependants.isEmpty()) {
        message += QStringLiteral("\n\n") + tr("Связанные записи также будут изменены:\n%1").arg(kDependants.join('\n'));
    }
    if (QMessageBox::warning(m_page, tr("Удаление"), message, QMessageBox::Yes | QMessageBox::Cancel,
                             QMessageBox::Cancel)
        != QMessageBox::Yes) {
        return;
    }
    QString error;
    if (!m_model->remove(kRow, &error)) {
        QMessageBox::warning(m_page, tr("Не удалось удалить"), error);
        return;
    }
    reload();
}
