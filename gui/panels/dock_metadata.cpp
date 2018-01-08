// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/panels/dock_metadata.cpp
//! @brief     Implements class ViewMetadata
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "dock_metadata.h"
#include "models.h"
#include "widgets/tree_views.h" // inheriting from
#include "thehub.h"

class MetadataView : public ListView {
public:
    MetadataView();

protected:
    int sizeHintForColumn(int) const;

private:
    MetadataModel* model() const { return static_cast<MetadataModel*>(ListView::model()); }
};

MetadataView::MetadataView() : ListView() {
    setModel(gHub->metadataModel);
    debug::ensure(dynamic_cast<MetadataModel*>(ListView::model()));
    connect(this, &MetadataView::clicked, [this](QModelIndex const& index) {
        model()->flipCheck(to_u(index.row()));
        gHub->suiteModel->showMetaInfo(model()->rowsChecked()); // REVIEW signal instead?
    });
}

int MetadataView::sizeHintForColumn(int col) const {
    switch (col) {
    case MetadataModel::COL_CHECK:
        return fontMetrics().width('m');
    default:
        return ListView::sizeHintForColumn(col);
    }
}


ViewMetadata::ViewMetadata() : DockWidget("Metadata", "dock-metadata") {
    box_->addWidget((metadataView_ = new MetadataView()));
}
