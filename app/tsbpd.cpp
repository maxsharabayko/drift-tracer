#include "tsbpd.hpp"

using namespace std::chrono;

static const uint32_t MAX_TIMESTAMP = 0xFFFFFFFF; //Full 32 bit (01h11m35s)

steady_clock::time_point tsbpd::get_pkt_time_base(uint32_t timestamp_us)
{
    /*
     * Packet timestamps wrap around every 01h11m35s (32-bit in usec)
     * When added to the peer start time (base time),
     * wrapped around timestamps don't provide a valid local packet delevery time.
     *
     * A wrap check period starts 30 seconds before the wrap point.
     * In this period, timestamps smaller than 30 seconds are considered to have wrapped around (then adjusted).
     * The wrap check period ends 30 seconds after the wrap point, afterwhich time base has been adjusted.
     */
    int64_t carryover = 0;

    // This function should generally return the timebase for the given timestamp_us.
    // It's assumed that the timestamp_us, for which this function is being called,
    // is received as monotonic clock. This function then traces the changes in the
    // timestamps passed as argument and catches the moment when the 64-bit timebase
    // should be increased by a "segment length" (MAX_TIMESTAMP+1).

    // The checks will be provided for the following split:
    // [INITIAL30][FOLLOWING30]....[LAST30] <-- == CPacket::MAX_TIMESTAMP
    //
    // The following actions should be taken:
    // 1. Check if this is [LAST30]. If so, ENTER TSBPD-wrap-check state
    // 2. Then, it should turn into [INITIAL30] at some point. If so, use carryover MAX+1.
    // 3. Then it should switch to [FOLLOWING30]. If this is detected,
    //    - EXIT TSBPD-wrap-check state
    //    - save the carryover as the current time base.

    if (m_bTsbPdWrapCheck)
    {
        // Wrap check period.

        if (timestamp_us < TSBPD_WRAP_PERIOD)
        {
            carryover = int64_t(MAX_TIMESTAMP) + 1;
        }
        //
        else if ((timestamp_us >= TSBPD_WRAP_PERIOD) && (timestamp_us <= (TSBPD_WRAP_PERIOD * 2)))
        {
            /* Exiting wrap check period (if for packet delivery head) */
            m_bTsbPdWrapCheck = false;
            m_tsTsbPdTimeBase += microseconds_from(int64_t(MAX_TIMESTAMP) + 1);
            spdlog::info("TSBPD wrap period ends with ts={}, drift: {}", timestamp_us, m_drift_tracer.drift());
        }
    }
    // Check if timestamp_us is in the last 30 seconds before reaching the MAX_TIMESTAMP.
    else if (timestamp_us > (MAX_TIMESTAMP - TSBPD_WRAP_PERIOD))
    {
        /* Approching wrap around point, start wrap check period (if for packet delivery head) */
        m_bTsbPdWrapCheck = true;
        spdlog::info("TSBPD wrap period begins with ts={}, drift: {}", timestamp_us, m_drift_tracer.drift());
    }

    return (m_tsTsbPdTimeBase + microseconds_from(carryover));
}
