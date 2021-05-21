#pragma once

#include "pkt_view.hpp"

enum class ctrl_type
{
	INVALID   = -1, //< invalid control packet type
	HANDSHAKE = 0, //< Connection Handshake. Control: see @a CHandShake.
	KEEPALIVE = 1, //< Keep-alive.
	ACK = 2, //< Acknowledgement. Control: past-the-end sequence number up to which packets have been received.
	LOSSREPORT = 3, //< Negative Acknowledgement (NAK). Control: Loss list.
	CGWARNING = 4, //< Congestion warning.
	SHUTDOWN = 5, //< Shutdown.
	ACKACK = 6, //< Acknowledgement of Acknowledgement. Add info: The ACK sequence number
	DROPREQ = 7, //< Message Drop Request. Add info: Message ID. Control Info: (first, last) number of the message.
	PEERERROR = 8, //< Signal from the Peer side. Add info: Error code.
	USERDEFINED = 0x7FFF //< For the use of user-defined control packets.
};

const char* ctrl_type_str(ctrl_type type);


/// A base class for SRT-like packet (data or control).
/// All other packet types that have a view to SRT header
/// derive from this class.
template <class storage>
class pkt_base : public pkt_view<storage>
{
public:
	pkt_base(const pkt_view<storage> &view)
		: pkt_view<storage>(view)
		, len_(view.capacity())
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
	typedef pkt_field<uint16_t, 0>  fld_ctrltype;
	typedef pkt_field<uint32_t, 8>  fld_timestamp;
	typedef pkt_field<uint32_t, 12> fld_dstsockid;

public:
	bool is_ctrl() const { return (0x80 & (pkt_view<storage>::template get_field<pkt_field<uint8_t, 0>>())) != 0; }
	bool is_data() const { return !this->is_ctrl(); }

	/// Get packet timestamp.
	/// @note A dependant template definition depends on template parameter,
	/// therefore the template keyword is required (ISO C++03 14.2/4).
	uint32_t timestamp() const { return pkt_view<storage>::template get_field<fld_timestamp>(); }

	/// Get destination socket ID.
	uint32_t dstsockid() const { return pkt_view<storage>::template get_field<fld_dstsockid>(); }

	ctrl_type control_type() const;

	void control_type(ctrl_type type);

	/// @brief Actual length of the packet.
	/// Maybe be lower than buffer capacity.
	size_t length() const { return this->len_; }

	void inline set_length(size_t len)
	{
		//SRTX_ASSERT(len <= m_buff.size());
		this->len_ = len;
	}

	const_bufv const_buf() const
	{
		return const_bufv(this->view_.data(), len_);
	}

public: // Setters
		// void set_control_flag(bool is_control);
		// void set_control_type(CtrlType type);
	void timestamp(unsigned ts_us) { pkt_view<storage>::template set_field<fld_timestamp>(ts_us); }
	void dstsockid(uint32_t sock_id) { pkt_view<storage>::template set_field<fld_dstsockid>(sock_id); }

private:
	size_t  len_;  ///< actual length of content
};

template<class storage>
inline ctrl_type pkt_base<storage>::control_type() const
{
	// Discard the highest bit, which defines type of packet: ctrl/data
	const uint16_t type = 0x7FFF & pkt_view<storage>::template get_field<fld_ctrltype>();
	if ((type >= (int) ctrl_type::HANDSHAKE && type <= (int) ctrl_type::PEERERROR)
		|| type == (int) ctrl_type::USERDEFINED)
	{
		return static_cast<ctrl_type>(type);
	}

	return ctrl_type::INVALID;
}

template<class storage>
inline void pkt_base<storage>::control_type(ctrl_type type)
{
	pkt_view<storage>::template set_field<fld_ctrltype>(0x8000 | (unsigned) type);
}
