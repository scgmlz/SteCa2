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
#include <QProgressBar>
#include <QButtonGroup>

//! Base class for dialogs for saving some output to a file.

class ExportfileDialogfield : public QVBoxLayout {
public:
    ExportfileDialogfield() = delete;
    ExportfileDialogfield(const ExportfileDialogfield&) = delete;
    ExportfileDialogfield(QWidget* parent, bool withTypes, std::function<void(void)> onSave);

    QString path(bool withSuffix, bool withNumber=false);
    QFile* file();
    QString separator() const;
    QProgressBar progressBar;
private:
    QcrLineEdit* dir_;
    QcrLineEdit* file_;
};

#endif // SUBDIALOG_FILE_H
