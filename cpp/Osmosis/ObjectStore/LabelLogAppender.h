#ifndef __OSMOSIS_OBJECT_STORE_LABEL_LOG_APPENDER_H__
#define __OSMOSIS_OBJECT_STORE_LABEL_LOG_APPENDER_H__

#include <string>
#include <boost/filesystem/path.hpp>
#include <boost/asio/ip/host_name.hpp>
#include "Osmosis/ObjectStore/LabelLogEntry.h"
#include "Osmosis/ObjectStore/MakeDirectory.h"
#include "Osmosis/ObjectStore/DirectoryNames.h"
#include "Osmosis/FileDescriptor.h"
#include "Osmosis/Debug.h"

namespace Osmosis {
namespace ObjectStore
{

class LabelLogAppender
{
public:
	LabelLogAppender( const boost::filesystem::path & rootPath );

	~LabelLogAppender();

	void set(const std::string & label, const Hash & hash);

	void get(const std::string & label, const Hash & hash);

	void remove(const std::string & label, const Hash & hash);

	void write();

private:
	enum { MAXIMUM_LOG_TO_KEEP_IN_MEMORY = 100 };

	typedef std::list< LabelLogEntry > Log;

	boost::filesystem::path  _labelLogsPath;
	Log                      _log;
	std::mutex               _logLock;
	MakeDirectory            _makeDirectory;

	void append( LabelLogEntry::Operation operation, const std::string & label, const Hash & hash );

	static std::string randomHex()
	{
		unsigned char buffer[10];
		FileDescriptor urandom( FileDescriptor::openForRead( "/dev/urandom" ) );
		ssize_t bytesRead = read( urandom.fd(), buffer, sizeof( buffer ) );
		if ( bytesRead != sizeof( buffer ) )
			THROW_BOOST_ERRNO_EXCEPTION( errno, "Unable to read from urandom" );
		char output[21];
		Hex::fromBuffer( buffer, sizeof( buffer ), output, sizeof( output ) );
		return std::move( std::string( output, sizeof( buffer ) * ( sizeof( "XX" ) - sizeof( '\0' ) ) ) );
	}

	static std::string logBasename()
	{
		std::time_t now = std::time( nullptr );
		std::string hostname( boost::asio::ip::host_name() );
		return std::to_string( now ) + "__" + hostname + "__" + randomHex();
	}

	LabelLogAppender( const LabelLogAppender & rhs ) = delete;
	LabelLogAppender & operator= ( const LabelLogAppender & rhs ) = delete;
};

} // namespace ObjectStore
} // namespace Osmosis

#endif // __OSMOSIS_OBJECT_STORE_LABEL_LOG_APPENDER_H__
