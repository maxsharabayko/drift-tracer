#pragma once
#include <cstdint>
#include <chrono>

static const int32_t SRT_SEQNO_NONE = -1;    // -1: no seq (0 is a valid seqno!)

template <size_t SIZE>
class ack_window
{
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
        std::chrono::steady_clock::time_point sendtime; // The timestamp when the ACK was sent further
    };

    /// Write an ACK record into the window.
    /// @param [in] ackno     ACK packet no.
    /// @param [in] pktseqno  packer number of a packet being acknowledged
    void store(int32_t ackno, int32_t pktseqno);

    /// Search the ACK-2 "seq" in the window, find out the DATA "ack" and caluclate RTT .
    /// @param [in] seq ACK-2 seq. no.
    /// @return RTT.
    int acknowledge(int32_t ackno);

private:

    entry records_[SIZE];
    int latest_idx;                 // Index of the lastest ACK record
    int oldest_idx;                 // Index of the oldest ACK record

};


template<size_t SIZE>
inline void ack_window<SIZE>::store(int32_t ackno, int32_t pktseqno)
{
    records_[latest_idx].ackno = ackno;
    records_[latest_idx].pktseqno = pktseqno;
    records_[latest_idx].sendtime = std::chrono::steady_clock::now();

    latest_idx = (latest_idx + 1) % SIZE;

    // overwrite the oldest ACK
    if (latest_idx == oldest_idx)
        oldest_idx = (oldest_idx + 1) % SIZE;
}

template<size_t SIZE>
inline int ack_window<SIZE>::acknowledge(int32_t ackno)
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
                const int rtt = (int) duration_cast<microseconds>(steady_clock::now() - records_[i].sendtime).count();

                if (i + 1 == latest_idx)
                {
                    oldest_idx = latest_idx = 0;
                    records_[0].ackno = SRT_SEQNO_NONE;
                }
                else
                    oldest_idx = (i + 1) % SIZE;

                return rtt;
            }
        }

        // Bad input, the ACK node has been overwritten
        return -1;
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
            const int rtt = (int) duration_cast<microseconds>(steady_clock::now() - records_[j].sendtime).count();

            if (j == latest_idx)
            {
                oldest_idx = latest_idx = 0;
                records_[0].ackno = SRT_SEQNO_NONE;
            }
            else
                oldest_idx = (j + 1) % SIZE;

            return rtt;
        }
    }

    // bad input, the ACK node has been overwritten
    return -1;
}
