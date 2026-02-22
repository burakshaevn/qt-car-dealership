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
#include "user.h"

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
    void SetDarkThemeEnabled(bool enabled);

    /*!
     * \brief Обновляет информацию о пользователе
     * \param user - информация о пользователе
     * \param parent - родительский виджет
     */
    void UpdateUser(const UserInfo& user, QWidget* parent);

private slots:
    void OnLoginClicked();

    void OnRegistrationClicked();

    void OnLogoutClicked();

    void OnNextLeftClicked();

    void OnNextRightClicked();

    void OnBackClicked();

    void OnToPayClicked();

    void OnInfoClicked();

    void OnTestDriveClicked();

    void OnOrderClicked();

    void OnNotificationsClicked();

    void OnSettingsClicked();

    void OnProfileClicked();

    void OnSortByColorClicked();

    void OnSearchClicked();

    void OnSortByTypeClicked();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void ApplyThemeIcons();

    Ui::MainWindow *ui;

    ProductInfo m_current_product;                      ///< Информация о текущем товаре, который выбрал пользователь
    int m_current_color_index = 0;                      ///< Индекс текущего цвета у товара, который выбрал пользователь

    QScopedPointer<AppServices> m_services;             ///< сервисы приложения (DB, модели, контроллеры)

    QScopedPointer<SettingsForm> m_settings_form;       ///< Форма настроек
    QScopedPointer<PurchaseMethodsController> m_purchase_methods_controller; ///< Контроллер методов покупки

    /*!
     * \brief Инициализация зависимостей
     */
    void BuildDependencies();
    
    /*!
     * \brief Инициализация бокового меню
     */
    void SetupFloatingMenu();

    /*!
     * \brief Обновление позиции плавающего меню при изменении размера окна
     */
    void UpdateFloatingMenuPosition();

    /*!
     * \brief Обработка выбора услуги
     * \param ok - флаг, указывающий, была ли выбрана услуга
     * \param selected_type - тип услуги
     * \param selected_color - цвет услуги
     */
    void SelectionProcessing(const bool ok, const QStringView selected_type, const QStringView selected_color = QStringView());

    /*!
     * \brief Загружает доступные услуги для клиента в ScrollArea на странице пользователя
     */
    void SetupServicesScrollArea();

    /*!
     * \brief Настраивает страницу с информацей о конкретном автомобиле
     * \param const ProductInfo& - характеристики, информация об автомобиле
     * \param QList<ProductInfo>& - список с доступными цветами конкретного автомобиля
     */
    void ShowProductOnPersonalPage(const ProductInfo&, QList<ProductInfo>&);

};

#endif // MAINWINDOW_H
