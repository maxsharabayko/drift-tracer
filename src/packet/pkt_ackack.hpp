#pragma once

#include <vector>

#include "pkt_base.hpp"

/// Class for SRT ACKACK packet.
template <class storage>
class pkt_ackack : public pkt_base<storage>
{
public:
	pkt_ackack(const storage &buf_view)
		: pkt_base<storage>(buf_view)
	{
	}

	pkt_ackack(const pkt_base<storage> &pkt)
		: pkt_base<storage>(pkt)
	{
	}

public:
	/// HS Induction Response
	///    0                   1                   2                   3
	///    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	///   +-+-+-+-+-+-+-+-+-+-+-+-+- SRT Header +-+-+-+-+-+-+-+-+-+-+-+-+-+
	/// 0 |1|         Control Type        |            Subtype            |
	///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	/// 1 |                     Acknowledgement Number                    |
	///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	/// 2 |                           Timestamp                           |
	///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	/// 3 |                     Destination Socket ID                     |
	///   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	typedef pkt_field<uint32_t, 1 * 4>     fld_ackno;

public: // Getters
	uint32_t ackno() const { return pkt_view<storage>::template get_field<fld_ackno>(); }

public: // Setters
	void ackno(uint32_t value) { return pkt_view<storage>::template set_field<fld_ackno>(value); }
};

