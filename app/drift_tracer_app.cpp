#include <stdio.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <thread>
#include <functional>
#include <signal.h>

// Third party libraries
#include "CLI/CLI.hpp"
#ifndef SPDLOG_FMT_EXTERNAL
#define SPDLOG_FMT_EXTERNAL
#endif
#ifndef SPDLOG_COMPILED_LIB
#define SPDLOG_COMPILED_LIB
#endif
#include "spdlog/spdlog.h"

#if _WIN32

// Keep this below commented out.
// This is for a case when you need cpp debugging on Windows.
//#ifdef _WINSOCKAPI_
//#error "You include <winsock.h> somewhere, remove it. It causes conflicts"
//#endif

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

inline bool SysInitializeNetwork()
{
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    return WSAStartup(wVersionRequested, &wsaData) == 0;
}

inline void SysCleanupNetwork()
{
    WSACleanup();
}

#else
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// Nothing needs to be done on POSIX; this is a Windows problem.
inline bool SysInitializeNetwork() {return true;}
inline void SysCleanupNetwork() {}

#endif

#include "start.hpp"

using namespace std;


atomic_bool force_break(false);


void OnINT_ForceExit(int)
{
    cerr << "\n-------- REQUESTED INTERRUPT!\n";
    force_break = true;
}

struct NetworkInit
{
    NetworkInit()
    {
        // This is mainly required on Windows to initialize the network system,
        // for a case when the instance would use UDP. SRT does it on its own, independently.
        if (!SysInitializeNetwork())
            throw std::runtime_error("Can't initialize network!");
    }

    // Symmetrically, this does a cleanup; put into a local destructor to ensure that
    // it's called regardless of how this function returns.
    ~NetworkInit()
    {
        SysCleanupNetwork();
    }
};


int main(int argc, char **argv)
{
    CLI::App app("Drift Tracer tool.");
    app.set_config("--config");
    app.set_help_all_flag("--help-all", "Expand all help");

    spdlog::set_pattern("%H:%M:%S.%f %^[%L]%$ %v");
    app.add_flag_function("--verbose,-v", [](size_t) {
            spdlog::set_level(spdlog::level::trace);
        }, "enable verbose output");

    app.add_flag_function("--handle-sigint", [](size_t) {
            signal(SIGINT, OnINT_ForceExit);
            signal(SIGTERM, OnINT_ForceExit);
        }, "Handle Ctrl+C interrupt");


    CLI::App* cmd_version = app.add_subcommand("version", "Show version info")
        ->callback([]() { cerr << "Version 0.0.1\n"; });

    string url;
    config cfg;
    CLI::App* sc_send = add_subcommand(app, cfg, url);

    app.require_subcommand(1);
    CLI11_PARSE(app, argc, argv);

    // Startup and cleanup network sockets library
    const NetworkInit nwobject;

    if (sc_send->parsed())
    {
        run(url, cfg, force_break);
        return 0;
    }
    else
    {
        cerr << "Failed to recognize subcommand" << endl;
    }

    return 0;
}


