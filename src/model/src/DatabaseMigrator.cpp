#include "DatabaseMigrator.h"

#include "SqlQueries.h"
#include "SqlQueryStore.h"
#include "SqlScript.h"

#include <QLoggingCategory>
#include <QSet>
#include <QSqlError>
#include <QSqlQuery>

Q_LOGGING_CATEGORY(lcMigrator, "dealership.sql.migrator")

namespace {
const QString kJournalScript = QStringLiteral("migrations_table");
} // namespace

DatabaseMigrator::DatabaseMigrator(QSqlDatabase database)
    : m_database(std::move(database))
{}

QString DatabaseMigrator::errorString() const
{
    return m_error;
}

int DatabaseMigrator::currentVersion() const
{
    QSqlQuery query(m_database);
    if (!query.exec(SqlQueryStore::query(SqlQuery::System::kSelectAppliedMigrations))) {
        return 0;
    }
    int version = 0;
    while (query.next()) {
        version = qMax(version, query.value(0).toInt());
    }
    return version;
}

bool DatabaseMigrator::execScript(const QString& script)
{
    const QStringList kStatements = SqlScript::splitStatements(script);
    for (const QString& statement : kStatements) {
        QSqlQuery query(m_database);
        if (!query.exec(statement)) {
            m_error = query.lastError().text();
            qCCritical(lcMigrator).noquote() << "Statement failed:" << m_error << "\n" << statement.left(300);
            return false;
        }
    }
    return true;
}

bool DatabaseMigrator::migrate()
{
    m_error.clear();
    if (!m_database.isOpen()) {
        m_error = QStringLiteral("database is not open");
        return false;
    }

    if (!execScript(SqlQueryStore::bootstrap(kJournalScript))) {
        return false;
    }

    QSet<int> applied;
    {
        QSqlQuery query(m_database);
        if (!query.exec(SqlQueryStore::query(SqlQuery::System::kSelectAppliedMigrations))) {
            m_error = query.lastError().text();
            return false;
        }
        while (query.next()) {
            applied.insert(query.value(0).toInt());
        }
    }

    const auto kMigrations = SqlQueryStore::migrations();
    for (const auto& migration : kMigrations) {
        if (applied.contains(migration.Version)) {
            continue;
        }

        qCInfo(lcMigrator) << "Applying migration" << migration.Version << migration.Name;
        if (!m_database.transaction()) {
            m_error = m_database.lastError().text();
            return false;
        }

        bool ok = execScript(migration.Script);
        if (ok) {
            QSqlQuery journal(m_database);
            ok = journal.prepare(SqlQueryStore::query(SqlQuery::System::kInsertAppliedMigration));
            journal.bindValue(QStringLiteral(":version"), migration.Version);
            journal.bindValue(QStringLiteral(":name"), migration.Name);
            ok = ok && journal.exec();
            if (!ok) {
                m_error = journal.lastError().text();
            }
        }

        if (!ok) {
            m_database.rollback();
            qCCritical(lcMigrator) << "Migration" << migration.Version << "failed:" << m_error;
            return false;
        }
        m_database.commit();
    }
    return true;
}
