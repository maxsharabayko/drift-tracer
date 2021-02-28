#include "catch2/catch.hpp"

#include <array>

#include "buf_view.hpp"
#include "packet/pkt_view.hpp"
#include "packet/pkt_base.hpp"

TEST_CASE("Packet buffer", "[pktbuf]")
{
	pkt_view const_bufv(const_bufv(nullptr, 0));
	//const_buf.set(5);

	pkt_view mut_buf(mut_bufv(nullptr, 0));
	//mut_buf.set(5);
}

TEST_CASE("Base packet control type", "[pkt_base]")
{
	std::array<unsigned char, 64> buffer = {};
	pkt_base<mut_bufv> pkt(mut_bufv(buffer.data(), buffer.size()));
	
	pkt.control_type(ctrl_type::ACK);
	REQUIRE(pkt.control_type() == ctrl_type::ACK);
}
