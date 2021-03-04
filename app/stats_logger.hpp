#pragma once
#include "stdafx.hpp"

#include "utils.hpp"

class stats_logger
{
    using steady_clock = std::chrono::steady_clock;
    using system_clock = std::chrono::system_clock;
public:
    stats_logger(const std::string& filename)
    {
        this->fout_.open(filename, std::ofstream::out);
        if (!this->fout_)
            throw std::runtime_error("Failed to open " + filename + "!!!");

        print_header();
    }

    ~stats_logger()
    {
        std::lock_guard<std::mutex> lck(this->mtx_);
        this->fout_.close();
    }

    void trace(const steady_clock::duration& elapsed_std, const system_clock::duration& elapsed_sys,
        unsigned ackack_timestamp, int rtt_sys, int rtt_std, int rtt_std_rma, int rtt_std_var,
        int64_t drift_sample_std, int64_t drift, int64_t overdrift,
        const std::chrono::steady_clock::time_point& tsbpd_base)
    {
        using namespace std::chrono;
        std::lock_guard<std::mutex> lck(this->mtx_);

        this->fout_ << print_timestamp() << ",";
        this->fout_ << duration_cast<microseconds>(elapsed_std).count() << ",";
        this->fout_ << duration_cast<microseconds>(elapsed_sys).count() << ",";
        this->fout_ << ackack_timestamp << ",";
        this->fout_ << rtt_sys << ",";
        this->fout_ << rtt_std << ",";
        this->fout_ << rtt_std_rma << ",";
        this->fout_ << rtt_std_var << ",";
        this->fout_ << drift_sample_std << ",";
        this->fout_ << drift << ",";
        this->fout_ << overdrift << ",";
        this->fout_ << format_time_stdy(tsbpd_base) << "\n";
        this->fout_.flush();
    }

private:
    void print_header()
    {
        //std::lock_guard<std::mutex> lck(this->mtx_);
        this->fout_ << "TimepointSys,usElapsedStd,usElapsedSys,usAckAckTimestamp,usRttSys,usRTTStd,usRTTStdRma,RTTStdVar,usDriftSampleStd,usDriftStd,usOverdriftStd,TSBPDBase\n";
    }

private:
    std::mutex mtx_;
    std::ofstream fout_;

};
