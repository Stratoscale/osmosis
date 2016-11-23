#ifndef __OSMOSIS_UDPSOCKET_H__
#define __OSMOSIS_UDPSOCKET_H__

#include <boost/asio.hpp>

namespace Osmosis {

class UDPSocket
{
public:
	UDPSocket( unsigned short listeningPort, bool bindAsLocalhost );

	std::size_t receive( unsigned char *buffer, size_t bufferSize,
	                     boost::asio::ip::udp::endpoint & remoteEndpoint,
	                     int timeout, boost::system::error_code& ec);

	void broadcast( const unsigned char * data, size_t size, unsigned short port );

private:
	void checkDeadline();

	void handleReceive( const boost::system::error_code ec, std::size_t length,
	                    boost::system::error_code* out_ec, std::size_t* out_length );

	void handleSend( const boost::system::error_code ec, std::size_t nrBytesTransferred );

	boost::asio::io_service _ioService;
	boost::asio::ip::udp::socket _socket;
	boost::asio::deadline_timer _deadline;
	boost::asio::ip::udp::endpoint _listeningEndpoint;
};

} // namespace Osmosis

#endif
