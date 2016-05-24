// ************************************************************************** //
//
//  STeCa2:    StressTexCalculator ver. 2
//
//! @file      core_fit_functions.h
//!
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016
//! @authors   Scientific Computing Group at MLZ Garching
//! @authors   Original version: Christian Randau
//! @authors   Version 2: Antti Soininen, Jan Burle, Rebecca Brydon
//
// ************************************************************************** //

#include "core_fit_functions.h"
#include "core_fit_methods.h"
#include "types/core_json.h"
#include <qmath.h>

namespace core { namespace fit {
//------------------------------------------------------------------------------

Function::Parameter::Parameter()
: value_(0), error_(0), range_(Range::infinite()), maxDelta_(qQNaN())
, maxDeltaPercent_(qQNaN()), maxError_(qQNaN()), maxErrorPercent_(qQNaN())
{
}

Range Function::Parameter::valueRange() const {
  return range_.isValid() ? range_ : Range(value_);
}

void Function::Parameter::setValueRange(qreal min, qreal max) {
  range_.set(min, max);
}

bool Function::Parameter::checkConstraints() {
  return checkConstraints(value_, error_);
}

bool Function::Parameter::checkConstraints(qreal value, qreal error) {
  if (range_.isValid() && !range_.contains(value))
    return false;

  if (!qIsNaN(maxDelta_) && qAbs(value - value_) > maxDelta_)
    return false;

  if (!qIsNaN(maxDeltaPercent_) &&
      (0 == value_ || qAbs((value - value_) / value_) * 100 > maxDeltaPercent_))
    return false;

  if (!qIsNaN(maxError_) && error > maxError_)
    return false;

  if (!qIsNaN(maxErrorPercent_) &&
      (0 == value_ || qAbs(error / value_) * 100 > maxErrorPercent_))
    return false;

  return true;
}

void Function::Parameter::setValue(qreal value, qreal error) {
  value_ = value; error_ = error;
}

static str const KEY_VALUE("value"), KEY_RANGE("range"),
    KEY_MAX_DELTA("maxDelta"), KEY_MAX_DELTA_PERCENT("maxDeltaPercent"),
    KEY_MAX_ERROR("maxError"), KEY_MAX_ERROR_PERCENT("maxErrorPercent");

JsonObj Function::Parameter::saveJson() const {
  return JsonObj()
      .saveReal(KEY_VALUE, value_)
      .saveRange(KEY_RANGE, range_)

      .saveReal(KEY_MAX_DELTA, maxDelta_)
      .saveReal(KEY_MAX_DELTA_PERCENT, maxDeltaPercent_)
      .saveReal(KEY_MAX_ERROR, maxError_)
      .saveReal(KEY_MAX_ERROR_PERCENT, maxErrorPercent_);
}

void Function::Parameter::loadJson(rcJsonObj obj) THROWS {
  value_ = obj.loadReal(KEY_VALUE);
  range_ = obj.loadRange(KEY_RANGE);

  maxDelta_        = obj.loadReal(KEY_MAX_DELTA);
  maxDeltaPercent_ = obj.loadReal(KEY_MAX_DELTA_PERCENT);
  maxError_        = obj.loadReal(KEY_MAX_ERROR);
  maxErrorPercent_ = obj.loadReal(KEY_MAX_ERROR_PERCENT);
}

//------------------------------------------------------------------------------

static str const
    KEY_FUNCTION_TYPE("type"),

    KEY_SUM_FUNCTIONS("sum"), KEY_POLYNOM("polynom"),

