#pragma once

#ifndef TOP_BAR_H
#define TOP_BAR_H

#include <QFrame>
#include <QHash>

class QButtonGroup;
class QHBoxLayout;
class QLabel;
class QPushButton;

/*!
 * \brief Horizontal customer navigation: wordmark, text tabs, user and account actions.
 *
 * API mirrors NavigationSidebar so that the shell can treat both the same way.
 */
class TopBar final : public QFrame
{
    Q_OBJECT
public:
    explicit TopBar(QWidget* parent = nullptr);

    void addSection(const QString& id, const QString& title);
    void setCurrentSection(const QString& id);
    void setBadge(const QString& id, int count);
    void setUser(const QString& name, const QString& subtitle);

signals:
    void sectionSelected(const QString& id);
    void settingsRequested();
    void logoutRequested();

private:
    void refreshBrand();

    QLabel* m_logo = nullptr;
    QLabel* m_wordmark = nullptr;
    QHBoxLayout* m_tabs = nullptr;
    QButtonGroup* m_group = nullptr;
    QHash<QString, QPushButton*> m_buttons;
    QHash<QString, QLabel*> m_badges;
    QLabel* m_user = nullptr;
};

#endif // TOP_BAR_H
