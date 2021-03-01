#pragma once
#include "stdafx.hpp"

#include "utils.hpp"

class stats_logger
{
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

    void trace(int rtt, int rtt_rma, int rtt_var, int64_t drift_sample, int64_t drift, int64_t overdrift,
        const std::chrono::steady_clock::time_point& tsbpd_base)
    {
        std::lock_guard<std::mutex> lck(this->mtx_);

        this->fout_ << print_timestamp() << ",";
        this->fout_ << rtt << ",";
        this->fout_ << rtt_rma << ",";
        this->fout_ << rtt_var << ",";
        this->fout_ << drift_sample << ",";
        this->fout_ << drift << ",";
        this->fout_ << overdrift << ",";
        this->fout_ << format_time_stdy(tsbpd_base) << "\n";
        this->fout_.flush();
    }

private:
    void print_header()
    {
        //std::lock_guard<std::mutex> lck(this->mtx_);
        this->fout_ << "Timepoint,RTT,RTTrma,RTTVar,DriftSample,Drift,Overdrift,TSBPDBase\n";
    }

private:
    std::mutex mtx_;
    std::ofstream fout_;

};
