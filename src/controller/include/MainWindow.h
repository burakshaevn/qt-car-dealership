#pragma once

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QScopedPointer>

#include "ProductListModel.h"

class AdminController;
class AdminPage;
class AppServices;
class AuthController;
class CatalogController;
class CatalogPage;
class LoginPage;
class NavigationSidebar;
class NotificationsController;
class NotificationsPage;
class ProductPage;
class ProfilePage;
class QStackedWidget;
class TopBar;
class RequestController;

/*!
 * \brief Application shell: login screen, or sidebar + content pages after sign-in.
 *
 * The window only wires pages to controllers; it contains no data access code.
 */
class MainWindow final : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(AppServices& services, QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    void buildUi();
    void onLogin(const QString& login, const QString& password);
    void onRegistration();
    void onLogout();
    void startCustomerSession();
    void startAdminSession();

    void navigate(const QString& section);
    void showProduct(const ProductInfo& product);
    void refreshProfile();
    void refreshBadges();
    void openSettings();

    AppServices& m_services;

    QStackedWidget* m_root = nullptr;      ///< login | workspace
    LoginPage* m_loginPage = nullptr;
    QWidget* m_workspace = nullptr;
    NavigationSidebar* m_sidebar = nullptr; ///< admin index
    TopBar* m_topBar = nullptr;             ///< customer navigation
    QStackedWidget* m_pages = nullptr;
    CatalogPage* m_catalogPage = nullptr;
    ProductPage* m_productPage = nullptr;
    ProfilePage* m_profilePage = nullptr;
    NotificationsPage* m_notificationsPage = nullptr;
    AdminPage* m_adminPage = nullptr;

    QScopedPointer<AuthController> m_auth;
    QScopedPointer<RequestController> m_requests;
    QScopedPointer<CatalogController> m_catalog;
    QScopedPointer<NotificationsController> m_notifications;
    QScopedPointer<AdminController> m_admin;
    ProductListModel m_purchasedModel;
};

#endif // MAIN_WINDOW_H
