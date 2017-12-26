// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      gui/actions.h
//! @brief     Defines classes Action, TriggerAction, ToggleAction, Actions
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef ACTIONS_H
#define ACTIONS_H

#include "refhub.h"
#include <QAction>

namespace gui {

class Action : public QAction {
public:
    Action(rcstr text, QObject*);

    Action& text(rcstr);
    Action& tip(rcstr);
    Action& key(QKeySequence);
    Action& icon(rcstr);

    virtual Action& alt(rcstr text2);
};

class TriggerAction : public Action {
public:
    TriggerAction(rcstr text, QObject* = nullptr);
};

class ToggleAction : public Action {
public:
    ToggleAction(rcstr text, QObject* = nullptr);

    Action& alt(rcstr text2);

protected:
    str text1_, text2_;
};

class Actions : protected RefHub {
public:
    Actions(TheHub&);

    Action *about, *online, *checkUpdate, *quit, *viewStatusbar, *viewFiles, *viewDatasets,
        *viewDatasetInfo, *viewReset,
#ifndef Q_OS_OSX // Mac has its own
        *fullScreen,
#endif
        *loadSession, *saveSession, *clearSession, *addFiles, *remFile, *enableCorr, *remCorr,
        *rotateImage, *mirrorImage, *linkCuts, *showOverlay, *stepScale, *showBins,
        *fixedIntenImage, *fixedIntenDgram, *combinedDgram, *selRegions, *showBackground,
        *clearBackground, *clearReflections, *addReflection, *remReflection, *outputPolefigures,
        *outputDiagrams, *outputDiffractograms;

private:
    Action& trg(Action*& action, rcstr text); //! < trigger
    Action& tgl(Action*& action, rcstr text); //! < toggle
};

} // namespace gui

#endif // ACTIONS_H
