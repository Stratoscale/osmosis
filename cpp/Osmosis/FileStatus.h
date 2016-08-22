#ifndef __OSMOSIS_FILE_STATUS_H__
#define __OSMOSIS_FILE_STATUS_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include "Common/SplitString.h"

namespace Osmosis
{

class FileStatus
{
public:
	FileStatus();

	FileStatus( const boost::filesystem::path & path );

	FileStatus( const std::string & serialized );

	const boost::filesystem::path symlink() const;

	bool syncContent() const;

	bool isRegular() const;
	bool isDirectory() const;
	bool isCharacter() const;
	bool isBlock() const;
	bool isFIFO() const;
	bool isSymlink() const;
	bool isSocket() const;

	mode_t mode() const;
	mode_t type() const;
	dev_t dev() const;
	uid_t uid() const;
	gid_t gid() const;
	time_t mtime() const;

	void setUIDGID( uid_t uid, gid_t gid );

	bool equalsButTime( const FileStatus & other ) const;

	bool operator == ( const FileStatus & other ) const;

	bool operator != ( const FileStatus & other ) const;

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

	void fromString( const std::string & serialized );

};

} // namespace Osmosis

#endif // __OSMOSIS_FILE_STATUS_H__
