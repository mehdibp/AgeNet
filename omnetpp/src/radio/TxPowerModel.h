#pragma once

namespace veins {

// Converts between a transmission RADIUS [m] (what the Python agents think in) and a transmit
// POWER [mW] (what the radio needs), for the free-space channel configured in omnetpp.ini/config.xml.
//
// It mirrors Veins' own SimplePathlossModel (veins-5.3.1):
//
//     received = P_tx * lambda^2 / (16 pi^2) * d^(-alpha)         (d <= 1 m: no attenuation at all)
//
//     Loss(d)  [dB]  = 20 log10(4 pi / lambda) + 10 alpha log10(d)   (for alpha = 2: 20 log10(4 pi d / lambda))
//     P_sens   [dBm] = noiseFloor + minSinr          weakest signal that is still received reliably
//     P_tx     [dBm] = P_sens + Loss(r)              radius -> power
//     r              = 10 ^ ((P_tx - P_sens - 20 log10(4 pi / lambda)) / (10 alpha))     power -> radius
//
// "Radius" therefore means: the distance (line of sight, no interference, no buildings) at which a
// frame arrives with SINR == minSinr. Interference and obstacles only make the real range smaller.
// minSinr is a design margin: Veins decides reception probabilistically (NistErrorRate), there is no
// hard threshold.
//
// This class is plain C++ on purpose, so it can be unit-tested without OMNeT++ (tools/tx_power_table.cc).

class TxPowerModel {
    private:
        Params params;
        double lossAtOneMeterDb;        // 20 log10(4 pi / lambda)

    public:
        struct Params {
            double carrierFrequencyHz = 5.89e9;     // = centerFrequency in config.xml
            double pathLossAlpha      = 2.0;        // = alpha of SimplePathlossModel in config.xml
            double noiseFloorDbm      = -98.0;      // = *.**.nic.phy80211p.noiseFloor in omnetpp.ini
            double minSinrDb          = 8.0;        // 6 Mbps = QPSK 1/2 (802.11p, 10 MHz); design margin
            double minTxPowerMw       = 0.0001;     // lower clamp of the power we are willing to set
            double maxTxPowerMw       = 100.0;      // upper clamp
        };

        // Throws std::invalid_argument for nonsensical parameters.
        explicit TxPowerModel(const Params& params);

        // Power [mW] needed to reach `radiusM`, clamped to [minTxPowerMw, maxTxPowerMw].
        double radiusToTxPowerMw(double radiusM) const;

        // Radius [m] that a power of `txPowerMw` reaches.
        double txPowerToRadius(double txPowerMw) const;

        double minRadius() const { return txPowerToRadius(params.minTxPowerMw); }
        double maxRadius() const { return txPowerToRadius(params.maxTxPowerMw); }

        double sensitivityDbm() const { return params.noiseFloorDbm + params.minSinrDb; }
        const Params& getParams() const { return params; }
};

} // namespace veins
