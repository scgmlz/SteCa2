// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/panels/controls_detector.h
//! @brief     Defines class ControlsDetector
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef CONTROLS_DETECTOR_H
#define CONTROLS_DETECTOR_H

#include "gui/base/new_q.h"

//! A widget with controls to view and change the detector geometry.

class ControlsDetector : public QWidget {
public:
    ControlsDetector();
private:
    void toSession();
    void fromSession();

    QDoubleSpinBox *detDistance_, *detPixelSize_;
    QSpinBox *beamOffsetI_, *beamOffsetJ_;
    QSpinBox *cutLeft_, *cutTop_, *cutRight_, *cutBottom_;
};

#endif // CONTROLS_DETECTOR_H
