#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimeEdit>
#include <QCheckBox>
#include <QCryptographicHash>
#include <QGraphicsDropShadowEffect>

#include "AppServices.h"
#include "PurchaseMethodsController.h"
#include "SettingsForm.h"
#include "UserInfo.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setDarkThemeEnabled(bool enabled);

    /*!
     * \brief Обновляет информацию о пользователе
     * \param user - информация о пользователе
     * \param parent - родительский виджет
     */
    void updateUser(const UserInfo& user, QWidget* parent);

private slots:
    void onLoginClicked();

    void onRegistrationClicked();

    void onLogoutClicked();

    void onNextLeftClicked();

    void onNextRightClicked();

    void onBackClicked();

    void onToPayClicked();

    void onInfoClicked();

    void onTestDriveClicked();

    void onOrderClicked();

    void onNotificationsClicked();

    void onSettingsClicked();

    void onProfileClicked();

    void onSortByColorClicked();

    void onSearchClicked();

    void onSortByTypeClicked();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void applyThemeIcons();

    Ui::MainWindow *m_ui;

    ProductInfo m_currentProduct;                      ///< Информация о текущем товаре, который выбрал пользователь
    int m_currentColorIndex = 0;                      ///< Индекс текущего цвета у товара, который выбрал пользователь

    QScopedPointer<AppServices> m_services;             ///< сервисы приложения (DB, модели, контроллеры)

    QScopedPointer<SettingsForm> m_settingsForm;       ///< Форма настроек
    QScopedPointer<PurchaseMethodsController> m_purchaseMethodsController; ///< Контроллер методов покупки

    /*!
     * \brief Инициализация зависимостей
     */
    void buildDependencies();
    
    /*!
     * \brief Инициализация бокового меню
     */
    void setupFloatingMenu();

    /*!
     * \brief Обновление позиции плавающего меню при изменении размера окна
     */
    void updateFloatingMenuPosition();

    /*!
     * \brief Обновляет размеры изображения и положение стрелок на странице автомобиля
     */
    void updatePersonalPageLayout();

    /*!
     * \brief Обработка выбора услуги
     * \param ok - флаг, указывающий, была ли выбрана услуга
     * \param selected_type - тип услуги
     * \param selected_color - цвет услуги
     */
    void selectionProcessing(const bool kOk, const QStringView kSelectedType, const QStringView kSelectedColor = QStringView());

    /*!
     * \brief Загружает доступные услуги для клиента в ScrollArea на странице пользователя
     */
    void setupServicesScrollArea();

    /*!
     * \brief Настраивает страницу с информацей о конкретном автомобиле
     * \param const ProductInfo& - характеристики, информация об автомобиле
     * \param QList<ProductInfo>& - список с доступными цветами конкретного автомобиля
     */
    void showProductOnPersonalPage(const ProductInfo&, QList<ProductInfo>&);

};

#endif // MAINWINDOW_H
