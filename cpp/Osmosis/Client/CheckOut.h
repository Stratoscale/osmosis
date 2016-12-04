#ifndef __OSMOSIS_CLIENT_CHECK_OUT_H__
#define __OSMOSIS_CLIENT_CHECK_OUT_H__

#include "Osmosis/Stream/SocketToBuffer.h"
#include "Osmosis/Client/FetchFiles.h"
#include "Osmosis/Client/DelayedLabels.h"
#include "Osmosis/Client/CheckOutProgress.h"

namespace Osmosis {
namespace Client
{

class CheckOut
{
public:
	CheckOut(       const boost::filesystem::path &  directory,
			const std::string &              label,
			Chain::Chain &                   chain,
			bool                             md5,
			bool                             removeUnknownFiles,
			bool                             myUIDandGIDcheckout,
			const Ignores &                  ignores,
			const boost::filesystem::path &  progressReport,
			unsigned                         progressReportIntervalSeconds,
			bool                             chainTouch );

	void go();

private:
	const boost::filesystem::path  _directory;
	DelayedLabels                  _labels;
	Chain::Chain &                 _chain;
	const bool                     _removeUnknownFiles;
	const bool                     _myUIDandGIDcheckout;
	const Ignores &                _ignores;
	DigestDirectory                _digestDirectory;
	CheckOutProgress               _checkOutProgress;
	bool                           _chainTouch;

	void removeUnknownFiles( const DirList & digested, const DirList & label, const FetchFiles & fetchFiles );

	inline bool areOneOrMoreAncestorsSymlinks( const boost::filesystem::path entry ) const
	{
		boost::filesystem::path closestAncestor = entry.parent_path();
		boost::filesystem::path curAncestor = _directory;
		for (auto & component : closestAncestor) {
			curAncestor /= component;
			const FileStatus fileStatus( const_cast< const boost::filesystem::path& > ( curAncestor ) );
			if ( fileStatus.isSymlink() ) {
				return true;
			}
		}
		return false;
	}

	inline bool fileInValidCondition( const boost::filesystem::path entry ) const
	{
		boost::system::error_code file_error;
		boost::filesystem::file_status status = boost::filesystem::status(entry, file_error);

		return status != boost::filesystem::file_status(boost::filesystem::status_error);
	}

	void decideWhatToDo(    FetchFiles &                     fetchFiles,
				const boost::filesystem::path &  path,
				const FileStatus &               status,
				const Hash *                     hash,
				const DirList &                  labelDirList );

	static bool startsWith( const std::string & str, const std::string & prefix )
	{
		return str.size() >= prefix.size() and memcmp( str.c_str(), prefix.c_str(), prefix.size() ) == 0;
	}

	static void dumpDirLists( const DirList & labelsDirList, const DirList & digestedDirlist )
	{
		BACKTRACE_BEGIN
		try {
			std::ofstream labelsDirListFile( "/tmp/labels.dirList" );
			labelsDirListFile << labelsDirList;
			std::ofstream digestedDirListFile( "/tmp/digested.dirList" );
			digestedDirListFile << digestedDirlist;
		} CATCH_ALL_IGNORE( "Unable to dump dirlists" );
		BACKTRACE_END
	}

	void _go();

	CheckOut( const CheckOut & rhs ) = delete;
	CheckOut & operator= ( const CheckOut & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_CHECK_OUT_H__
