// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      gui/thehub.h
//! @brief     Defines class TheHub
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef THEHUB_H
#define THEHUB_H

#include "models.h"
#include "signalling.h" // defines base class TheHubSignallingBase
#include "calc/calc_lens.h"
#include "calc/calc_reflection_info.h"

class QAction;

extern class gui::TheHub* gHub;

namespace gui {

class TheHub : public TheHubSignallingBase {
public:
    TheHub();

    static uint constexpr MAX_POLYNOM_DEGREE = 4;

    QAction *trigger_about, *trigger_online, *trigger_checkUpdate, *trigger_quit, *toggle_viewStatusbar, *toggle_viewFiles, *toggle_viewDatasets,
        *toggle_viewDatasetInfo, *trigger_viewReset,
#ifndef Q_OS_OSX // Mac has its own
        *toggle_fullScreen,
#endif
        *trigger_loadSession, *trigger_saveSession, *trigger_clearSession, *trigger_addFiles, *trigger_removeFile, *toggle_enableCorr, *trigger_remCorr,
        *trigger_rotateImage, *toggle_mirrorImage, *toggle_linkCuts, *toggle_showOverlay, *toggle_stepScale, *toggle_showBins,
        *toggle_fixedIntenImage, *toggle_fixedIntenDgram, *toggle_combinedDgram, *toggle_selRegions, *toggle_showBackground,
        *trigger_clearBackground, *trigger_clearReflections, *trigger_addReflection, *trigger_remReflection, *trigger_outputPolefigures,
        *trigger_outputDiagrams, *trigger_outputDiffractograms;

    // modifying methods:
    void removeFile(uint);
    void clearSession();
    void sessionFromFile(QFileInfo const&) THROWS;
    void addGivenFile(rcstr filePath) THROWS;
    void addGivenFiles(QStringList const& filePaths) THROWS;
    void collectDatasetsFromFiles(uint_vec, pint);
    void collectDatasetsFromFiles(uint_vec);
    void combineDatasetsBy(pint);
    void setCorrFile(rcstr filePath) THROWS;
    void tryEnableCorrection(bool);
    void setImageCut(bool isTopOrLeft, bool linked, typ::ImageCut const&);
    void setGeometry(preal detectorDistance, preal pixSize, typ::IJ const& midPixOffset);
    void setGammaRange(typ::Range const&);

    void setBgRanges(typ::Ranges const&);
    void addBgRange(typ::Range const&);
    void remBgRange(typ::Range const&);
    void setBgPolyDegree(uint);

    void setIntenScaleAvg(bool, preal);
    void setNorm(eNorm);
    void setFittingTab(eFittingTab);

    void setPeakFunction(QString const&);
    void addReflection(QString const&);
    void remReflection(uint);

    // const methods:
    bool isFixedIntenImageScale() const { return isFixedIntenImageScale_; }
    bool isFixedIntenDgramScale() const { return isFixedIntenDgramScale_; }
    bool isCombinedDgram() const { return isCombinedDgram_; }

    calc::shp_DatasetLens datasetLens(data::Dataset const&) const;

    typ::Curve avgCurve(data::Datasets const& dss) const;

    void saveSession(QFileInfo const&) const;
    QByteArray saveSession() const;

    pint datasetsGroupedBy() const { return datasetsGroupedBy_; }

    typ::Range collectedDatasetsRgeGma() const;
    typ::ImageCut const& imageCut() const;

    eFittingTab fittingTab() const { return fittingTab_; }

    data::shp_Dataset selectedDataset() const { return selectedDataset_; }

private:
    friend class TheHubSignallingBase;
    bool isFixedIntenImageScale_;
    bool isFixedIntenDgramScale_;
    bool isCombinedDgram_;
    uint_vec collectFromFiles_;
    pint datasetsGroupedBy_ = pint(1);
    eFittingTab fittingTab_ = eFittingTab::NONE;
    data::shp_Dataset selectedDataset_;
    calc::shp_Reflection selectedReflection_;

    void setImageRotate(typ::ImageTransform);
    void setImageMirror(bool);
    void configActions();
    void sessionFromJson(QByteArray const&) THROWS;

public:
    class models::FilesModel* filesModel;
    class models::DatasetsModel* datasetsModel;
    class models::MetadataModel* metadataModel;
    class models::ReflectionsModel* reflectionsModel;
};

} // namespace gui

#endif // THEHUB_H
