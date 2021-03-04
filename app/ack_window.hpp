#pragma once
#include <cstdint>
#include <chrono>

static const int32_t SRT_SEQNO_NONE = -1;    // -1: no seq (0 is a valid seqno!)

template <size_t SIZE>
class ack_window
{
    using steady_clock = std::chrono::steady_clock;
    using system_clock = std::chrono::system_clock;
public:
    ack_window() :
        records_(),
        latest_idx(0),
        oldest_idx(0)
    {
        records_[0].ackno = SRT_SEQNO_NONE;
    }

    ~ack_window() {}

    ack_window(const ack_window&) = delete;
    ack_window& operator=(const ack_window&) = delete;

    struct entry
    {
        int32_t ackno;       // Seq. No. for the ACK packet
        int32_t pktseqno;    // Data Seq. No. carried by the ACK packet
        steady_clock::time_point sendtime_std; // The timestamp when the ACK was sent further (steady clock)
        system_clock::time_point sendtime_sys; // The timestamp when the ACK was sent further (system clock)
    };

    /// Write an ACK record into the window.
    /// @param [in] ackno         ACK packet no.
    /// @param [in] pktseqno      packer number of a packet being acknowledged
    /// @param [in] send_time_std time when ACK was sent (steady clock)
    /// @param [in] send_time_sys time when ACK was sent (system clock)
    void store(int32_t ackno, int32_t pktseqno, const steady_clock::time_point& send_time_std, const system_clock::time_point& send_time_sys);

    struct rtt_pair
    {
        int rtt_std;
        int rtt_sys;
    };

    /// Search the ACK-2 "seq" in the window, find out the DATA "ack" and calculate RTT.
    /// @param [in] seq ACK-2 seq. no.
    /// @param [in] recv_time_std time when ACKACK was received (steady clock)
    /// @param [in] recv_time_sys time when ACKACK was received (system clock)
    /// @return RTT.
    rtt_pair acknowledge(int32_t ackno, const std::chrono::steady_clock::time_point& recv_time_std,
        const std::chrono::system_clock::time_point& recv_time_sys);

private:

    entry records_[SIZE];
    int latest_idx;                 // Index of the lastest ACK record
    int oldest_idx;                 // Index of the oldest ACK record

};

template<size_t SIZE>
inline void ack_window<SIZE>::store(int32_t ackno, int32_t pktseqno, const std::chrono::steady_clock::time_point& send_time_std,
    const std::chrono::system_clock::time_point& send_time_sys)
{
    records_[latest_idx].ackno = ackno;
    records_[latest_idx].pktseqno = pktseqno;
    records_[latest_idx].sendtime_std = send_time_std;
    records_[latest_idx].sendtime_sys = send_time_sys;

    latest_idx = (latest_idx + 1) % SIZE;

    // overwrite the oldest ACK
    if (latest_idx == oldest_idx)
        oldest_idx = (oldest_idx + 1) % SIZE;
}

// C++11 Standard Section 14.6 Name Resolution:
// A name used in a template declaration or definition and that is dependent on a template-parameter is assumed not to name a type
// unless the applicable name lookup finds a type name or the name is qualified by the keyword typename.
template<size_t SIZE>
inline typename ack_window<SIZE>::rtt_pair ack_window<SIZE>::acknowledge(int32_t ackno,
    const std::chrono::steady_clock::time_point& recv_time_std,
    const std::chrono::system_clock::time_point& recv_time_sys)
{
    using namespace std::chrono;
    int32_t pktseqno = SRT_SEQNO_NONE; // TODO: consider returning as a pair. Unused now.

    if (latest_idx >= oldest_idx)
    {
        // Head has not exceeded the physical boundary of the window

        for (int i = oldest_idx, n = latest_idx; i < n; ++i)
        {
            // looking for identical ACK Seq. No.
            if (ackno == records_[i].ackno)
            {
                // return the Data ACK it carried
                pktseqno = records_[i].pktseqno;

                // calculate RTT
                const int rtt_std = (int) duration_cast<microseconds>(recv_time_std - records_[i].sendtime_std).count();
                const int rtt_sys = (int) duration_cast<microseconds>(recv_time_sys - records_[i].sendtime_sys).count();

                if (i + 1 == latest_idx)
                {
                    oldest_idx = latest_idx = 0;
                    records_[0].ackno = SRT_SEQNO_NONE;
                }
                else
                    oldest_idx = (i + 1) % SIZE;

                return { rtt_std, rtt_sys };
            }
        }

        // Bad input, the ACK node has been overwritten
        return { -1, -1 };
    }

    // Head has exceeded the physical window boundary, so it is behind tail
    for (int j = oldest_idx, n = latest_idx + SIZE; j < n; ++j)
    {
        // looking for indentical ACK seq. no.
        if (ackno == records_[j % SIZE].ackno)
        {
            // return Data ACK
            j %= SIZE;
            pktseqno = records_[j].pktseqno;

            // calculate RTT
            const int rtt_std = (int) duration_cast<microseconds>(recv_time_std - records_[j].sendtime_std).count();
            const int rtt_sys = (int)duration_cast<microseconds>(recv_time_sys - records_[j].sendtime_sys).count();

            if (j == latest_idx)
            {
                oldest_idx = latest_idx = 0;
                records_[0].ackno = SRT_SEQNO_NONE;
            }
            else
                oldest_idx = (j + 1) % SIZE;

            return { rtt_std, rtt_sys };
        }
    }

    // bad input, the ACK node has been overwritten
    return { -1, -1 };
}
