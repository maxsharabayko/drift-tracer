#include "stdafx.hpp"

#include "utils.hpp"
#include "drift_tracer.hpp"


// submodules
#include "spdlog/spdlog.h"

class tsbpd
{
public:
    /// @returns current drift sample
    long long on_ackack(unsigned timestamp_us, int rtt_us)
    {
        if (m_tsTsbPdTimeBase == std::chrono::steady_clock::time_point())
        {
            m_tsTsbPdTimeBase = std::chrono::steady_clock::now() - microseconds_from(timestamp_us);
            m_first_rtt_us = rtt_us;
            return 0;
        }

        const std::chrono::steady_clock::duration drift =
            std::chrono::steady_clock::now() - (get_time_base(timestamp_us) + microseconds_from(timestamp_us));
        const long long drift_us = count_microseconds(drift) - (rtt_us - m_first_rtt_us) / 2;
        const bool updated = m_drift_tracer.update(drift_us);
        if (updated)
        {
            // tracer's overdrift will be reset to 0 with the next incoming sample.
            std::chrono::steady_clock::duration overdrift = microseconds_from(m_drift_tracer.overdrift());
            m_tsTsbPdTimeBase += overdrift;

            spdlog::info("TSBPD base time shift {} us, drift {}", count_microseconds(overdrift), m_drift_tracer.drift());
        }

        return drift_us;
    }

    std::chrono::steady_clock::time_point get_time_base(uint32_t timestamp_us);

    int64_t drift() const { return m_drift_tracer.drift(); }
    int64_t overdrift() const { return m_drift_tracer.overdrift(); }
    std::chrono::steady_clock::time_point get_time_base() const { return m_tsTsbPdTimeBase; }

private:
    /// Max drift (usec) above which TsbPD Time Offset is adjusted
    static const int TSBPD_DRIFT_MAX_VALUE = 5000;
    /// Number of samples (UMSG_ACKACK packets) to perform drift calculation and compensation
    static const int TSBPD_DRIFT_MAX_SAMPLES = 1000;
    DriftTracer<TSBPD_DRIFT_MAX_SAMPLES, TSBPD_DRIFT_MAX_VALUE> m_drift_tracer;

    std::chrono::steady_clock::time_point m_tsTsbPdTimeBase = {};   // localtime base for TsbPd mode
    // Note: m_tsTsbPdTimeBase cumulates values from:
    // 1. Initial SRT_CMD_HSREQ packet returned value diff to current time:
    //    == (NOW - PACKET_TIMESTAMP), at the time of HSREQ reception
    // 2. Timestamp overflow (@c CRcvBuffer::getTsbPdTimeBase), when overflow on packet detected
    //    += CPacket::MAX_TIMESTAMP+1 (it's a hex round value, usually 0x1*e8).
    // 3. Time drift (CRcvBuffer::addRcvTsbPdDriftSample, executed exclusively
    //    from UMSG_ACKACK handler). This is updated with (positive or negative) TSBPD_DRIFT_MAX_VALUE
    //    once the value of average drift exceeds this value in whatever direction.
    //    += (+/-)CRcvBuffer::TSBPD_DRIFT_MAX_VALUE
    //
    // XXX Application-supplied timestamps won't work therefore. This requires separate
    // calculation of all these things above.

    bool m_bTsbPdWrapCheck = false;              // true: check packet time stamp wrap around
    int m_first_rtt_us = 0;
    static const uint32_t TSBPD_WRAP_PERIOD = (30*1000000);    //30 seconds (in usec)
};