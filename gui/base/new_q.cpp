// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/base/new_q.cpp
//! @brief     Implements functions that return new Qt objects
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "gui/base/new_q.h"
#include "core/def/numbers.h"
#include <QApplication> // for qApp for new Action
#include <QDebug>
#include <QMessageBox>
#include <QtGlobal> // to define Q_OS_WIN

namespace {

static void setWidth(QWidget* w, int ndigits, bool withDot) {
    int width = ndigits;
#ifdef Q_OS_WIN
    width += 1 + (withDot?1:0);
#endif
    w->setMaximumWidth(width * w->fontMetrics().width('m'));
}

} // local methods


QBoxLayout* newQ::HBoxLayout() {
    auto ret = new QHBoxLayout;
    ret->setSpacing(2);
    ret->setMargin(2);
    return ret;
}

QBoxLayout* newQ::VBoxLayout() {
    auto ret = new QVBoxLayout;
    ret->setSpacing(2);
    ret->setMargin(2);
    return ret;
}

QGridLayout* newQ::GridLayout() {
    auto ret = new QGridLayout;
    ret->setSpacing(2);
    ret->setMargin(2);
    return ret;
}

QFile* newQ::OutputFile(
    const QString& name, QWidget* parent, const QString& path, bool check_overwrite) {
    QFile* ret = new QFile(path);
    if (check_overwrite && ret->exists() &&
        QMessageBox::question(parent, "File exists", "Overwrite " + path + " ?") !=
        QMessageBox::Yes) {
        delete ret;
        return nullptr;
    }
    if (!ret->open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file for writing: " << path;
        return nullptr;
    }
    return ret;
}

QAction* newQ::Trigger(const QString& name, rcstr text, rcstr iconFile) {
    QAction* ret = new QAction(text, qApp);
    ret->setToolTip(text.toLower());
    if (iconFile!="")
        ret->setIcon(QIcon(iconFile));
    return ret;
};

QAction* newQ::Toggle(const QString& name, rcstr text, bool value, rcstr iconFile) {
    QAction* ret = new QAction(text, qApp);
    ret->setToolTip(text.toLower());
    if (iconFile!="")
        ret->setIcon(QIcon(iconFile));
    ret->setCheckable(true);
    ret->setChecked(value);
    return ret;
};

QLabel* newQ::Label(const QString& name, rcstr text) {
    return new QLabel(text);
}

QLabel* newQ::Icon(const QString& name, rcstr fileName) {
    auto ret = new QLabel;
    int h = ret->sizeHint().height();
    ret->setPixmap(QIcon(fileName).pixmap(QSize(h, h)));
    return ret;
}

QLineEdit* newQ::LineDisplay(const QString& name, int ndigits, bool withDot) {
    auto ret = new QLineEdit;
    setWidth(ret, ndigits, withDot);
    ret->setReadOnly(true);
    return ret;
}

// A QSpinBox controls an integer value. Therefore normally we need no extra width for a dot.
// However, sometimes we want to make a QSpinBox exactly as wide as a given QDoubleSpinBox,
// for nice vertical alignement. Then we use withDot=true.
QSpinBox* newQ::SpinBox(const QString& name, int ndigits, bool withDot, int min, int max) {
    auto ret = new QSpinBox;
    setWidth(ret, ndigits, withDot);
    ret->setMinimum(min);
    ret->setMaximum(max > min ? max : min);
    return ret;
}

QDoubleSpinBox* newQ::DoubleSpinBox(const QString& name, int ndigits, qreal min, qreal max) {
    auto ret = new QDoubleSpinBox;
    setWidth(ret, ndigits, true);
    ret->setMinimum(min);
    ret->setMaximum(max > min ? max : min);
    return ret;
}

QCheckBox* newQ::CheckBox(const QString& name, rcstr text) {
    return new QCheckBox(text);
}

QCheckBox* newQ::CheckBox(const QString& name, QAction* action) {
    if (!action)
        return new QCheckBox("");
    auto ret = new QCheckBox(action->text().toLower());
    QObject::connect(ret, &QCheckBox::toggled, [action](bool on) { action->setChecked(on); });
    QObject::connect(action, &QAction::toggled, [ret](bool on) { ret->setChecked(on); });
    ret->setToolTip(action->toolTip());
    ret->setChecked(action->isChecked());
    return ret;
}

QToolButton* newQ::TextButton(const QString& name, QAction* action) {
    auto ret = new QToolButton;
    ret->setDefaultAction(action);
    ret->setToolButtonStyle(Qt::ToolButtonTextOnly);
    return ret;
}

QToolButton* newQ::IconButton(const QString& name, QAction* action) {
    auto ret = new QToolButton;
    ret->setDefaultAction(action);
    ret->setToolButtonStyle(Qt::ToolButtonIconOnly);
    return ret;
}

QRadioButton* newQ::RadioButton(const QString& name, rcstr text) {
    return new QRadioButton(text);
}
