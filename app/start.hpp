#pragma once
#include <atomic>
#include <string>

// Third party libraries
#include "CLI/CLI.hpp"


struct config
{
    int message_size = 1456;
};


void run(const std::string& sock_url,
    const config& cfg, const std::atomic_bool& force_break);

CLI::App* add_subcommand(CLI::App& app, config& cfg,
    std::string& sock_url);

