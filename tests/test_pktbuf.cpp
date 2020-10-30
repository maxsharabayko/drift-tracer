#include "catch2/catch.hpp"

#include "buf_view.hpp"
#include "packet/pkt_view.hpp"

TEST_CASE("Packet buffer", "[pktbuf]")
{
	pkt_view const_bufv(const_bufv(nullptr, 0));
	//const_buf.set(5);

	pkt_view mut_buf(mut_bufv(nullptr, 0));
	//mut_buf.set(5);
}
