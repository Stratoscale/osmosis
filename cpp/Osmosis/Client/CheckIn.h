#ifndef __OSMOSIS_CLIENT_CHECK_IN_H__
#define __OSMOSIS_CLIENT_CHECK_IN_H__

#include "Osmosis/Client/DigestDirectory.h"
#include "Osmosis/Client/CheckExistingThread.h"
#include "Osmosis/Client/PutThread.h"
#include "Osmosis/Client/LabelOps.h"
#include "Osmosis/Stream/BufferToSocket.h"
#include "Common/NumberOfCPUs.h"

namespace Osmosis {
namespace Client
{

class CheckIn
{
public:
	CheckIn(        const boost::filesystem::path &  directory,
			const std::string &              label,
			const std::string &              hostname,
			unsigned short                   port,
			bool                             md5 ) :
		_label( label ),
		_md5( md5 ),
		_digestDirectory( directory, md5 ),
		_putConnection( hostname, port ),
		_putQueue( CHECK_EXISTING_THREADS )
	{
		for ( unsigned i = 0; i < CHECK_EXISTING_THREADS; ++ i )
			_threads.push_back( std::thread(
				CheckExistingThread::task, std::ref( _digestDirectory.digestedQueue() ), std::ref( _putQueue ), hostname, port ) );
		_threads.push_back( std::thread( PutThread::task, std::ref( _putQueue ), std::ref( _putConnection ) ) );
	}

	void go()
	{
		_digestDirectory.join();
		for ( auto & i : _threads )
			i.join();
		Hash hash = putDirList();
		LabelOps( _putConnection.socket() ).set( hash, _label );
	}

private:
	enum {
		CHECK_EXISTING_THREADS = 10,
	};

	const std::string              _label;
	const bool                     _md5;
	DigestDirectory                _digestDirectory;
	Connect                        _putConnection;
	DigestedTaskQueue              _putQueue; 
	std::vector< std::thread >     _threads; 

	Hash putDirList()
	{
		std::ostringstream out;
		out << _digestDirectory.dirList();
		std::string text( out.str() );
		Hash hash = CalculateHash::SHA1( text.c_str(), text.size() );
		struct Tongue::Header header = { static_cast< unsigned char >( Tongue::Opcode::PUT ) };
		_putConnection.socket().sendAllConcated( header, hash.raw() );
		Stream::BufferToSocket transfer( text.c_str(), text.size(), _putConnection.socket() );
		transfer.transfer();
		TRACE_DEBUG( "Transferred dirList" );
		return hash;
	}

	static unsigned digestionThreads() { return numberOfCPUs() + 1; }

	CheckIn( const CheckIn & rhs ) = delete;
	CheckIn & operator= ( const CheckIn & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_CHECK_IN_H__
