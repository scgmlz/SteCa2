//  ***********************************************************************************************
//
//  Steca: stress and texture calculator
//
//! @file      gui/panels/tab_bigtable.cpp
//! @brief     Implements class BigtableTab, and its dependences
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
//  ***********************************************************************************************

#include "gui/panels/tab_bigtable.h"
#include "core/session.h"
#include "gui/actions/triggers.h"
#include "gui/mainwin.h"
#include "gui/view/bigtable.h"
#include <QButtonGroup>
//#include "qcr/base/debug.h"

//  ***********************************************************************************************
//! @class ColumnSelector (local scope)

//! A row of controls for choosing which data columns are to be displayed in a TabTable.

class ColumnSelector : public QcrWidget {
public:
    ColumnSelector();
private:
    std::vector<QcrCheckBox*> showCols_;
    void setOne(int pos, bool on);
    void setAll(bool on);
    using eReflAttr = PeakInfo::eReflAttr;
};

ColumnSelector::ColumnSelector()
    : QcrWidget("ColumnSelector")
{
    auto* box = new QVBoxLayout;

    auto* trigAll   = new QcrTrigger {"bigtabAll", "select all columns", ":/icon/All"};
    auto* trigClear = new QcrTrigger {"bigtabClear", "unselect all columns", ":/icon/clear"};

    trigAll  ->setTriggerHook([this](){ setAll(true);  });
    trigClear->setTriggerHook([this](){ setAll(false); });

    auto* hb = new QHBoxLayout;
    hb->addSpacing(4);
    hb->addStretch(1);
    hb->addWidget(new QcrIconTriggerButton(trigAll));
    hb->addWidget(new QcrIconTriggerButton(trigClear));
    hb->addSpacing(4);
    box->addLayout(hb);
    box->addSpacing(8);

    const QStringList& headers = PeakInfo::dataTags(false);
    for (int i=0; i<headers.count(); ++i)
        gSession->params.bigMetaSelection.vec.push_back({true});
    showCols_.resize(headers.count());
    for (int i=0; i<showCols_.size(); ++i) {
        showCols_[i] = new QcrCheckBox(
            "cb"+QString::number(i), headers[i], &gSession->params.bigMetaSelection.vec[i]);
        box->addWidget(showCols_[i]);
    }
    setLayout(box);
}

void ColumnSelector::setOne(int pos, bool on)
{
    gSession->params.bigMetaSelection.vec.at(pos).pureSetVal(on);
}

void ColumnSelector::setAll(bool on)
{
    for (auto& col : gSession->params.bigMetaSelection.vec)
        col.pureSetVal(on);
}

//  ***********************************************************************************************
//! @class BigtableTab

BigtableTab::BigtableTab()
    : QcrWidget("BigtableTab")
{
    auto bigtableView = new BigtableView;

    auto* colSelBox = new QcrScrollArea("colSelBox");
    colSelBox->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    colSelBox->setWidget(new ColumnSelector());

    auto* buttonBox = new QHBoxLayout;
    buttonBox->addStretch(1);
    buttonBox->addWidget(new QcrIconTriggerButton {&gGui->triggers->spawnTable});
    buttonBox->addWidget(new QcrIconTriggerButton {&gGui->triggers->exportBigtable});

    auto* sideBox = new QVBoxLayout;
    sideBox->addWidget(colSelBox);
    sideBox->addLayout(buttonBox);
    sideBox->setStretch(0,1000);

    auto* layout = new QHBoxLayout;
    layout->addWidget(bigtableView);
    layout->addLayout(sideBox);
    layout->setStretch(0,1000);
    setLayout(layout);

    setRemake([=](){bigtableView->refresh();});
}
