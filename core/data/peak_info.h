//  ***********************************************************************************************
//
//  Steca: stress and texture calculator
//
//! @file      core/data/peak_info.h
//! @brief     Defines classes PeakInfo, PeakInfos
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
//  ***********************************************************************************************

#ifndef PEAK_INFO_H
#define PEAK_INFO_H

#include "core/raw/metadata.h"
#include "core/typ/range.h"

//! Metadata, peak fit results, and pole figure angles.

class PeakInfo final {
public:
    PeakInfo();
    PeakInfo(const Metadata*, deg alpha, deg beta, Range, float, float /*error*/,
             deg, deg /*error*/, float, float /*error*/);
    PeakInfo(const Metadata*, deg alpha, deg beta, Range);
    PeakInfo(deg alpha, deg beta, Range, float, float /*error*/, deg, deg /*error*/,
             float, float /*error*/);
    PeakInfo(deg alpha, deg beta);
// TODO after PR#78    PeakInfo(const PeakInfo&) = delete;
//    PeakInfo(PeakInfo&&) = default;

    enum class eReflAttr {
        ALPHA,
        BETA,
        GAMMA1,
        GAMMA2,
        INTEN,
        SIGMA_INTEN,
        TTH,
        SIGMA_TTH,
        FWHM,
        SIGMA_FWHM,
        NUM_REFL_ATTR,
    };

    static QStringList dataTags(bool out);
    static QVector<VariantComparator*> dataCmps();

    deg alpha() const { return alpha_; }
    deg beta() const { return beta_; }
    Range rgeGma() const { return rgeGma_; }
    float inten() const { return inten_; }
    float intenError() const { return intenError_; }
    deg tth() const { return tth_; }
    deg tthError() const { return tthError_; }
    float fwhm() const { return fwhm_; }
    float fwhmError() const { return fwhmError_; }
    QVector<QVariant> data() const;

private:
    const Metadata* md_;
    deg alpha_, beta_;
    Range rgeGma_;
    float inten_, intenError_;
    deg tth_, tthError_;
    float fwhm_, fwhmError_;

    static QString const reflStringTag(int attr, bool out);
};


//! A list of PeakInfo's

class PeakInfos : public QVector<PeakInfo> {
public:
    PeakInfos() { clearCache(); }
    void append(const PeakInfo&);
    float averageInten() const;
    const Range& rgeInten() const;
    void get4(const int idxX, const int idxY,
              QVector<double>& xs, QVector<double>& ys,
              QVector<double>& ysLow, QVector<double>& ysHig) const;
private:
    void clearCache();
    mutable float avgInten_;
    mutable Range rgeInten_;
};

#endif // PEAK_INFO_H
