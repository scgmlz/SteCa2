// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      core/data/experiment.h
//! @brief     Defines class Experiment
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include "typ/angles.h"
#include "typ/array2d.h"
#include "typ/curve.h"
#include "typ/variant.h"

class Session;
class Metadata;
class Measurement;
class DataSequence;
class Experiment;

class Experiment final : public vec<QSharedPointer<DataSequence>> {
public:
    Experiment();

    void appendHere(QSharedPointer<DataSequence>);

    size2d imageSize() const;

    qreal avgMonitorCount() const;
    qreal avgDeltaMonitorCount() const;
    qreal avgDeltaTime() const;

    Range const& rgeGma(Session const&) const;
    Range const& rgeFixedInten(Session const&, bool trans, bool cut) const;

    Curve avgCurve(Session const&) const;

    void invalidateAvgMutables() const;

private:
    QSharedPointer<DataSequence> combineAll() const;
    qreal calcAvgMutable(qreal (DataSequence::*avgMth)() const) const;

    // computed on demand (NaNs or emptiness indicate yet unknown values)
    mutable qreal avgMonitorCount_, avgDeltaMonitorCount_, avgDeltaTime_;
    mutable Range rgeFixedInten_;
    mutable Range rgeGma_;
    mutable Curve avgCurve_;
};

#endif // EXPERIMENT_H
