// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/dialogs/export_dfgram.cpp
//! @brief     Implements class ExportDfgram
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "gui/dialogs/export_dfgram.h"
#include "core/session.h"
#include "gui/base/file_dialog.h"
#include "gui/dialogs/tab_save.h"
#include "gui/mainwin.h"
#include <cmath>
#include <QGroupBox>
#include <QMessageBox>
#include <QProgressBar>

namespace {

// TODO move file saving code to Core
void writeCurve(QTextStream& stream, const Curve& curve, const Cluster* cluster,
                const Range& rgeGma, const QString& separator)
{
    ASSERT(rgeGma.isValid());
    const Metadata* md = cluster->avgeMetadata().data();
    ASSERT(md);
    stream << "Comment: " << md->comment << '\n';
    stream << "Date: " << md->date << '\n';
    stream << "Gamma range min: " << rgeGma.min << '\n';
    stream << "Gamma range max: " << rgeGma.max << '\n';

    for_i (Metadata::numAttributes(true))
        stream << Metadata::attributeTag(i, true) << ": "
               << md->attributeValue(i).toDouble() << '\n';

    stream << "Tth" << separator << "Intensity" << '\n';
    for_i (curve.xs().count())
        stream << curve.x(i) << separator << curve.y(i) << '\n';

    stream.flush(); // not sure whether we need this
}

//! Returns templatedName with '%d' replaced by string representation of num.

//!  The string representation of num has leading zeros, and its number of
//!  digits is determined by the maximum value maxNum.

QString numberedName(const QString& templatedName, int num, int maxNum) {
    if (!templatedName.contains("%d"))
        qFatal("path does not contain placeholder %%d");
    QString ret = templatedName;
    int nDigits = (int)log10((double)maxNum)+1;
    ret.replace("%d", QString("%1").arg(num, nDigits, 10, QLatin1Char('0')));
    qDebug() << "PATH " << templatedName << " -> " << ret;
    return ret;
}

} // namespace


// ************************************************************************** //
//  local class TabDiffractogramsSave
// ************************************************************************** //

//! The main part of ExportDfgram. Extends TabSave by an output content control.

class TabDiffractogramsSave : public TabSave {
public:
    TabDiffractogramsSave();

    bool currentChecked() { return rbCurrent_.isChecked(); }
    bool allSequentialChecked() { return rbAllSequential_.isChecked(); }
    bool allChecked() { return rbAll_.isChecked(); }

private:
    CRadioButton rbCurrent_ {"rbCurrent", "Current diffractogram"};
    CRadioButton rbAllSequential_ {"rbAllSequential", "All diffractograms to numbered files"};
    CRadioButton rbAll_ {"rbAll", "All diffractograms to one file"};
};

TabDiffractogramsSave::TabDiffractogramsSave()
    : TabSave(true)
{
    rbAll_.setChecked(true);

    auto* boxlayout = new QVBoxLayout;
    boxlayout->addWidget(&rbCurrent_);
    boxlayout->addWidget(&rbAllSequential_);
    boxlayout->addWidget(&rbAll_);

    auto* box = new QGroupBox {"Save what"};
    box->setLayout(boxlayout);

    grid_->addWidget(box, grid_->rowCount(), 0, 1, 2);
    grid_->setRowStretch(grid_->rowCount(), 1);
}

// ************************************************************************** //
//  class ExportDfgram
// ************************************************************************** //

