#pragma once

#include <vector>

#include "pkt_base.hpp"

/// Class for SRT ACK packet.
template <class storage>
class pkt_ack : public pkt_base<storage>
{
public:
	pkt_ack(const storage &buf_view)
		: pkt_base<storage>(buf_view)
	{
	}

	pkt_ack(const pkt_base<storage> &pkt)
		: pkt_base<storage>(pkt)
	{
	}

public:
	/// SRT Header:
	///    0                   1                   2                   3
	///    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	///   +-+-+-+-+-+-+-+-+-+-+-+-+- SRT Header +-+-+-+-+-+-+-+-+-+-+-+-+-+
	/// 0 |1|         Control Type        |            Subtype            |
	///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	/// 1 |                        ACK Packet Number                      |
	///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	/// 2 |                           Timestamp                           |
	///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	/// 3 |                     Destination Socket ID                     |
	///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	/// ACK Packet contents:
	///
	///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	/// 4 |            Last Acknowledged Packet Sequence Number           |
	///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	/// 5 |                              RTT                              |
	///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	/// 6 |                          RTT variance                         |
	///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	/// 7 |                     Available Buffer Size                     |
	///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	/// 8 |                     Packets Receiving Rate                    |
	///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	/// 9 |                     Estimated Link Capacity                   |
	///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	///   |                         Receiving Rate                        |
	///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	typedef pkt_field<uint32_t, 1 * 4>  fld_ackno;
	typedef pkt_field<uint32_t, 4 * 4>  fld_ackseqno;
	typedef pkt_field<uint32_t, 5 * 4>  fld_rtt;
	typedef pkt_field<uint32_t, 6 * 4>  fld_rttvar;
	typedef pkt_field<uint32_t, 7 * 4>  fld_bufavail;
	typedef pkt_field<uint32_t, 8 * 4>  fld_recvpktrate;
	typedef pkt_field<uint32_t, 9 * 4>  fld_capacity;
	typedef pkt_field<uint32_t, 10 * 4> fld_recvrate;

public: // Getters
	uint32_t ackno() const { return pkt_view<storage>::template get_field<fld_ackno>(); }
	uint32_t ackseqno() const { return pkt_view<storage>::template get_field<fld_ackseqno>(); }
	uint32_t rtt() const { return pkt_view<storage>::template get_field<fld_rtt>(); }
	uint32_t rttvar() const { return pkt_view<storage>::template get_field<fld_rtt_var>(); }
	uint32_t bufavail() const { return pkt_view<storage>::template get_field<fld_bufavail>(); }
	uint32_t recvpktrate() const { return pkt_view<storage>::template get_field<fld_hrecvpktrate>(); }
	uint32_t capacity() const { return pkt_view<storage>::template get_field<fld_capacity>(); }
	uint32_t recvrate() const { return pkt_view<storage>::template get_field<fld_recvrate>(); }

public: // Setters
	void ackno(uint32_t value) { return pkt_view<storage>::template set_field<fld_ackno>(value); }
	void ackseqno(uint32_t value) { return pkt_view<storage>::template set_field<fld_ackseqno>(value); }
	void rtt(uint32_t value) { return pkt_view<storage>::template set_field<fld_rtt>(value); }
	void rttvar(uint32_t value) { return pkt_view<storage>::template set_field<fld_rttvar>(value); }
	void bufavail(uint32_t value) { return pkt_view<storage>::template set_field<fld_bufavail>(value); }
	void recvpktrate(uint32_t value) { return pkt_view<storage>::template set_field<fld_recvpktrate>(value); }
	void capacity(uint32_t value) { return pkt_view<storage>::template set_field<fld_capacity>(value); }
	void recvrate(uint32_t value) { return pkt_view<storage>::template set_field<fld_recvrate>(value); }
};
