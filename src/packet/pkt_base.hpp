#pragma once

#include "pkt_view.hpp"

/// A base class for SRT-like packet (data or control).
/// All other packet types that have a view to SRT header
/// derive from this class.
template <class storage>
class pkt_base : public pkt_view<storage>
{
public:
	pkt_base(const pkt_view<storage> &view)
		: pkt_view<storage>(view)
	{
	}

	//  0                   1                   2                   3
	//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	// +-+-+-+-+-+-+-+-+-+-+-+-+- SRT Header +-+-+-+-+-+-+-+-+-+-+-+-+-+
	// |F|        (Field meaning depends on the packet type)           |
	// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	// |          (Field meaning depends on the packet type)           |
	// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	// |                           Timestamp                           |
	// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	// |                     Destination Socket ID                     |
	// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+- CIF -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	// +                        Packet Contents                        +
	// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	typedef pkt_field<uint32_t, 8>  fld_timestamp;
	typedef pkt_field<uint32_t, 12> fld_dstsockid;

public:
	bool is_ctrl() const;
	bool is_data() const;

	/// Get packet timestamp.
	/// @note A dependant template definition depends on template parameter,
	/// therefore the template keyword is required (ISO C++03 14.2/4).
	uint32_t timestamp() const { return pkt_view<storage>::template get_field<fld_timestamp>(); }

	/// Get destination socket ID.
	uint32_t dstsockid() const { return pkt_view<storage>::template get_field<fld_dstsockid>(); }

public: // Setters
		// void set_control_flag(bool is_control);
		// void set_control_type(CtrlType type);
	void timestamp(unsigned ts_us) { pkt_view<storage>::template set_field<fld_timestamp>(ts_us); }
	void dstsockid(uint32_t sock_id) { pkt_view<storage>::template set_field<fld_dstsockid>(sock_id); }
};
