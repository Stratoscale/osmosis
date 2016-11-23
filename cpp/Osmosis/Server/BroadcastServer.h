#ifndef __OSMOSIS_BROADCAST_SERVER_SERVER_H__
#define __OSMOSIS_BROADCAST_SERVER_SERVER_H__

#include <boost/system/error_code.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/array.hpp>
#include <boost/asio/placeholders.hpp>
#include "Osmosis/Tongue.h"
#include "Osmosis/ObjectStore/Labels.h"

namespace Osmosis {
namespace Server
{

class BroadcastServer
{
public:

	BroadcastServer( boost::asio::io_service & ioService,
	                 boost::filesystem::path & rootPath,
	                 unsigned short port );

private:
	enum {
		MAX_LABEL_SIZE = 1024,
		HEADER_BUFFER_SIZE = sizeof( Tongue::Header ),
		MESSAGE_BUFFER_SIZE = MAX_LABEL_SIZE,
		RECEIVE_BUFFER_SIZE = HEADER_BUFFER_SIZE + MESSAGE_BUFFER_SIZE
	};

	void asyncReceive();

	void receiveHandler( const boost::system::error_code& error, std::size_t );

	void sendHandler( const boost::system::error_code& error, std::size_t,
	                  boost::asio::ip::udp::endpoint remoteEndpoint );

	void doesLabelExist( boost::asio::ip::udp::endpoint remoteEndpoint,
	                     unsigned char * message,
	                     std::size_t messageSize );

	static const int nrHandlers = static_cast< int > ( Tongue::Opcode::MAX_OPCODE );
	typedef void( BroadcastServer::*CommandHandler )( boost::asio::ip::udp::endpoint,
	                                                  unsigned char *,
	                                                  std::size_t );
	typedef void( BroadcastServer::*SocketHandler )( const boost::system::error_code& , std::size_t );
	CommandHandler                                     _commandHandlers[nrHandlers];
	Tongue::Opcode                            _opcode;
	boost::array< unsigned char, RECEIVE_BUFFER_SIZE > _messageBuffer;
	boost::asio::ip::udp::socket                       _socket;
	boost::asio::ip::udp::endpoint                     _remoteEndpoint;
	Osmosis::ObjectStore::Labels                       _labels;
	ObjectStore::Store                                 _store;
	std::array< unsigned char, RECEIVE_BUFFER_SIZE >   _recvBuffer;
};

} // namespace Server
} // namespace Osmosis

#endif // __OSMOSIS_BROADCAST_SERVER_SERVER_H__
