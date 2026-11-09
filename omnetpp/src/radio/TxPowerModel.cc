#include "radio/TxPowerModel.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace veins {

namespace {
    constexpr double SpeedOfLight = 299792458.0;    // same value as BaseWorldUtility::speedOfLight()
    constexpr double Pi           = 3.14159265358979323846;
}


TxPowerModel::TxPowerModel(const Params& p) : params(p) {
    if (!(p.carrierFrequencyHz > 0.0)) throw std::invalid_argument("TxPowerModel: carrierFrequency must be > 0");
    if (!(p.pathLossAlpha      > 0.0)) throw std::invalid_argument("TxPowerModel: pathLossAlpha must be > 0");
    if (!(p.minTxPowerMw       > 0.0)) throw std::invalid_argument("TxPowerModel: minTxPower must be > 0");
    if (!(p.maxTxPowerMw >= p.minTxPowerMw)) throw std::invalid_argument("TxPowerModel: maxTxPower must be >= minTxPower");

    const double wavelength = SpeedOfLight / p.carrierFrequencyHz;
    lossAtOneMeterDb = 20.0 * std::log10(4.0 * Pi / wavelength);
}


// radius -> power ---------------------------------------------------------------------------
double TxPowerModel::radiusToTxPowerMw(double radiusM) const {
    const double distance = std::max(radiusM, 1.0);

    const double lossDb = lossAtOneMeterDb + 10.0 * params.pathLossAlpha * std::log10(distance);
    const double txDbm  = sensitivityDbm() + lossDb;
    const double txMw   = std::pow(10.0, txDbm / 10.0);

    return std::clamp(txMw, params.minTxPowerMw, params.maxTxPowerMw);
}


// power -> radius ---------------------------------------------------------------------------
double TxPowerModel::txPowerToRadius(double txPowerMw) const {
    if (!(txPowerMw > 0.0)) return 0.0;

    const double txDbm    = 10.0 * std::log10(txPowerMw);
    const double exponent = (txDbm - sensitivityDbm() - lossAtOneMeterDb) / (10.0 * params.pathLossAlpha);

    return std::pow(10.0, exponent);
}

} // namespace veins
