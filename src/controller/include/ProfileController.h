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
    void setDependencies(const QSharedPointer<ProductRepository>& products,
                         const QSharedPointer<DatabaseHandler>& database);

    /*!
     * \brief Инициализирует список купленных продуктов
     * \param purchasedListView - список купленных продуктов
     * \param clientNameLabel - лейбл с именем клиента
     * \param purchasedGroupBox - групповая коробка для списка купленных продуктов
     */
    void initialize(QListView* purchasedListView,
                    QLabel* clientNameLabel,
                    QGroupBox* purchasedGroupBox);

    /*!
     * \brief Отображает профиль пользователя
     * \param userId - ID пользователя
     * \param userName - имя пользователя
     */
    void showProfile(int userId, const QString& userName);
    /*!
     * \brief Возвращает список купленных продуктов
     * \param userId - ID пользователя
     * \returns Список купленных продуктов
     */
    QList<ProductRepository::ProductKey> getPurchasedProductKeys(int userId) const;

private:
    /*!
     * \brief Возвращает список купленных продуктов
     * \param userId - ID пользователя
     * \returns Список купленных продуктов
     */
    QList<ProductRepository::ProductKey> getPurchasedProducts(int userId) const;

    /*!
     * \brief Конфигурирует список купленных продуктов
     */
    void configurePurchasedListView();

    /*!
     * \brief Обновляет список купленных продуктов
     * \param userId - ID пользователя
     */
    void updatePurchasedList(int userId);

    QPointer<QListView> m_purchasedListView;
    QPointer<QLabel> m_clientNameLabel;
    QPointer<QGroupBox> m_purchasedGroupBox;

    QSharedPointer<ProductRepository> m_products;
    QSharedPointer<DatabaseHandler> m_database;
    QScopedPointer<ProductListModel> m_purchasedModel;
    QScopedPointer<ProductCardDelegate> m_purchasedDelegate;
};

#endif // PROFILE_CONTROLLER_H
