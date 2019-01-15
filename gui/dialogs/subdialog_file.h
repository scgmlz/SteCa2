//  ***********************************************************************************************
//
//  Steca: stress and texture calculator
//
//! @file      gui/dialogs/subdialog_file.h
//! @brief     Defines class ExportfileDialogfield
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
//  ***********************************************************************************************

#ifndef SUBDIALOG_FILE_H
#define SUBDIALOG_FILE_H

#include "qcr/widgets/controls.h"
#include "qcr/widgets/modal_dialogs.h"
#include <QButtonGroup>
#include <QGroupBox>
#include <QProgressBar>

class DialogfieldPath;

//! Common part of all export dialogs: choice of file and format.

class ExportfileDialogfield : public QVBoxLayout {
public:
    ExportfileDialogfield() = delete;
    ExportfileDialogfield(const ExportfileDialogfield&) = delete;
    ExportfileDialogfield(
        QcrDialog* _parent, QStringList extensions,
        std::function<void(QFile* file, const QString& format, QcrDialog* parent)> _onSave);

    QString format() const { return saveFmt; }
    DialogfieldPath* pathField;
    QProgressBar progressBar;
    QString path(bool withSuffix, bool withNumber);
private:
    QcrDialog* parent;
    QString saveFmt {"dat"}; //!< setting: default format for data export
    std::function<void(QFile* file, const QString& format, QcrDialog* parent)> onSave;
};

#endif // SUBDIALOG_FILE_H