    KEY_RAW("Raw"), KEY_GAUSSIAN("Gaussian"), KEY_LORENTZIAN("Lorentzian"),
    KEY_PSEUDOVOIGT1("PseudoVoigt1"), KEY_PSEUDOVOIGT2("PseudoVoigt2");

Function* Function::factory(rcstr type) {
  if (KEY_SUM_FUNCTIONS == type) return new SumFunctions();
  if (KEY_POLYNOM       == type) return new Polynom();
  if (KEY_RAW           == type) return new Raw();
  if (KEY_GAUSSIAN      == type) return new Gaussian();
  if (KEY_LORENTZIAN    == type) return new CauchyLorentz();
  if (KEY_PSEUDOVOIGT1  == type) return new PseudoVoigt1();
  if (KEY_PSEUDOVOIGT2  == type) return new PseudoVoigt2();

  NEVER return nullptr;
}

Function* Function::factory(rcJsonObj obj) THROWS {
  QScopedPointer<Function> f(factory(obj.loadString(KEY_FUNCTION_TYPE)));
  f->loadJson(obj);
  return f.take();
}

Function::Function() {}

JsonObj Function::saveJson() const {
  // nothing to do
  return JsonObj();
}

void Function::loadJson(rcJsonObj) THROWS {
  // nothing to do
}

#ifndef QT_NO_DEBUG
QDebug& operator<<(QDebug& os, Function const& f) {
  auto parCount = f.parameterCount();
  os << str("[%1]").arg(parCount);
  for_i (parCount) { os << str("%1: %2").arg(i).arg(f.parameterAt(i).value()); }
  return os;
}
#endif

//------------------------------------------------------------------------------

SimpleFunction::SimpleFunction() {}

void SimpleFunction::setParameterCount(uint count) {
  parameters_.fill(Parameter(), count);
}

uint SimpleFunction::parameterCount() const {
  return parameters_.count();
}

Function::Parameter& SimpleFunction::parameterAt(uint i) {
  return parameters_[i];
}

void SimpleFunction::reset() {
  for_i (parameters_.count()) {
    auto& p = parameters_[i];
    p.setValue(p.valueRange().bound(0), 0);
  }
}

static str const KEY_PARAMS("parameters");

JsonObj SimpleFunction::saveJson() const {
  JsonArr params;

  for (auto& param : parameters_)
    params.append(param.saveJson());

  return super::saveJson() + JsonObj().saveArr(KEY_PARAMS, params);
}

void SimpleFunction::loadJson(rcJsonObj obj) THROWS {
  super::loadJson(obj);

  JsonArr params = obj.loadArr(KEY_PARAMS);

  uint parCount = params.count();
  setParameterCount(parCount);

  for_i (parCount)
    parameters_[i].loadJson(params.objAt(i));
}

qreal SimpleFunction::parValue(uint i, qreal const* parValues) const {
  return parValues ? parValues[i] : parameters_[i].value();
}

void SimpleFunction::setValue(uint i, qreal val) {
  parameters_[i].setValue(val, 0);
}

//------------------------------------------------------------------------------

SumFunctions::SumFunctions() {}

SumFunctions::~SumFunctions() {
  // dispose of the Functions that were added
  for (Function* f : functions_)
    delete f;
}

void SumFunctions::addFunction(Function* function) {
  EXPECT(function)

  uint parIndex = parameterCount();
  functions_.append(function);

  for_i (function->parameterCount()) {
    // aggregate parameter list
    allParameters_.append(&function->parameterAt(i));
    // lookup helpers
    function4parIndex_.append(function);
    firstParIndex4parIndex_.append(parIndex);
  }
}

uint SumFunctions::parameterCount() const {
  return allParameters_.count();
}

Function::Parameter& SumFunctions::parameterAt(uint i) {
  return *allParameters_.at(i);
}

qreal SumFunctions::y(qreal x, qreal const* parValues) const {
  qreal sum = 0;

  for (Function* f : functions_) {
    sum += f->y(x, parValues);

    if (parValues)
      parValues += f->parameterCount();  // advance to next function
  }

  return sum;
}

qreal SumFunctions::dy(qreal x, uint parIndex, qreal const* parValues) const {
  Function* f = function4parIndex_.at(
      parIndex);  // aggregate parIndex refers to function f

  // offset parameter index
  uint firstIndex = firstParIndex4parIndex_[parIndex];
  if (parValues) parValues += firstIndex;

  ENSURE(firstIndex <= parIndex)
  parIndex -= firstIndex;

  ENSURE(parIndex < f->parameterCount())
  return f->dy(x, parIndex, parValues);
}

static str const KEY_FUNCTION_COUNT("function count"), KEY_FUNCTION("f%1");

JsonObj SumFunctions::saveJson() const {
  JsonObj obj;
  obj.saveString(KEY_FUNCTION_TYPE, KEY_SUM_FUNCTIONS);

  uint funCount = functions_.count();
  obj.saveUint(KEY_FUNCTION_COUNT, funCount);

  for_i (funCount)
    obj.saveObj(KEY_FUNCTION.arg(i + 1), functions_[i]->saveJson());

  return super::saveJson() + obj;
}

void SumFunctions::loadJson(rcJsonObj obj) THROWS {
  RUNTIME_CHECK(functions_.isEmpty(),
                "non-empty sum of functions; cannot load twice");

  super::loadJson(obj);

  uint funCount = obj.loadUint(KEY_FUNCTION_COUNT);

  for_i (funCount) {
    auto                     funObj = obj.loadObj(KEY_FUNCTION.arg(i + 1));
    QScopedPointer<Function> f(
        Function::factory(funObj.loadString(KEY_FUNCTION_TYPE)));
    f->loadJson(funObj);
    addFunction(f.take());
  }
}

//------------------------------------------------------------------------------

Polynom::Polynom(uint degree) {
  setDegree(degree);
}

uint Polynom::degree() const {
  uint parCount = super::parameterCount();
  ENSURE(parCount > 0)
  return parCount - 1;
}

void Polynom::setDegree(uint degree) {
  super::setParameterCount(degree + 1);
}

// the power with *uint* exponent
static qreal pow_n(qreal x, uint n) {
  qreal val = 1;
  while (n-- > 0)
    val *= x;
  return val;
}

qreal Polynom::y(qreal x, qreal const* parValues) const {
  qreal val = 0, xPow = 1;
  for_i (parameters_.count()) {
    val += parValue(i, parValues) * xPow;
    xPow *= x;
  }
  return val;
}

qreal Polynom::dy(qreal x, uint i, qreal const*) const {
  return pow_n(x, i);
}

// REVIEW
qreal Polynom::avgY(rcRange rgeX, qreal const* parValues) const {
  EXPECT(rgeX.isValid())

  qreal w = rgeX.width();
  if (w <= 0) return y(rgeX.min, parValues);

  qreal minY = 0, maxY = 0, minPow = 1, maxPow = 1;

  for_i (parameters_.count()) {
    qreal facY = parValue(i, parValues) / (i + 1);
    minY += facY * (minPow *= rgeX.min);
    maxY += facY * (maxPow *= rgeX.max);
  }

  return (1 / w) * (maxY - minY);
}

void Polynom::fit(rcCurve curve, rcRanges ranges) {
  FittingLevenbergMarquardt().fit(*this, curve.intersect(ranges));
}

Polynom Polynom::fromFit(uint degree, rcCurve curve, rcRanges ranges) {
  Polynom poly(degree);
  poly.fit(curve, ranges);
  return poly;
}

JsonObj Polynom::saveJson() const {
  JsonObj obj;
  obj.saveString(KEY_FUNCTION_TYPE, KEY_POLYNOM);
  return super::saveJson() + obj;
}

void Polynom::loadJson(rcJsonObj obj) THROWS {
  super::loadJson(obj);
}

//------------------------------------------------------------------------------

PeakFunction* PeakFunction::factory(ePeakType type) {
  switch (type) {
  case ePeakType::RAW:
    return new Raw();
  case ePeakType::GAUSSIAN:
    return new Gaussian();
  case ePeakType::LORENTZIAN:
    return new CauchyLorentz();
  case ePeakType::PSEUDOVOIGT1:
    return new PseudoVoigt1();
  case ePeakType::PSEUDOVOIGT2:
    return new PseudoVoigt2();
  default:
    NEVER return nullptr;
  }
}

PeakFunction* PeakFunction::clone() const {
  PeakFunction *f = factory(type());
  *f = *this;
  return f;
}

void PeakFunction::setRange(rcRange range) {
  range_ = range;
}

PeakFunction::PeakFunction() : guessedPeak_(), guessedFWHM_(qQNaN()) {}

void PeakFunction::setGuessedPeak(rcXY peak) {
  guessedPeak_ = peak;
}

void PeakFunction::setGuessedFWHM(qreal fwhm) {
  guessedFWHM_ = fwhm;
}

void PeakFunction::reset() {
  super::reset();
  setGuessedPeak(guessedPeak_);
  setGuessedFWHM(guessedFWHM_);
}

void PeakFunction::fit(rcCurve curve, rcRange range) {
  Curve c = prepareFit(curve, range);
  if (c.isEmpty()) return;

  if (!guessedPeak().isValid()) {  // calculate guesses
    uint peakIndex  = c.maxYindex();
    auto peakTth    = c.x(peakIndex);
    auto peakIntens = c.y(peakIndex);

    // half-maximum indices
    uint hmi1 = peakIndex, hmi2 = peakIndex;

    // left
    for (uint i = peakIndex; i-- > 0;) {
      hmi1 = i;
      if (c.y(i) < peakIntens / 2) break;
    }

    // right
    for (uint i = peakIndex, iCnt = c.count(); i < iCnt; ++i) {
      hmi2 = i;
      if (c.y(i) < peakIntens / 2) break;
    }

    setGuessedPeak(XY(peakTth, peakIntens));
    setGuessedFWHM(c.x(hmi2) - c.x(hmi1));
  }

  FittingLevenbergMarquardt().fit(*this, c);
}

Curve PeakFunction::prepareFit(rcCurve curve, rcRange range) {
  reset();
  return curve.intersect(range);
}

static str const KEY_GUESSED_PEAK("guessed peak"),
    KEY_GUESSED_FWHM("guessed fwhm");

JsonObj PeakFunction::saveJson() const {
  return super::saveJson()
      .saveRange(KEY_RANGE, range_)
      .saveObj(KEY_GUESSED_PEAK, guessedPeak_.saveJson())
      .saveReal(KEY_GUESSED_FWHM, guessedFWHM_);
}

void PeakFunction::loadJson(rcJsonObj obj) THROWS {
  super::loadJson(obj);
  range_ = obj.loadRange(KEY_RANGE);
  guessedPeak_.loadJson(obj.loadObj(KEY_GUESSED_PEAK));
  guessedFWHM_ = obj.loadReal(KEY_GUESSED_FWHM);
}

//------------------------------------------------------------------------------

Raw::Raw() {}

qreal Raw::y(qreal x, qreal const* /*parValues*/) const {
  if (!x_count_ || !range_.contains(x)) return 0;

  uint i = qBound(0, qFloor((x - range_.min) / dx_), x_count_ - 1);
  return fittedCurve_.y(i);
}

qreal Raw::dy(qreal, uint, qreal const*) const {
  return 0; // fake
}

XY Raw::fittedPeak() const {
  if (qIsNaN(sum_y_))
    sum_y_ = fittedCurve_.sumY();

  return XY(range_.center(),  // approximate x
            sum_y_);
}

qreal Raw::fittedFWHM() const {
  return range_.width();  // kind-of
}

XY Raw::peakError() const {
  return XY(0, 0);
}

qreal Raw::fwhmError() const {
  return 0;
}

void Raw::setRange(rcRange range) {
  super::setRange(range);
  prepareY();
}

void Raw::fit(rcCurve curve, rcRange range) {
  fittedCurve_ = prepareFit(curve, range);  // do no more than this
  prepareY();
}

void Raw::prepareY() {
  if (range_.isEmpty() || fittedCurve_.isEmpty()) {
    x_count_ = 0;
    dx_      = 0;
  } else {
    x_count_ = fittedCurve_.count();
    dx_      = range_.width() / x_count_;
  }

  sum_y_ = qQNaN();
}

JsonObj Raw::saveJson() const {
  return super::saveJson().saveString(KEY_FUNCTION_TYPE, KEY_RAW);
}

//------------------------------------------------------------------------------

Gaussian::Gaussian(qreal ampl, qreal xShift, qreal sigma) {
  setParameterCount(3);

  auto& parAmpl   = parameters_[parAMPL];
  auto& parXShift = parameters_[parXSHIFT];
  auto& parSigma  = parameters_[parSIGMA];

  parAmpl.setValueRange(0, qInf());
  parAmpl.setValue(ampl, 0);

  parXShift.setValue(xShift, 0);

  parSigma.setValueRange(0, qInf());
  parSigma.setValue(sigma, 0);
}

qreal Gaussian::y(qreal x, qreal const* parValues) const {
  qreal ampl   = parValue(parAMPL, parValues);
  qreal xShift = parValue(parXSHIFT, parValues);
  qreal sigma  = parValue(parSIGMA, parValues);

  qreal arg = (x - xShift) / sigma;
  qreal exa = exp(-0.5 * arg * arg);

  return ampl * exa;
}

qreal Gaussian::dy(qreal x, uint parIndex, qreal const* parValues) const {
  qreal ampl   = parValue(parAMPL, parValues);
  qreal xShift = parValue(parXSHIFT, parValues);
  qreal sigma  = parValue(parSIGMA, parValues);

  qreal arg = (x - xShift) / sigma;
  qreal exa = exp(-0.5 * arg * arg);

  switch (parIndex) {
  case parAMPL:
    return exa;
  case parXSHIFT:
    return ampl * exa * (x - xShift) / (sigma * sigma);
  case parSIGMA:
    return ampl * exa * ((x - xShift) * (x - xShift)) / (sigma * sigma * sigma);
  default:
    NEVER return 0;
  }
}

void Gaussian::setGuessedPeak(rcXY xy) {
  super::setGuessedPeak(xy);
  setValue(parXSHIFT, xy.x);
  setValue(parAMPL, xy.y);
}

void Gaussian::setGuessedFWHM(qreal val) {
  super::setGuessedFWHM(val);
  // sigma = FWHM * 1/4 * (SQRT(2)/SQRT(ln(2))) = FWHM * 0.424661
  setValue(parSIGMA, val * 0.424661);
}

XY Gaussian::fittedPeak() const {
  return XY(parameters_[parXSHIFT].value(), parameters_[parAMPL].value());
}

qreal Gaussian::fittedFWHM() const {
  return parameters_[parSIGMA].value() / 0.424661;
}

XY Gaussian::peakError() const {
  return XY(parameters_[parXSHIFT].error(), parameters_[parAMPL].error());
}

qreal Gaussian::fwhmError() const {
  // REVIEW
  return parameters_[parSIGMA].error();
}

JsonObj Gaussian::saveJson() const {
  return super::saveJson().saveString(KEY_FUNCTION_TYPE, KEY_GAUSSIAN);
}

//------------------------------------------------------------------------------

CauchyLorentz::CauchyLorentz(qreal ampl, qreal xShift, qreal gamma) {
  setParameterCount(3);

  auto& parAmpl   = parameters_[parAMPL];
  auto& parXShift = parameters_[parXSHIFT];
  auto& parGamma  = parameters_[parGAMMA];

  parAmpl.setValueRange(0, qInf());
  parAmpl.setValue(ampl, 0);

  parXShift.setValue(xShift, 0);

  parGamma.setValueRange(0, qInf());
  parGamma.setValue(gamma, 0);
}

qreal CauchyLorentz::y(qreal x, qreal const* parValues) const {
  qreal ampl   = parValue(parAMPL, parValues);
  qreal xShift = parValue(parXSHIFT, parValues);
  qreal gamma  = parValue(parGAMMA, parValues);

  qreal arg = (x - xShift) / gamma;
  return ampl / (1 + arg * arg);
}

qreal CauchyLorentz::dy(qreal x, uint parIndex, qreal const* parValues) const {
  qreal ampl   = parValue(parAMPL, parValues);
  qreal xShift = parValue(parXSHIFT, parValues);
  qreal gamma  = parValue(parGAMMA, parValues);

  qreal arg1 = (x - xShift) / gamma;
  qreal arg2 = arg1 * arg1;
  qreal arg3 = (1 + arg2) * (1 + arg2);

  switch (parIndex) {
  case parAMPL:
    return 1 / (1 + arg2);
  case parXSHIFT:
    return 2 * ampl * (x - xShift) / (arg3 * gamma * gamma);
  case parGAMMA:
    return 2 * ampl * (x - xShift) * (x - xShift) /
           (arg3 * gamma * gamma * gamma);
  default:
    NEVER return 0;
  }
}

void CauchyLorentz::setGuessedPeak(rcXY xy) {
  super::setGuessedPeak(xy);
  setValue(parXSHIFT, xy.x);
  setValue(parAMPL, xy.y);
}

void CauchyLorentz::setGuessedFWHM(qreal val) {
  super::setGuessedFWHM(val);
  // gamma = HWHM = FWHM / 2
  setValue(parGAMMA, val / 2);
}

XY CauchyLorentz::fittedPeak() const {
  return XY(parameters_[parXSHIFT].value(), parameters_[parAMPL].value());
}

qreal CauchyLorentz::fittedFWHM() const {
  return parameters_[parGAMMA].value() * 2;
}

XY CauchyLorentz::peakError() const {
  return XY(parameters_[parXSHIFT].error(), parameters_[parAMPL].error());
}

qreal CauchyLorentz::fwhmError() const {
  return parameters_[parGAMMA].error();
}

JsonObj CauchyLorentz::saveJson() const {
  return super::saveJson().saveString(KEY_FUNCTION_TYPE, KEY_LORENTZIAN);
}

//------------------------------------------------------------------------------

PseudoVoigt1::PseudoVoigt1(qreal ampl, qreal xShift, qreal sigmaGamma,
                           qreal eta) {
  setParameterCount(4);

  auto& parAmpl       = parameters_[parAMPL];
  auto& parXShift     = parameters_[parXSHIFT];
  auto& parSigmaGamma = parameters_[parSIGMAGAMMA];
  auto& parEta        = parameters_[parETA];

  parAmpl.setValueRange(0, qInf());
  parAmpl.setValue(ampl, 0);

  parXShift.setValue(xShift, 0);

  parSigmaGamma.setValueRange(0, qInf());
  parSigmaGamma.setValue(sigmaGamma, 0);

  parEta.setValueRange(0, 1);
  parEta.setValue(eta, 0);
}

qreal PseudoVoigt1::y(qreal x, qreal const* parValues) const {
  qreal ampl       = parValue(parAMPL, parValues);
  qreal xShift     = parValue(parXSHIFT, parValues);
  qreal sigmaGamma = parValue(parSIGMAGAMMA, parValues);
  qreal eta        = parValue(parETA, parValues);

  qreal arg      = (x - xShift) / sigmaGamma;
  qreal arg2     = arg * arg;
  qreal gaussian = ampl * exp(-arg2 * log(2.0));
  qreal lorentz  = ampl / (1 + arg2);

  return (1 - eta) * gaussian + eta * lorentz;
}

qreal PseudoVoigt1::dy(qreal x, uint parIndex, qreal const* parValues) const {
  qreal ampl       = parValue(parAMPL, parValues);
  qreal xShift     = parValue(parXSHIFT, parValues);
  qreal sigmaGamma = parValue(parSIGMAGAMMA, parValues);
  qreal eta        = parValue(parETA, parValues);

  qreal arg1 = (x - xShift) / sigmaGamma;
  qreal arg2 = arg1 * arg1;
  qreal arg3 = exp(-arg2 * log(2.0));
  qreal arg4 = 1 + arg2;

  switch (parIndex) {
  case parAMPL:
    return eta / arg4 + (1 - eta) * arg3;
  case parXSHIFT:
    return eta * 2 * ampl * (x - xShift) /
               (arg4 * arg4 * sigmaGamma * sigmaGamma) +
           (1 - eta) * 2 * ampl * (x - xShift) * log(2.0) * arg3 /
               (sigmaGamma * sigmaGamma);
  case parSIGMAGAMMA:
    return eta * 2 * ampl * (x - xShift) * (x - xShift) /
               (arg4 * arg4 * sigmaGamma * sigmaGamma * sigmaGamma) +
           (1 - eta) * 2 * ampl * (x - xShift) * (x - xShift) * log(2.0) *
               arg3 / (sigmaGamma * sigmaGamma * sigmaGamma);
  case parETA:
    return ampl / arg4 - ampl * arg3;
  default:
    NEVER return 0;
  }
}

void PseudoVoigt1::setGuessedPeak(rcXY xy) {
  super::setGuessedPeak(xy);
  setValue(parXSHIFT, xy.x);
  setValue(parAMPL, xy.y);
}

void PseudoVoigt1::setGuessedFWHM(qreal val) {
  super::setGuessedFWHM(val);
  setValue(parSIGMAGAMMA, val / 2);
}

XY PseudoVoigt1::fittedPeak() const {
  return XY(parameters_[parXSHIFT].value(), parameters_[parAMPL].value());
}

qreal PseudoVoigt1::fittedFWHM() const {
  return parameters_[parSIGMAGAMMA].value() * 2;
}

XY PseudoVoigt1::peakError() const {
  return XY(parameters_[parXSHIFT].error(), parameters_[parAMPL].error());
}

qreal PseudoVoigt1::fwhmError() const {
  return parameters_[parSIGMAGAMMA].error();
}

JsonObj PseudoVoigt1::saveJson() const {
  return super::saveJson().saveString(KEY_FUNCTION_TYPE, KEY_PSEUDOVOIGT1);
}

//------------------------------------------------------------------------------

PseudoVoigt2::PseudoVoigt2(qreal ampl, qreal mu, qreal hwhmG, qreal hwhmL,
                           qreal eta) {
  setParameterCount(5);

  auto& parAmpl  = parameters_[parAMPL];
  auto& parMu    = parameters_[parXSHIFT];
  auto& parHwhmG = parameters_[parSIGMA];
  auto& parHwhmL = parameters_[parGAMMA];
  auto& parEta   = parameters_[parETA];

  parAmpl.setValueRange(0, qInf());
  parAmpl.setValue(ampl, 0);

  parMu.setValue(mu, 0);

  parHwhmG.setValueRange(0, qInf());
  parHwhmG.setValue(hwhmG, 0);

  parHwhmL.setValueRange(0, qInf());
  parHwhmL.setValue(hwhmL, 0);

  parEta.setValueRange(0, 1);
  parEta.setValue(eta, 0);
}

qreal PseudoVoigt2::y(qreal x, qreal const* parValues) const {
  qreal ampl   = parValue(parAMPL, parValues);
  qreal xShift = parValue(parXSHIFT, parValues);
  qreal sigma  = parValue(parSIGMA, parValues);
  qreal gamma  = parValue(parGAMMA, parValues);
  qreal eta    = parValue(parETA, parValues);

  qreal argG     = (x - xShift) / sigma;
  qreal argG2    = argG * argG;
  qreal gaussian = ampl * exp(-argG2 * log(2.0));

  qreal argL    = (x - xShift) / gamma;
  qreal argL2   = argL * argL;
  qreal lorentz = ampl / (1 + argL2);

  return (1 - eta) * gaussian + eta * lorentz;
}

qreal PseudoVoigt2::dy(qreal x, uint parIndex, qreal const* parValues) const {
  qreal ampl   = parValue(parAMPL, parValues);
  qreal xShift = parValue(parXSHIFT, parValues);
  qreal sigma  = parValue(parSIGMA, parValues);
  qreal gamma  = parValue(parGAMMA, parValues);
  qreal eta    = parValue(parETA, parValues);

  qreal argG1 = (x - xShift) / sigma;
  qreal argG2 = argG1 * argG1;
  qreal argG3 = exp(-argG2 * log(2.0));

  qreal argL1 = (x - xShift) / gamma;
  qreal argL2 = argL1 * argL1;
  qreal argL3 = 1 + argL2;

  switch (parIndex) {
  case parAMPL:
    return eta / argL3 + (1 - eta) * argG3;
  case parXSHIFT:
    return eta * 2 * ampl * (x - xShift) / (argL3 * argL3 * gamma * gamma) +
           (1 - eta) * 2 * ampl * (x - xShift) * log(2.0) * argG3 /
               (sigma * sigma);
  case parSIGMA:
    return (1 - eta) * 2 * ampl * (x - xShift) * (x - xShift) * log(2.0) *
           argG3 / (sigma * sigma * sigma);
  case parGAMMA:
    return eta * 2 * ampl * (x - xShift) * (x - xShift) /
           (argL3 * argL3 * gamma * gamma * gamma);
  case parETA:
    return ampl / argL3 - ampl * argG3;
  default:
    NEVER return 0;
  }
}

void PseudoVoigt2::setGuessedPeak(rcXY xy) {
  super::setGuessedPeak(xy);
  setValue(parXSHIFT, xy.x);
  setValue(parAMPL, xy.y);
}

void PseudoVoigt2::setGuessedFWHM(qreal val) {
  super::setGuessedFWHM(val);
  setValue(parSIGMA, val * 0.424661);
  setValue(parGAMMA, val / 2);
}

XY PseudoVoigt2::fittedPeak() const {
  return XY(parameters_[parXSHIFT].value(), parameters_[parAMPL].value());
}

qreal PseudoVoigt2::fittedFWHM() const {
  qreal eta = parameters_[parETA].value();
  return ((1 - eta) * parameters_[parSIGMA].value() / 0.424661 +
          eta * parameters_[parGAMMA].value() * 2) /
  2;
}

XY PseudoVoigt2::peakError() const {
  return XY(parameters_[parXSHIFT].error(), parameters_[parAMPL].error());
}

qreal PseudoVoigt2::fwhmError() const {
  // REVIEW
  return parameters_[parSIGMA].error() + parameters_[parGAMMA].error();
}

JsonObj PseudoVoigt2::saveJson() const {
  return super::saveJson().saveString(KEY_FUNCTION_TYPE, KEY_PSEUDOVOIGT2);
}

//------------------------------------------------------------------------------
}}
// eof
