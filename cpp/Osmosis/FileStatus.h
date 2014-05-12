#ifndef __OSMOSIS_FILE_STATUS_H__
#define __OSMOSIS_FILE_STATUS_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <boost/algorithm/string/trim.hpp>
#include "Common/SplitString.h"

namespace Osmosis
{

class FileStatus
{
public:
	FileStatus()
	{
		memset( & _stat, 0, sizeof( _stat ) );
	}

	FileStatus( const boost::filesystem::path & path )
	{
		int result = lstat( path.string().c_str(), & _stat );
		if ( result != 0 )
			THROW_BOOST_ERRNO_EXCEPTION( errno, "Unable to stat " << path );
		if ( isSymlink() )
			_symlink = boost::filesystem::read_symlink( path );
	}

	FileStatus( const std::string & serialized )
	{
		try {
			fromString( serialized );
		} catch ( boost::exception & e ) {
			THROW( Error, "'" << serialized << "' is in an invalid format for a file status: " <<
					boost::diagnostic_information( e ) );
		}
	}

	const boost::filesystem::path symlink() const
	{
		ASSERT( isSymlink() );
		return _symlink;
	}

	bool syncContent() const
	{
		if ( isDirectory() or isCharacter() or isBlock() or
				isFIFO() or isSymlink() or isSocket() )
			return false;
		ASSERT( isRegular() );
		return true;
	}

	bool isRegular() const { return S_ISREG( _stat.st_mode ); }
	bool isDirectory() const { return S_ISDIR( _stat.st_mode ); }
	bool isCharacter() const { return S_ISCHR( _stat.st_mode ); }
	bool isBlock() const { return S_ISBLK( _stat.st_mode ); }
	bool isFIFO() const { return S_ISFIFO( _stat.st_mode ); }
	bool isSymlink() const { return S_ISLNK( _stat.st_mode ); }
	bool isSocket() const { return S_ISSOCK( _stat.st_mode ); }

	mode_t mode() const { return _stat.st_mode; }
	mode_t type() const { return _stat.st_mode & S_IFMT; }
	dev_t dev() const { return _stat.st_rdev; }
	uid_t uid() const { return _stat.st_uid; }
	gid_t gid() const { return _stat.st_gid; }
	time_t mtime() const { return _stat.st_mtime; }

	void setUIDGID( uid_t uid, gid_t gid )
	{
		_stat.st_uid = uid;
		_stat.st_gid = gid;
	}

	bool operator == ( const FileStatus & other ) const
	{
		return _stat.st_mode == other._stat.st_mode and
			_stat.st_uid == other._stat.st_uid and
			_stat.st_gid == other._stat.st_gid and
			_stat.st_rdev == other._stat.st_rdev and
//			_stat.st_atime == other._stat.st_atime and
			( isDirectory() or _stat.st_mtime == other._stat.st_mtime ) and
//			_stat.st_ctime == other._stat.st_ctime and
			_symlink == other._symlink;
	}
	bool operator != ( const FileStatus & other ) const { return not operator == ( other ); }

	friend std::ostream & operator << ( std::ostream & os, const FileStatus & status )
	{
		os <<
			status._stat.st_mode << '*' <<
			status._stat.st_uid << '*' <<
			status._stat.st_gid << '*' <<
			status._stat.st_rdev << '*' <<
//			status._stat.st_atime << '*' <<
			status._stat.st_mtime << '*' <<
//			status._stat.st_ctime << '*' <<
			status._symlink;
		return os;
	}

private:
	struct stat              _stat;
	boost::filesystem::path  _symlink;

	void fromString( const std::string & serialized )
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

};

} // namespace Osmosis

#endif // __OSMOSIS_FILE_STATUS_H__
