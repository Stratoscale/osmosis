#ifndef __OSMOSIS_OBJECT_STORE_LABEL_LOG_APPENDER_H__
#define __OSMOSIS_OBJECT_STORE_LABEL_LOG_APPENDER_H__

#include "Osmosis/ObjectStore/LabelLogEntry.h"
#include "Osmosis/Debug.h"

namespace Osmosis {
namespace ObjectStore
{

class LabelLogAppender
{
public:
	LabelLogAppender( const boost::filesystem::path & rootPath ) :
		_labelLogsPath( rootPath / DirectoryNames::LABEL_LOG )
	{
		if ( not boost::filesystem::is_directory( _labelLogsPath ) )
			boost::filesystem::create_directories( _labelLogsPath );
	}

	~LabelLogAppender()
	{
		try {
			write();
		} CATCH_ALL_IGNORE( "Unable to write label log on object store closing" );
	}

	void set(const std::string & label, const Hash & hash)
	{
		append(LabelLogEntry::SET, label, hash);
	}

	void get(const std::string & label, const Hash & hash)
	{
		append(LabelLogEntry::GET, label, hash);
	}

	void remove(const std::string & label, const Hash & hash)
	{
		append(LabelLogEntry::REMOVE, label, hash);
	}

	void write()
	{
		if ( _log.size() == 0 )
			return;
		Log temp;
		{
			std::lock_guard< std::mutex > lock( _logLock );
			temp.splice(temp.begin(), _log);
		}
		boost::filesystem::path logFilePath = _labelLogsPath / logBasename();
		std::ofstream output( logFilePath.string() );
		for ( auto & entry : temp )
			output << entry;
	} 

private:
	typedef std::list< LabelLogEntry > Log;

	boost::filesystem::path  _labelLogsPath;
	Log                      _log;
	std::mutex               _logLock;

	void append( LabelLogEntry::Operation operation, const std::string & label, const Hash & hash )
	{
		Log temp;
		temp.emplace_back( operation, label, hash );
		std::lock_guard< std::mutex > lock( _logLock );
		_log.splice(_log.begin(), temp);
	}

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
