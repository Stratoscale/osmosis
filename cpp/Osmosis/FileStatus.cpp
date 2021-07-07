#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include "Osmosis/FileStatus.h"
#include "Common/SplitString.h"
#include "Common/Error.h"

namespace Osmosis
{

FileStatus::FileStatus()
{
	memset( & _stat, 0, sizeof( _stat ) );
}

FileStatus::FileStatus( const boost::filesystem::path & path )
{
	int result = lstat( path.string().c_str(), & _stat );
	if ( result != 0 )
		THROW_BOOST_ERRNO_EXCEPTION( errno, "Unable to stat " << path );
	if ( isSymlink() )
		_symlink = boost::filesystem::read_symlink( path );
}

FileStatus::FileStatus( const std::string & serialized )
{
	try {
		fromString( serialized );
	} catch ( boost::exception & e ) {
		THROW( Error, "'" << serialized << "' is in an invalid format for a file status: " <<
				boost::diagnostic_information( e ) );
	}
}

const boost::filesystem::path FileStatus::symlink() const
{
	ASSERT( isSymlink() );
	return _symlink;
}

bool FileStatus::syncContent( bool followSymlinks ) const
{
	if ( isDirectory() or isCharacter() or isBlock() or
	     isFIFO() or isSocket() or ( isSymlink() and not followSymlinks ) )
		return false;
	ASSERT( isRegular() );
	return true;
}

bool FileStatus::isRegular() const { return S_ISREG( _stat.st_mode ); }
bool FileStatus::isDirectory() const { return S_ISDIR( _stat.st_mode ); }
bool FileStatus::isCharacter() const { return S_ISCHR( _stat.st_mode ); }
bool FileStatus::isBlock() const { return S_ISBLK( _stat.st_mode ); }
bool FileStatus::isFIFO() const { return S_ISFIFO( _stat.st_mode ); }
bool FileStatus::isSymlink() const { return S_ISLNK( _stat.st_mode ); }
bool FileStatus::isSocket() const { return S_ISSOCK( _stat.st_mode ); }

mode_t FileStatus::mode() const { return _stat.st_mode; }
mode_t FileStatus::type() const { return _stat.st_mode & S_IFMT; }
dev_t FileStatus::dev() const { return _stat.st_rdev; }
uid_t FileStatus::uid() const { return _stat.st_uid; }
gid_t FileStatus::gid() const { return _stat.st_gid; }
time_t FileStatus::mtime() const { return _stat.st_mtime; }

void FileStatus::setUIDGID( uid_t uid, gid_t gid )
{
	_stat.st_uid = uid;
	_stat.st_gid = gid;
}

bool FileStatus::equalsButTime( const FileStatus & other ) const
{
	return _stat.st_mode == other._stat.st_mode and
		_stat.st_uid == other._stat.st_uid and
		_stat.st_gid == other._stat.st_gid and
		_stat.st_rdev == other._stat.st_rdev and
		_symlink == other._symlink;
}

bool FileStatus::operator == ( const FileStatus & other ) const
{
	return equalsButTime( other ) and
//			_stat.st_atime == other._stat.st_atime and
		( not isRegular() or _stat.st_mtime == other._stat.st_mtime );
//			_stat.st_ctime == other._stat.st_ctime and
}

bool FileStatus::operator != ( const FileStatus & other ) const { return not operator == ( other ); }

void FileStatus::fromString( const std::string & serialized )
{
	SplitString split( serialized, '*' );
	_stat.st_mode = boost::lexical_cast< mode_t >( split.asString() );
	split.next();
	if ( split.done() )
		THROW( Error, "'" << serialized << "' is in an invalid format for a file status" );
	_stat.st_uid = boost::lexical_cast< uid_t >( split.asString() );
	split.next();
	if ( split.done() )
		THROW( Error, "'" << serialized << "' is in an invalid format for a file status" );
	_stat.st_gid = boost::lexical_cast< gid_t >( split.asString() );
	split.next();
	if ( split.done() )
		THROW( Error, "'" << serialized << "' is in an invalid format for a file status" );
	_stat.st_rdev = boost::lexical_cast< dev_t >( split.asString() );
	split.next();
	if ( split.done() )
		THROW( Error, "'" << serialized << "' is in an invalid format for a file status" );
//		_stat.st_atime = boost::lexical_cast< time_t >( split.asString() );
	_stat.st_mtime = boost::lexical_cast< time_t >( split.asString() );
	split.next();
	if ( split.done() )
		THROW( Error, "'" << serialized << "' is in an invalid format for a file status" );
//		_stat.st_ctime = boost::lexical_cast< time_t >( split.asString() );
	_symlink = boost::trim_copy_if( split.asString(), boost::is_any_of( "\"" ) );
	split.next();
	if ( not split.done() )
		THROW( Error, "'" << serialized << "' is in an invalid format for a file status" );
}

} // namespace Osmosis
