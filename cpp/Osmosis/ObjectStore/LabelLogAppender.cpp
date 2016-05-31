#include <fstream>
#include <time.h>
#include <string>
#include <boost/thread/lock_guard.hpp>
#include <boost/filesystem/path.hpp>
#include <Osmosis/ObjectStore/LabelLogAppender.h>
#include <Osmosis/ObjectStore/LabelLogEntry.h>
#include <Osmosis/ObjectStore/DirectoryNames.h>
#include <Osmosis/Debug.h>
#include <Osmosis/FileDescriptor.h>
#include <Osmosis/Hex.h>

namespace Osmosis {
namespace ObjectStore
{

LabelLogAppender::LabelLogAppender( const boost::filesystem::path & rootPath ) :
	_labelLogsPath( rootPath / DirectoryNames::LABEL_LOG ),
	_makeDirectory( rootPath / DirectoryNames::LABEL_LOG )
{}

LabelLogAppender::~LabelLogAppender()
{
	try {
		write();
	} CATCH_ALL_IGNORE( "Unable to write label log on object store closing" );
}

void LabelLogAppender::set(const std::string & label, const Hash & hash)
{
	append(LabelLogEntry::SET, label, hash);
}

void LabelLogAppender::get(const std::string & label, const Hash & hash)
{
	append(LabelLogEntry::GET, label, hash);
}

void LabelLogAppender::remove(const std::string & label, const Hash & hash)
{
	append(LabelLogEntry::REMOVE, label, hash);
}

void LabelLogAppender::write()
{
	if ( _log.size() == 0 )
		return;
	Log temp;
	{
		std::lock_guard< std::mutex > lock( _logLock );
		temp.splice(temp.begin(), _log);
	}
	_makeDirectory.makeSureExists();
	boost::filesystem::path logFilePath = _labelLogsPath / logBasename();
	std::ofstream output( logFilePath.string() );
	for ( auto & entry : temp )
		output << entry;
}

void LabelLogAppender::append( LabelLogEntry::Operation operation, const std::string & label, const Hash & hash )
{
	Log temp;
	temp.emplace_back( operation, label, hash );
	{
		std::lock_guard< std::mutex > lock( _logLock );
		_log.splice(_log.begin(), temp);
	}
	if ( _log.size() > MAXIMUM_LOG_TO_KEEP_IN_MEMORY )
		write();
}

} // namespace ObjectStore
} // namespace Osmosis
