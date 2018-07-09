//  ***********************************************************************************************
//
//  Steca: stress and texture calculator
//
//! @file      gui/panels/controls_peakfits.h
//! @brief     Defines class ControlsPeakfits
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
//  ***********************************************************************************************

#ifndef CONTROLS_PEAKFITS_H
#define CONTROLS_PEAKFITS_H

#include "qcr/widgets/controls.h"
#include "qcr/widgets/views.h"

//! A widget with controls to view and change the detector geometry.

class ControlsPeakfits : public QcrWidget {
public:
    ControlsPeakfits();
private:
    QHBoxLayout topControls_;
    QcrComboBox comboReflType_;
    void hideEvent(QHideEvent*) final;
    void showEvent(QShowEvent*) final;
};

#endif // CONTROLS_PEAKFITS_H
