#ifndef __OSMOSIS_CLIENT_CHECK_OUT_H__
#define __OSMOSIS_CLIENT_CHECK_OUT_H__

#include "Osmosis/Stream/SocketToBuffer.h"
#include "Osmosis/Client/FetchFiles.h"
#include "Osmosis/Client/DelayedLabels.h"
#include "Osmosis/Client/FetchJointDirlistFromLabels.h"
#include "Osmosis/Client/CheckOutProgress.h"
#include "Osmosis/ApplyFileStatus.h"
#include "Osmosis/OSUtils.h"

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
			bool                             chainTouch ) :
		_directory( directory ),
		_labels( label ),
		_chain( chain ),
		_removeUnknownFiles( removeUnknownFiles ),
		_myUIDandGIDcheckout( myUIDandGIDcheckout ),
		_ignores( ignores ),
		_digestDirectory( directory, md5, ignores ),
		_checkOutProgress( progressReport, _digestDirectory, progressReportIntervalSeconds ),
		_chainTouch( chainTouch )
	{}

	void go()
	{
		BACKTRACE_BEGIN
		_labels.fetch();
		DirList labelsDirList( FetchJointDirlistFromLabels( _labels.labels(), _chain, _chainTouch ).joined() );
		_digestDirectory.join();

		try {
			FetchFiles fetchFiles( _directory, _chain );
			_checkOutProgress.setFetchFiles( fetchFiles );
			for ( auto & entry : labelsDirList.entries() )
				if ( _myUIDandGIDcheckout ) {
					FileStatus modifiedStatus( entry.status );
					modifiedStatus.setUIDGID( OSUtils::uid(), OSUtils::gid() );
					decideWhatToDo( fetchFiles,
							entry.path,
							modifiedStatus,
							entry.hash.get(),
							labelsDirList );
				} else
					decideWhatToDo( fetchFiles,
							entry.path,
							entry.status,
							entry.hash.get(),
							labelsDirList );
			fetchFiles.noMoreFilesToFetch();
			removeUnknownFiles( _digestDirectory.dirList(), labelsDirList, fetchFiles );
			fetchFiles.join();
			_checkOutProgress.stop();
		} CATCH_ALL( "Dumping dirlists", dumpDirLists( labelsDirList, _digestDirectory.dirList() ); throw; );
		_checkOutProgress.stop();
		TRACE_DEBUG( "Checkout Complete" );
		BACKTRACE_END
	}

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

	void removeUnknownFiles( const DirList & digested, const DirList & label, const FetchFiles & fetchFiles )
	{
		BACKTRACE_BEGIN
		if ( not _removeUnknownFiles )
			return;
		boost::filesystem::path leftOversFromPreviousFailedOsmosisAttemptThatWillAnywaysBeErased = fetchFiles.draftsPath();
		std::string leftOversFromPreviousFailedOsmosisAttemptThatWillAnywaysBeErasedPrefix = leftOversFromPreviousFailedOsmosisAttemptThatWillAnywaysBeErased.string();
		for ( auto & entry : digested.entries() )
			if ( label.find( entry.path ) == nullptr ) {
				boost::filesystem::path absolute = _directory / entry.path;
				std::string relative = entry.path.string();
				if ( entry.path == leftOversFromPreviousFailedOsmosisAttemptThatWillAnywaysBeErased and
						startsWith( entry.path.string(), leftOversFromPreviousFailedOsmosisAttemptThatWillAnywaysBeErasedPrefix ) )
					continue;
				if ( _ignores.parentOfAnIgnored( absolute ) )
					continue;
				if ( not boost::filesystem::exists( absolute ) and not boost::filesystem::symbolic_link_exists( absolute ) )
					continue;
				if ( areOneOrMoreAncestorsSymlinks( entry.path ) )
					continue;
				try {
					boost::filesystem::remove_all( absolute );
				} CATCH_TRACEBACK_EXCEPTION
			}
		BACKTRACE_END
	}

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

	void decideWhatToDo(    FetchFiles &                     fetchFiles,
				const boost::filesystem::path &  path,
				const FileStatus &               status,
				const Hash *                     hash,
				const DirList &                  labelDirList )
	{
		boost::filesystem::path absolute = _directory / path;
		auto digestedEntry = _digestDirectory.dirList().find( path );
		if ( digestedEntry == nullptr ) {
			if ( status.syncContent() ) {
				if ( hash == nullptr )
					THROW( Error, "No hash for file that should have data - directory listing is defective" );
				fetchFiles.fetch( path, status, * hash );
			} else
				ApplyFileStatus( absolute, status ).createNonRegular();
		} else {
			if ( status.syncContent() ) {
				if ( digestedEntry->status.syncContent() ) {
					if ( hash == nullptr )
						THROW( Error, "No hash for file that should have data - directory listing is defective" );
					if ( ! digestedEntry->hash )
						THROW( Error, "No hash for file that should have data - directory listing is defective" );
					if ( * hash != * digestedEntry->hash )
						fetchFiles.fetch( path, status, * hash );
					else if ( status != digestedEntry->status ) {
						ApplyFileStatus( absolute, status ).applyExistingRegular();
						ASSERT_VERBOSE( FileStatus( absolute ) == status,
								absolute << ": " << FileStatus( absolute ) << " != " << status );
					}
				} else {
					if ( hash == nullptr )
						THROW( Error, "No hash for file that should have data - directory listing is defective" );
					try {
						boost::filesystem::remove_all( absolute );
					} CATCH_TRACEBACK_EXCEPTION
					fetchFiles.fetch( path, status, * hash );
				}
			} else {
				if ( status != digestedEntry->status )
					ApplyFileStatus( absolute, status ).applyNonRegular( digestedEntry->status );
			}
		}
	}

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

	CheckOut( const CheckOut & rhs ) = delete;
	CheckOut & operator= ( const CheckOut & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_CHECK_OUT_H__