ExportDfgram::ExportDfgram()
    : QDialog(gGui)
    , CModal("dgram")
{
    progressBar_ = new QProgressBar;
    tabSave_ = new TabDiffractogramsSave();

    auto* actCancel = new CTrigger("cancel", "Cancel");
    auto* actSave = new CTrigger("save", "Save");

    setModal(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle("Diffractograms");
    progressBar_->hide();

    // internal connections
    connect(actCancel, &QAction::triggered, [this]() { close(); });
    connect(actSave, &QAction::triggered, [this]() { save(); });

    // layout
    auto* hb_bottom = new QHBoxLayout();
    hb_bottom->addWidget(progressBar_);
    hb_bottom->setStretchFactor(progressBar_, 333);
    hb_bottom->addStretch(1);
    hb_bottom->addWidget(new XTextButton(actCancel));
    hb_bottom->addWidget(new XTextButton(actSave));

    auto* vbox = new QVBoxLayout();
    vbox->addWidget(tabSave_);
    vbox->setStretch(vbox->count() - 1, 1);
    vbox->addLayout(hb_bottom);
    setLayout(vbox);

    show();
}

ExportDfgram::~ExportDfgram()
{
    delete tabSave_; // auto deletion would happen in wrong order
}

void ExportDfgram::onCommand(const QStringList&)
{
// TODO get rid of this
}

void ExportDfgram::save()
{
    if (tabSave_->currentChecked())
        saveCurrent();
    else if (tabSave_->allSequentialChecked())
        saveAll(false);
    else if (tabSave_->allChecked())
        saveAll(true);
    else
        qFatal("Invalid call of ExportDfgram::saveDiffractogramOutput");
    close();
}

void ExportDfgram::saveCurrent()
{
    QString path = tabSave_->filePath(true);
    if (path.isEmpty())
        return;
    QFile* file = file_dialog::openFileConfirmOverwrite("file", this, path);
    if (!file)
        return;
    QTextStream stream(file);
    const Cluster* cluster = gSession->dataset().highlight().cluster();
    ASSERT(cluster);
    const Curve& curve = cluster->toCurve();
    if (curve.isEmpty())
        qFatal("curve is empty");
    writeCurve(stream, curve, cluster, cluster->rgeGma(), tabSave_->separator());
}

void ExportDfgram::saveAll(bool oneFile)
{
    const Experiment& expt = gSession->experiment();
    // In one-file mode, start output stream; in multi-file mode, only do prepations.
    QString path = tabSave_->filePath(true, !oneFile);
    if (path.isEmpty())
        return;
    QTextStream* stream = nullptr;
    if (oneFile) {
        QFile* file = file_dialog::openFileConfirmOverwrite("file", this, path);
        if (!file)
            return;
        stream = new QTextStream(file);
    } else {
        // check whether any of the numbered files already exists
        QStringList existingFiles;
        for_i (expt.size()) {
            QString currPath = numberedName(path, i, expt.size()+1);
            if (QFile(currPath).exists())
                existingFiles << QFileInfo(currPath).fileName();
        }
        if (existingFiles.size() &&
            QMessageBox::question(this, existingFiles.size()>1 ? "Files exist" : "File exists",
                                  "Overwrite files " + existingFiles.join(", ") + " ?") !=
            QMessageBox::Yes)
            return;
    }
    Progress progress(expt.size(), progressBar_);
    int picNum = 0;
    int fileNum = 0;
    int nSlices = gSession->gammaSelection().numSlices();
    for (const Cluster* cluster : expt.clusters()) {
        ++picNum;
        progress.step();
        qreal normFactor = cluster->normFactor();
        for (int i=0; i<qMax(1,nSlices); ++i) {
            if (!oneFile) {
                QFile* file = new QFile(numberedName(path, ++fileNum, expt.size()+1));
                if (!file->open(QIODevice::WriteOnly | QIODevice::Text))
                    THROW("Cannot open file for writing: " + path);
                delete stream;
                stream = new QTextStream(file);
            }
            ASSERT(stream);
            const Range gmaStripe = gSession->gammaSelection().slice2range(i);
            const Curve& curve = cluster->toCurve(normFactor, gmaStripe);
            ASSERT(!curve.isEmpty());
            *stream << "Picture Nr: " << picNum << '\n';
            if (nSlices > 1)
                *stream << "Gamma slice Nr: " << i+1 << '\n';
            writeCurve(*stream, curve, cluster, gmaStripe, tabSave_->separator());
        }
    }
    delete stream;
}
