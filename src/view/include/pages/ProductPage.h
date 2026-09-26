#pragma once

#ifndef PRODUCT_PAGE_H
#define PRODUCT_PAGE_H

#include <QList>
#include <QWidget>

#include "ProductRepository.h"

class QButtonGroup;
class QHBoxLayout;
class FlowLayout;
class QLabel;
class QPushButton;

/*!
 * \brief Model details: large image with colour carousel, specs and actions.
 */
class ProductPage final : public QWidget
{
    Q_OBJECT
public:
    explicit ProductPage(QWidget* parent = nullptr);

    /// Shows \a variants of one model (one per colour) and selects \a current.
    void setVariants(const QList<ProductInfo>& variants, int current = 0);
    [[nodiscard]] ProductInfo currentProduct() const;

signals:
    void backRequested();
    void checkoutRequested(const ProductInfo& product);
    void orderRequested(const ProductInfo& product);
    void testDriveRequested(const ProductInfo& product);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void select(int index);
    void step(int delta);
    void updateImage();

    QList<ProductInfo> m_variants;
    int m_current = 0;

    QLabel* m_image = nullptr;
    QPushButton* m_prev = nullptr;
    QPushButton* m_next = nullptr;
    QLabel* m_counter = nullptr;
    QLabel* m_type = nullptr;
    QLabel* m_name = nullptr;
    QLabel* m_availability = nullptr;
    QLabel* m_price = nullptr;
    QLabel* m_description = nullptr;
    QLabel* m_colorValue = nullptr;
    QLabel* m_trimValue = nullptr;
    QLabel* m_stockValue = nullptr;
    FlowLayout* m_swatches = nullptr;
    QButtonGroup* m_swatchGroup = nullptr;
    QPushButton* m_checkout = nullptr;
    QPushButton* m_order = nullptr;
    QPushButton* m_testDrive = nullptr;
};

#endif // PRODUCT_PAGE_H
