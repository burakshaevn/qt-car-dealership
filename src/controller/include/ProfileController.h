#pragma once

#ifndef PROFILE_CONTROLLER_H
#define PROFILE_CONTROLLER_H

#include <QObject>
#include <QPointer>
#include <QSharedPointer>
#include "ProductRepository.h"

class QLabel;
class QListView;
class QGroupBox;
class DatabaseHandler;
class ProductRepository;
class ProductListModel;
class ProductCardDelegate;

/*!
 * \brief Класс, предоставляющий сервисы для работы с профилем пользователя
 */
class ProfileController : public QObject
{
    Q_OBJECT
public:
    explicit ProfileController(QObject* parent = nullptr);

    /*!
     * \brief Устанавливает зависимости
     * \param products - указатель на объект Products
     * \param database - указатель на объект DatabaseHandler
     */
    void SetDependencies(const QSharedPointer<ProductRepository>& products,
                         const QSharedPointer<DatabaseHandler>& database);

    /*!
     * \brief Инициализирует список купленных продуктов
     * \param purchasedListView - список купленных продуктов
     * \param clientNameLabel - лейбл с именем клиента
     * \param purchasedGroupBox - групповая коробка для списка купленных продуктов
     */
    void Initialize(QListView* purchasedListView,
                    QLabel* clientNameLabel,
                    QGroupBox* purchasedGroupBox);

    /*!
     * \brief Отображает профиль пользователя
     * \param userId - ID пользователя
     * \param userName - имя пользователя
     */
    void ShowProfile(int userId, const QString& userName);
    /*!
     * \brief Возвращает список купленных продуктов
     * \param userId - ID пользователя
     * \returns Список купленных продуктов
     */
    QList<ProductRepository::ProductKey> GetPurchasedProductKeys(int userId) const;

private:
    /*!
     * \brief Возвращает список купленных продуктов
     * \param userId - ID пользователя
     * \returns Список купленных продуктов
     */
    QList<ProductRepository::ProductKey> GetPurchasedProducts(int userId) const;

    /*!
     * \brief Конфигурирует список купленных продуктов
     */
    void ConfigurePurchasedListView();

    /*!
     * \brief Обновляет список купленных продуктов
     * \param userId - ID пользователя
     */
    void UpdatePurchasedList(int userId);

    QPointer<QListView> purchased_list_view_;
    QPointer<QLabel> client_name_label_;
    QPointer<QGroupBox> purchased_group_box_;

    QSharedPointer<ProductRepository> products_;
    QSharedPointer<DatabaseHandler> database_;
    QScopedPointer<ProductListModel> purchased_model_;
    QScopedPointer<ProductCardDelegate> purchased_delegate_;
};

#endif // PROFILE_CONTROLLER_H
