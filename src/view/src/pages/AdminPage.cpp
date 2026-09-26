#include "pages/AdminPage.h"

#include "ThemeManager.h"
#include "UiKit.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVBoxLayout>

AdminPage::AdminPage(QWidget* parent)
    : QWidget(parent)
    , m_proxy(new QSortFilterProxyModel(this))
{
    setObjectName(QStringLiteral("adminPage"));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(48, 36, 48, 32);
    root->setSpacing(0);

    auto* header = new QHBoxLayout;
    auto* titles = new QVBoxLayout;
    titles->setSpacing(8);
    m_title = UiKit::label(QString(), "h1", this);
    m_title->setWordWrap(true);
    m_description = UiKit::label(QString(), "muted", this);
    m_description->setWordWrap(true);
    titles->addWidget(m_title);
    titles->addWidget(m_description);
    header->addLayout(titles, 1);
    m_summary = UiKit::label(QString(), "price", this);
    m_summary->hide();
    header->addWidget(m_summary, 0, Qt::AlignBottom);
    root->addLayout(header);
    root->addSpacing(28);

    // Toolbar
    auto* toolbar = new QHBoxLayout;
    toolbar->setSpacing(8);
    m_search = new QLineEdit(this);
    m_search->setObjectName(QStringLiteral("searchField"));
    m_search->setPlaceholderText(tr("Поиск по таблице"));
    m_search->setClearButtonEnabled(true);
    m_search->setFixedWidth(300);
    toolbar->addWidget(m_search, 0, Qt::AlignVCenter);
    toolbar->addStretch(1);

    auto makeButton = [&](const QString& text, const char* type, const QString& objectName) {
        auto* button = UiKit::button(text, type, this);
        button->setObjectName(objectName);
        toolbar->addWidget(button);
        return button;
    };
    m_delete = makeButton(tr("Удалить"), "ghost", QStringLiteral("delete_button"));
    toolbar->addSpacing(16);
    m_reject = makeButton(tr("Отклонить"), nullptr, QStringLiteral("reject_button"));
    m_edit = makeButton(tr("Изменить"), nullptr, QStringLiteral("edit_button"));
    m_approve = makeButton(tr("Одобрить"), "primary", QStringLiteral("approve_button"));
    m_add = makeButton(tr("Новая запись"), "primary", QStringLiteral("add_button"));
    root->addLayout(toolbar);
    root->addSpacing(20);

    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setFilterKeyColumn(-1);
    m_proxy->setSortCaseSensitivity(Qt::CaseInsensitive);

    m_table = new QTableView(this);
    m_table->setModel(m_proxy);
    m_table->setSortingEnabled(true);
    m_table->setAlternatingRowColors(false);
    m_table->setShowGrid(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setWordWrap(false);
    m_table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_table->verticalHeader()->hide();
    m_table->verticalHeader()->setDefaultSectionSize(38);
    m_table->horizontalHeader()->setHighlightSections(false);
    m_table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    root->addWidget(m_table, 1);

    connect(m_search, &QLineEdit::textChanged, m_proxy, &QSortFilterProxyModel::setFilterFixedString);
    connect(m_approve, &QPushButton::clicked, this, &AdminPage::approveRequested);
    connect(m_reject, &QPushButton::clicked, this, &AdminPage::rejectRequested);
    connect(m_add, &QPushButton::clicked, this, &AdminPage::addRequested);
    connect(m_edit, &QPushButton::clicked, this, &AdminPage::editRequested);
    connect(m_delete, &QPushButton::clicked, this, &AdminPage::deleteRequested);
    connect(m_table, &QTableView::doubleClicked, this, [this] {
        if (m_edit->isVisible()) {
            emit editRequested();
        }
    });
}

void AdminPage::setModel(QAbstractItemModel* model)
{
    m_search->clear();
    m_proxy->setSourceModel(model);
    m_table->sortByColumn(-1, Qt::AscendingOrder);
    m_table->resizeColumnsToContents();
    for (int column = 0; column < m_proxy->columnCount(); ++column) {
        m_table->setColumnWidth(column, qBound(60, m_table->columnWidth(column) + 24, 360));
    }
}

void AdminPage::setHeader(const QString& title, const QString& description, const QString& summary)
{
    m_title->setText(title);
    m_description->setText(description);
    m_description->setVisible(!description.isEmpty());
    m_summary->setText(summary);
    m_summary->setVisible(!summary.isEmpty());
}

void AdminPage::setRequestMode(const bool isRequestTable)
{
    m_approve->setVisible(isRequestTable);
    m_reject->setVisible(isRequestTable);
}

void AdminPage::setEditable(const bool editable)
{
    m_add->setVisible(editable);
    m_edit->setVisible(editable);
    m_delete->setVisible(editable);
}

int AdminPage::currentSourceRow() const
{
    const QModelIndex kIndex = m_table->currentIndex();
    if (!kIndex.isValid() || !m_table->selectionModel()->isRowSelected(kIndex.row(), {})) {
        return -1;
    }
    return m_proxy->mapToSource(kIndex).row();
}
