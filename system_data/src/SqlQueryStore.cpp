#include "SqlQueryStore.h"

#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QRegularExpression>
#include <algorithm>

Q_LOGGING_CATEGORY(lcSqlStore, "dealership.sql.store")

namespace {
const QString kQueriesRoot = QStringLiteral(":/sql/queries/");
const QString kMigrationsRoot = QStringLiteral(":/sql/migrations/");
const QString kBootstrapRoot = QStringLiteral(":/sql/bootstrap/");
const QString kSqlSuffix = QStringLiteral(".sql");
} // namespace

QMutex SqlQueryStore::s_mutex;
QHash<QString, QString> SqlQueryStore::s_cache;

QString SqlQueryStore::load(const QString& resourcePath)
{
    QMutexLocker lock(&s_mutex);
    if (const auto it = s_cache.constFind(resourcePath); it != s_cache.constEnd()) {
        return it.value();
    }

    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCCritical(lcSqlStore) << "SQL resource is missing:" << resourcePath;
        return {};
    }
    const QString kText = QString::fromUtf8(file.readAll()).trimmed();
    s_cache.insert(resourcePath, kText);
    return kText;
}

QString SqlQueryStore::query(const QString& name)
{
    return load(kQueriesRoot + name + kSqlSuffix);
}

QString SqlQueryStore::bootstrap(const QString& name)
{
    return load(kBootstrapRoot + name + kSqlSuffix);
}

QList<SqlQueryStore::Migration> SqlQueryStore::migrations()
{
    static const QRegularExpression kPattern(QStringLiteral("^(\\d+)_(.+)\\.sql$"));

    QList<Migration> result;
    QDirIterator it(kMigrationsRoot, {QStringLiteral("*.sql")}, QDir::Files);
    while (it.hasNext()) {
        const QFileInfo kInfo(it.next());
        const auto kMatch = kPattern.match(kInfo.fileName());
        if (!kMatch.hasMatch()) {
            qCWarning(lcSqlStore) << "Ignoring migration with unexpected name:" << kInfo.fileName();
            continue;
        }
        result.append({kMatch.captured(1).toInt(), kMatch.captured(2), load(kInfo.filePath())});
    }

    std::sort(result.begin(), result.end(), [](const Migration& a, const Migration& b) {
        return a.Version < b.Version;
    });
    return result;
}

QStringList SqlQueryStore::queryNames()
{
    QStringList names;
    QDirIterator it(kQueriesRoot, {QStringLiteral("*.sql")}, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString path = it.next();
        path.remove(0, kQueriesRoot.size());
        path.chop(kSqlSuffix.size());
        names << path;
    }
    names.sort();
    return names;
}
