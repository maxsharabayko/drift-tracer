#include "tsbpd.hpp"

using namespace std::chrono;

static const uint32_t MAX_TIMESTAMP = 0xFFFFFFFF; //Full 32 bit (01h11m35s)

void tsbpd::updateTsbPdTimeBase(uint32_t usPktTimestamp)
{
    if (m_bTsbPdWrapCheck)
    {
        // Wrap check period.
        if ((usPktTimestamp >= TSBPD_WRAP_PERIOD) && (usPktTimestamp <= (TSBPD_WRAP_PERIOD * 2)))
        {
            /* Exiting wrap check period (if for packet delivery head) */
            m_bTsbPdWrapCheck = false;
            m_tsTsbPdTimeBase += microseconds_from(int64_t(MAX_TIMESTAMP) + 1);
            spdlog::info("TSBPD wrap period ends with ts={}, drift: {}", usPktTimestamp, m_drift_tracer.drift());
        }
        return;
    }

    // Check if timestamp is in the last 30 seconds before reaching the MAX_TIMESTAMP.
    if (usPktTimestamp > (MAX_TIMESTAMP - TSBPD_WRAP_PERIOD))
    {
        // Approching wrap around point, start wrap check period (if for packet delivery head)
        m_bTsbPdWrapCheck = true;
        spdlog::info("TSBPD wrap period begins with ts={}, drift: {}", usPktTimestamp, m_drift_tracer.drift());
    }
}

steady_clock::time_point tsbpd::get_pkt_time_base(uint32_t timestamp_us) const
{
    const uint64_t carryover_us =
        (m_bTsbPdWrapCheck && timestamp_us < TSBPD_WRAP_PERIOD) ? uint64_t(MAX_TIMESTAMP) + 1 : 0;

    return (m_tsTsbPdTimeBase + microseconds_from(carryover_us));
}

