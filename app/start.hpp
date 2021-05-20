#pragma once
#include "stdafx.hpp"

// Third party libraries
#include "CLI/CLI.hpp"


struct config
{
    int message_size    = 1456;
    bool compensate_rtt = false;
    bool compact_trace  = false;
    int  duration       = 0;
    int  sendrate       = 0;
    std::string statsfile;
};


void run(const std::string& sock_url,
    const config& cfg, const std::atomic_bool& force_break);

CLI::App* add_subcommand(CLI::App& app, config& cfg,
    std::string& sock_url);

