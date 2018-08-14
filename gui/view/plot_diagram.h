//  ***********************************************************************************************
//
//  Steca: stress and texture calculator
//
//! @file      gui/view/plot_diagram.h
//! @brief     Defines class PlotDiagram
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
//  ***********************************************************************************************

#ifndef PLOT_DIAGRAM_H
#define PLOT_DIAGRAM_H

#include "core/calc/peak_info.h"
#include "qcr/widgets/views.h"

//! Tab in DiagramsFrame, to display a plot of something against something.

class PlotDiagram : public QWidget {
public:
    PlotDiagram();
    PlotDiagram(int, int);
    void refresh();
private:
    void erase();
    class QCustomPlot* plotter_;
    class QCPGraph *graph_, *graphLo_, *graphUp_;
};

#endif // PLOT_DIAGRAM_H
