#include <array>
#include <chrono>
#include <future>
#include <map>
#include <mutex>

#include "start.hpp"
#include "uri_parser.hpp"
#include "udp_socket.hpp"
#include "ack_window.hpp"
#include "utils.hpp"
#include "path.hpp"
#include "drift_tracer.hpp"
#include "tsbpd.hpp"
#include "stats_logger.hpp"

#include "buf_view.hpp"
#include "packet/pkt_base.hpp"
#include "packet/pkt_ack.hpp"
#include "packet/pkt_ackack.hpp"

// submodules
#include "spdlog/spdlog.h"

using namespace std;
using namespace chrono;
#define LOG_SC_RECV "[PATH] "

using shared_udp = shared_ptr<socket_udp>;

mutex g_path_mut;
path_metrics g_path;
const auto g_start_time = steady_clock::now();
tsbpd g_tsbpd;

unique_ptr<stats_logger> g_stats_logger;

unsigned int get_timestamp()
{
    return duration_cast<microseconds>(steady_clock::now() - g_start_time).count();
}

void update_tsbpd_base(unsigned peer_timestamp)
{
    g_tsbpd.on_ackack(peer_timestamp);
}

/// @brief Sends ACK packets every 10 ms
/// @param sock_udp UDP socket to use for ACK sending
/// @param force_break a flag to check in case app wants to close itself
void ack_sending_loop(shared_udp sock_udp, const atomic_bool& force_break)
{
    const size_t mtu_size = 1500;
    vector<unsigned char> buffer(mtu_size);
    socket_udp& sock_dst = *sock_udp.get();

    spdlog::info(LOG_SC_RECV "SND Started");

    unsigned int ackno = 1;
    while (!force_break)
    {
        this_thread::sleep_for(10ms);

        fill(buffer.begin(), buffer.end(), 0);
        pkt_ack<mut_bufv> pkt(mut_bufv(buffer.data(), buffer.size()));
        pkt.control_type(ctrl_type::ACK);
        pkt.timestamp(get_timestamp());
        pkt.ackno(ackno++);

        g_path_mut.lock();
        if (g_path.rtt != 0)
        {
            pkt.rtt(g_path.rtt);
            pkt.rttvar(g_path.rtt_var);
        }
        g_path_mut.unlock();

        const int bytes_sent = sock_dst.send(pkt.const_buf());

        if (bytes_sent != pkt.length())
        {
            spdlog::warn("SND send returned {} bytes, expected {}", bytes_sent, pkt.length());
            continue;
        }

        lock_guard<mutex> lck(g_path_mut);
        g_path.ack_records.store(pkt.ackno(), pkt.ackseqno());
    }
}

void on_ctrl_ack(pkt_ack<const_bufv> ackpkt, socket_udp& sock_udp)
{
    array<unsigned char, 40> buffer = {};
    pkt_ackack<mut_bufv> pkt(mut_bufv(buffer.data(), buffer.size()));
    pkt.control_type(ctrl_type::ACKACK);
    pkt.timestamp(get_timestamp());
    pkt.ackno(ackpkt.ackno());

    // TODO: Extract RTT and RTTVar

    const int bytes_sent = sock_udp.send(pkt.const_buf());
}

void on_ctrl_ackack(pkt_ackack<const_bufv> ackpkt)
{
    lock_guard<mutex> lck(g_path_mut);
    const int rtt = g_path.ack_records.acknowledge(ackpkt.ackno());

    if (g_path.rtt == 0)
    {
        g_path.rtt = rtt;
        g_path.rtt_var = rtt / 2;
    }
    else
    {
        g_path.rtt_var = avg_rma<4, int>(g_path.rtt_var, abs(rtt - g_path.rtt));
        g_path.rtt = avg_rma<8>(g_path.rtt, rtt);
    }

    const long long drift_sample = g_tsbpd.on_ackack(ackpkt.timestamp());

    if (g_stats_logger)
    {
        g_stats_logger->trace(rtt, g_path.rtt, g_path.rtt_var, drift_sample,
            g_tsbpd.drift(), g_tsbpd.overdrift(), g_tsbpd.get_time_base());
    }
    else
    {
        spdlog::info("Estimated RTT {}, RTT rma {}, RTT var {}, drift {}", rtt, g_path.rtt, g_path.rtt_var, g_tsbpd.drift());
    }
}

/// @brief Receives packets from data receiver and forwards them over multiple links to data sender.
/// @param src source UDP socket
/// @param force_break a flag to break the loop and return from the function
void ack_reply_loop(shared_udp src, const atomic_bool& force_break)
{
    const size_t mtu_size = 1500;
    vector<unsigned char> buffer(mtu_size);

    socket_udp& sock_src = *src.get();

    spdlog::info(LOG_SC_RECV "RCV Started");

    while (!force_break)
    {
        // TODO: Save timepoint as close to packet reception as possible
        const auto [bytes_read, src_addr] = sock_src.recvfrom(mut_bufv(buffer.data(), buffer.size()), -1);

        if (bytes_read == 0)
        {
            spdlog::info(LOG_SC_RECV "RCV received 0 bytes on a socket (spurious read-ready?). Retrying.");
            continue;
        }

        const_bufv pkt_buf(buffer.data(), bytes_read);
        pkt_base<const_bufv> pkt(pkt_buf);

        if (!pkt.is_ctrl())
        {
            spdlog::info(LOG_SC_RECV "RCV received unknown packet. Ignoring.");
            continue;
        }

        const auto ctrl_pkt_type = pkt.control_type();
        if (ctrl_pkt_type == ctrl_type::ACK)
        {
            on_ctrl_ack(pkt, sock_src);
        }
        else if (ctrl_pkt_type == ctrl_type::ACKACK)
        {
            on_ctrl_ackack(pkt);
        }
    }
}

shared_udp create_socket(const string& url_str)
{
    const UriParser url(url_str);

    if (url.proto() == "udp")
    {
        return make_shared<socket_udp>(url);
    }

    return nullptr;
}


void run(const string& sock_url,
    const config& cfg, const atomic_bool& force_break)
{
    if (sock_url.empty())
    {
        spdlog::error(LOG_SC_RECV "Empty URL was provided");
        return;
    }

    // 2. Create UDP socket
    shared_udp sock_udp = create_socket(sock_url);
    if (!sock_udp)
    {
        spdlog::error(LOG_SC_RECV "Target creation failed!");
        return;
    }

    if (!cfg.statsfile.empty())
    {
        try {
            g_stats_logger = make_unique<stats_logger>(cfg.statsfile);
        }
        catch (const runtime_error& e)
        {
            spdlog::error(e.what());
            return;
        }
    }

    future<void> fb_route = ::async(::launch::async, ack_reply_loop, sock_udp, ref(force_break));

    ack_sending_loop(sock_udp, force_break);

    fb_route.wait();
}

CLI::App* add_subcommand(CLI::App& app, config& cfg, string& sock_url)
{
    CLI::App* sc_route = app.add_subcommand("start", "Start exchange")->fallthrough();
    sc_route->add_option("sock_url", sock_url, "Source URI")->expected(1);
    sc_route->add_option("--tracefile", cfg.statsfile, "Trace output file");

    return sc_route;
}

