#include <string>
#include <chrono>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include "Osmosis/Client/WhoHasLabel.h"
#include "Osmosis/Tongue.h"
#include "Common/Error.h"

namespace Osmosis {
namespace Client
{

WhoHasLabel::WhoHasLabel( const std::string & label, unsigned short timeoutInMilliseconds,
                          unsigned short remoteListeningPort, bool broadcastToLocalhost ):
	_label( label ),
	_timeoutInMilliseconds( timeoutInMilliseconds ),
	_localListeningPort( 2015 ),
	_remoteListeningPort( remoteListeningPort ),
	_socket( _localListeningPort, broadcastToLocalhost )
{}

void WhoHasLabel::broadcastRequest()
{
	BACKTRACE_BEGIN
	// TODO: Make labelops usable with UDP sockets to use it instead of the following logic
	static Tongue::Header * header = reinterpret_cast< Tongue::Header * > ( & _buffer );
	struct Tongue::Label * labelHeader = reinterpret_cast< struct Tongue::Label * >( header + 1 );
	unsigned char * labelText = reinterpret_cast< unsigned char * >( labelHeader + 1 );
	size_t size = reinterpret_cast< unsigned char * >( labelText + _label.size() ) - _buffer;
	if ( size > sizeof( _buffer ) )
		THROW( Error, "Label too long" );
	header->opcode = static_cast< unsigned char >( Tongue::Opcode::WHO_HAS_LABEL );
	labelHeader->length = static_cast< unsigned short >( _label.size() );
	memcpy( labelText, _label.c_str(), _label.size() );
	_socket.broadcast( const_cast< const unsigned char * > (
	    reinterpret_cast< unsigned char * > ( & _buffer ) ), size, _remoteListeningPort );
	BACKTRACE_END
}

std::list< std::string > WhoHasLabel::receiveObjectStores()
{
	BACKTRACE_BEGIN
	std::list< std::string > objectStores;
	std::chrono::duration< double > timeToListen( _timeoutInMilliseconds / 1000.0 );
	std::chrono::duration< double > elapsedTime( 0 );
	std::chrono::time_point<std::chrono::system_clock> startTime = std::chrono::system_clock::now();
	std::list< std::string > result;
	do {
		int timeoutInMilliseconds = ( timeToListen - elapsedTime ).count() * 1000;
		if ( 0 == timeoutInMilliseconds )
			break;
		boost::system::error_code ec;
		boost::asio::ip::udp::endpoint remoteEndpoint;
		_socket.receive( _buffer, sizeof( _buffer ), remoteEndpoint, timeoutInMilliseconds, ec );
		if ( ec ==  boost::system::errc::success )
			result.push_back( boost::lexical_cast<std::string>( remoteEndpoint ) );
		else if ( ec != boost::asio::error::operation_aborted )
			THROW( Error, "An error has occurred while receiving data. Code: " << ec );
		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
		elapsedTime = now - startTime;
	} while ( elapsedTime.count() < timeToListen.count() );
	return result;
	BACKTRACE_END
}

std::list< std::string > WhoHasLabel::go()
{
	BACKTRACE_BEGIN
	broadcastRequest();
	return receiveObjectStores();
	BACKTRACE_END
}

} // namespace Client
} // namespace Osmosis
