#pragma once

#ifndef LOGIN_PAGE_H
#define LOGIN_PAGE_H

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;

/*!
 * \brief Sign-in screen: brand hero on the left, credentials card on the right.
 */
class LoginPage final : public QWidget
{
    Q_OBJECT
public:
    explicit LoginPage(QWidget* parent = nullptr);

    [[nodiscard]] QString login() const;
    [[nodiscard]] QString password() const;

    void setHeroImage(const QString& path);
    void showError(const QString& message);
    void clear();
    void setBusy(bool busy);

signals:
    void loginRequested(const QString& login, const QString& password);
    void registrationRequested();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void submit();
    void updateHero();

    QLabel* m_hero = nullptr;
    QString m_heroPath;
    QLineEdit* m_login = nullptr;
    QLineEdit* m_password = nullptr;
    QLabel* m_error = nullptr;
    QPushButton* m_submit = nullptr;
};

#endif // LOGIN_PAGE_H
