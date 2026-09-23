#pragma once

#include <QHash>
#include <QMutex>
#include <QString>
#include <QStringList>

/*!
 * \brief Read-only catalogue of SQL assets compiled into the application (qrc).
 *
 * Layout of the embedded resources:
 *  - :/sql/queries/<group>/<name>.sql — one parameterised statement per file,
 *    addressed from C++ by its logical name "<group>/<name>";
 *  - :/sql/migrations/NNN_<title>.sql — ordered schema/data migrations;
 *  - :/sql/bootstrap/<name>.sql       — technical scripts (PRAGMAs, journal).
 *
 * The store never exposes SQL text to UI or business code: repositories only
 * pass logical names to DatabaseHandler.
 */
class SqlQueryStore final
{
public:
    struct Migration {
        int Version = 0;
        QString Name;
        QString Script;
    };

    [[nodiscard]] static QString query(const QString& name);
    [[nodiscard]] static QString bootstrap(const QString& name);
    [[nodiscard]] static QList<Migration> migrations();
    [[nodiscard]] static QStringList queryNames();

private:
    [[nodiscard]] static QString load(const QString& resourcePath);

    static QMutex s_mutex;
    static QHash<QString, QString> s_cache;
};
