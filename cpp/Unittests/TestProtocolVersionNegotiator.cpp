#include <iostream>
#include <boost/asio.hpp>
#include <Osmosis/Tongue.h>
#include <cxxtest/TestSuite.h>
#include <Osmosis/TCPConnection.h>
#include <Osmosis/Stream/AckOps.h>
#include <Osmosis/Chain/Remote/ProtocolVersionNegotiator.h>

std::mutex globalTraceLock;

namespace Osmosis {
namespace Stream
{

AckOps::AckOps( TCPSocket & socket ):
	_socket( socket )
{}

void AckOps::wait( const char * waitReason )
{
}

void AckOps::sendAck()
{
}

}
}

class ProtocolVersionNegotiatorTest : public CxxTest::TestSuite
{
public:
    void test_ProtocolNegotiation(void)
    {
		boost::asio::io_service ioService;
		boost::asio::ip::tcp::socket boostSocket(ioService);
    }
};
