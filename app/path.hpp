#include "stdafx.hpp"

#include "ack_window.hpp"

struct path_metrics
{
    //path_metrics() {};
    //path_metrics(path_metrics&& pp) {};

    int rtt     = 0;
    int rtt_var = 0;
    // TODO: track lost packets
    ack_window<1024> ack_records;
};