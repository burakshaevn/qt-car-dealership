#pragma once

#ifndef NAVIGATION_SIDEBAR_H
#define NAVIGATION_SIDEBAR_H

#include <QFrame>
#include <QHash>

class QButtonGroup;
class QLabel;
class QPushButton;
class QVBoxLayout;

/*!
 * \brief Left navigation rail: brand, sections, user card and sign-out.
 *
 * Sections are registered by the owner with addSection(); the sidebar keeps
 * the checked state exclusive and emits sectionSelected(id).
 */
class NavigationSidebar final : public QFrame
{
    Q_OBJECT
public:
    explicit NavigationSidebar(QWidget* parent = nullptr);

    void addCaption(const QString& text);
    void addSection(const QString& id, const QString& title, const QString& icon);
    void setCurrentSection(const QString& id);
    void setBadge(const QString& id, int count);
    void setUser(const QString& name, const QString& subtitle);

signals:
    void sectionSelected(const QString& id);
    void settingsRequested();
    void logoutRequested();

private:
    QVBoxLayout* m_sections = nullptr;
    QButtonGroup* m_group = nullptr;
    QHash<QString, QPushButton*> m_buttons;
    QHash<QString, QLabel*> m_badges;
    QLabel* m_avatar = nullptr;
    QLabel* m_userName = nullptr;
    QLabel* m_userSubtitle = nullptr;
};

#endif // NAVIGATION_SIDEBAR_H
