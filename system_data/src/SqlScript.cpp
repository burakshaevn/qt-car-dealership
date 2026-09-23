#include "SqlScript.h"

namespace SqlScript {
namespace {

bool isIdentifierChar(QChar c)
{
    return c.isLetterOrNumber() || c == QLatin1Char('_');
}

class Splitter final
{
public:
    explicit Splitter(const QString& script)
        : m_script(script)
    {}

    QStringList run()
    {
        const qsizetype kLength = m_script.size();
        qsizetype i = 0;
        while (i < kLength) {
            const QChar c = m_script.at(i);
            const QChar next = i + 1 < kLength ? m_script.at(i + 1) : QChar();

            if (c == QLatin1Char('-') && next == QLatin1Char('-')) {
                i = skipUntil(i, QStringLiteral("\n"));
                m_current += QLatin1Char('\n');
                continue;
            }
            if (c == QLatin1Char('/') && next == QLatin1Char('*')) {
                i = skipUntil(i + 2, QStringLiteral("*/"));
                m_current += QLatin1Char(' ');
                continue;
            }
            if (c == QLatin1Char('\'') || c == QLatin1Char('"') || c == QLatin1Char('`')) {
                i = copyQuoted(i, c);
                continue;
            }
            if (c == QLatin1Char('[')) {
                i = copyQuoted(i, QLatin1Char(']'));
                continue;
            }
            if (isIdentifierChar(c)) {
                i = copyWord(i);
                continue;
            }
            if (c == QLatin1Char(';')) {
                m_current += c;
                ++i;
                if (!m_isTrigger || (m_sawBegin && m_depth == 0)) {
                    flush();
                }
                continue;
            }

            m_current += c;
            ++i;
        }
        flush();
        return m_statements;
    }

private:
    qsizetype skipUntil(qsizetype from, const QString& terminator) const
    {
        const qsizetype kPos = m_script.indexOf(terminator, from);
        return kPos < 0 ? m_script.size() : kPos + terminator.size();
    }

    qsizetype copyQuoted(qsizetype from, QChar closing)
    {
        qsizetype i = from;
        m_current += m_script.at(i++);
        while (i < m_script.size()) {
            const QChar c = m_script.at(i++);
            m_current += c;
            if (c == closing) {
                // Doubled quote is an escaped quote inside the literal.
                if (i < m_script.size() && m_script.at(i) == closing && closing != QLatin1Char(']')) {
                    m_current += m_script.at(i++);
                    continue;
                }
                break;
            }
        }
        return i;
    }

    qsizetype copyWord(qsizetype from)
    {
        qsizetype i = from;
        while (i < m_script.size() && isIdentifierChar(m_script.at(i))) {
            ++i;
        }
        const QString kWord = m_script.mid(from, i - from);
        m_current += kWord;
        onWord(kWord.toUpper());
        return i;
    }

    void onWord(const QString& word)
    {
        m_words << word;
        if (m_words.size() <= 4 && word == QLatin1String("TRIGGER") && m_words.first() == QLatin1String("CREATE")) {
            m_isTrigger = true;
        }
        if (!m_isTrigger) {
            return;
        }
        if (word == QLatin1String("BEGIN")) {
            m_sawBegin = true;
            ++m_depth;
        } else if (word == QLatin1String("CASE") && m_sawBegin) {
            ++m_depth;
        } else if (word == QLatin1String("END") && m_sawBegin) {
            --m_depth;
        }
    }

    void flush()
    {
        const QString kStatement = m_current.trimmed();
        if (!kStatement.isEmpty() && kStatement != QLatin1String(";")) {
            m_statements << kStatement;
        }
        m_current.clear();
        m_words.clear();
        m_isTrigger = false;
        m_sawBegin = false;
        m_depth = 0;
    }

    const QString& m_script;
    QStringList m_statements;
    QString m_current;
    QStringList m_words;
    bool m_isTrigger = false;
    bool m_sawBegin = false;
    int m_depth = 0;
};

} // namespace

QStringList splitStatements(const QString& script)
{
    return Splitter(script).run();
}

} // namespace SqlScript
