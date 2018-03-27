// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/view/bigtable.h
//! @brief     Defines classes DataView
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef DATA_TABLE_H
#define DATA_TABLE_H

#include "core/typ/variant.h"
#include "gui/base/model_view.h"
#include <memory>

//! Model for the DataView view.

class DataModel : public TableModel {
public:
    DataModel();
    DataModel(DataModel&) = delete;

    void refresh();
    void moveColumn(int from, int to);
    void setSortColumn(int);
    void sortData();
    const row_t& row(int);

    int columnCount() const final { return numCols_ + 1; }
    int rowCount() const final { return rows_.count(); }
    int highlighted() const final { return 0; } // unused
    void setHighlight(int i) final { ; }        // unused

    QVariant data(const QModelIndex&, int) const;
    QVariant headerData(int, Qt::Orientation, int) const;

private:
    int numCols_;
    int sortColumn_ {-1};

    QStringList headers_;
    QVector<int> colIndexMap_;
    cmp_vec cmpFunctions_;

    struct numRow {
        typedef const numRow& rc;
        numRow() : n(0), row() {}
        numRow(int n_, const row_t& row_) : n(n_), row(row_) {}
        int n;
        row_t row;
    };

    QVector<numRow> rows_;
};

//! A data table view, for use in the 'Points' tab of an output Frame.

class DataView : public TableView {
public:
    DataView();
    DataView(DataView&) = delete;

    void refresh();

    void toFile(QTextStream& stream, const QString& separator) const;

private:
    void updateShownColumns();
    void keyPressEvent(QKeyEvent *event);
    DataModel* model() const { return dynamic_cast<DataModel*>(model_); }

    QString exportSelection() const;
    QStringList outHeaders() const { return outHeaders_; }

    QStringList outHeaders_;
};

#endif // DATA_TABLE_H
