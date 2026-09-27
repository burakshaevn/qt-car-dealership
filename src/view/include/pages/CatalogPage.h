#pragma once

#ifndef CATALOG_PAGE_H
#define CATALOG_PAGE_H

#include <QList>
#include <QPair>
#include <QWidget>
#include <optional>

class QAbstractItemModel;
class QButtonGroup;
class QComboBox;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QListView;
class QModelIndex;
class QStackedWidget;
class QTimer;

/*!
 * \brief Catalogue screen: search, body-type chips, colour filter and a card grid.
 *
 * The page is passive: it exposes the current filter state and emits
 * filtersChanged(); the controller supplies the model.
 */
class CatalogPage final : public QWidget
{
    Q_OBJECT
public:
    using TypeOption = QPair<int, QString>;

    explicit CatalogPage(QWidget* parent = nullptr);

    void setModel(QAbstractItemModel* model);
    void setTypes(const QList<TypeOption>& types);
    void setColors(const QStringList& colors, const QString& current = {});
    void setResultCount(int count);
    void resetFilters(const QString& color = {});

    [[nodiscard]] QString searchText() const;
    [[nodiscard]] std::optional<int> typeId() const;
    [[nodiscard]] QString color() const;
    [[nodiscard]] QListView* view() const { return m_view; }

signals:
    void filtersChanged();
    void productActivated(const QModelIndex& index);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void updateGrid();

    QLineEdit* m_search = nullptr;
    QHBoxLayout* m_chips = nullptr;
    QButtonGroup* m_typeGroup = nullptr;
    QComboBox* m_color = nullptr;
    QLabel* m_count = nullptr;
    QStackedWidget* m_content = nullptr;
    QListView* m_view = nullptr;
    QTimer* m_searchDebounce = nullptr;
    bool m_updating = false;
};

#endif // CATALOG_PAGE_H
