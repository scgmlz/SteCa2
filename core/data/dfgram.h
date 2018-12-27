//  ***********************************************************************************************
//
//  Steca: stress and texture calculator
//
//! @file      core/data/dfgram.h
//! @brief     Defines class Dfgram
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
//  ***********************************************************************************************

#ifndef DFGRAM_H
#define DFGRAM_H

#include "core/typ/curve.h"
#include "core/fit/raw_outcome.h"
#include "core/fit/parametric_function.h"
#include "lazy_data.h"

//! A curve, and associated fit parameters.

class Dfgram {
public:
    Dfgram(Curve&& c);
    ~Dfgram();
    Dfgram(const Dfgram&) = delete;
    Dfgram(Dfgram&&) = default;

    void invalidateBg() const;
    void invalidatePeaks() const;
    void invalidatePeakAt(int) const;

    const Curve curve; // not cached here because recompute depends on context (sector vs avg)
    const Fitted& getBgFit() const { return bgFit_.get(this); }
    const Curve& getBgAsCurve() const { return bgAsCurve_.get(this); }
    const Curve& getCurveMinusBg() const { return curveMinusBg_.get(this); }
    const RawOutcome& getRawOutcome(int jP) const { return rawOutcomes_.getget(this,jP); }
    const Fitted& getPeakFit(int jP) const { return peakFits_.getget(this,jP); }
    const Curve& getPeakAsCurve(int jP) const { return peaksAsCurve_.getget(this,jP); }

private:
    mutable lazy_data::Cached<Fitted,const Dfgram*> bgFit_;
    mutable lazy_data::Cached<Curve, const Dfgram*> bgAsCurve_;
    mutable lazy_data::Cached<Curve, const Dfgram*> curveMinusBg_;
    mutable lazy_data::SelfKachingVector<Dfgram,RawOutcome> rawOutcomes_;
    mutable lazy_data::SelfKachingVector<Dfgram,Fitted> peakFits_;
    mutable lazy_data::SelfKachingVector<Dfgram,Curve> peaksAsCurve_;
};

#endif // DFGRAM_H
