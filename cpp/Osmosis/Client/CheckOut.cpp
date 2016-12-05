#include <thread>
#include "boost/filesystem.hpp"
#include "Osmosis/Client/CheckOut.h"
#include "Osmosis/Client/DelayedLabels.h"
#include "Osmosis/Client/FetchJointDirlistFromLabels.h"
#include "Osmosis/ApplyFileStatus.h"
#include "Osmosis/OSUtils.h"
#include "Osmosis/FileStatus.h"
#include "Common/Error.h"

namespace Osmosis {
namespace Client
{

CheckOut::CheckOut(       const boost::filesystem::path &  directory,
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

void CheckOut::go()
{
    try {
        _go();
    } CATCH_ALL( "Stopping progress thread",
        _checkOutProgress.stop();
        throw;
    );
}

void CheckOut::_go()
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

void CheckOut::removeUnknownFiles( const DirList & digested, const DirList & label, const FetchFiles & fetchFiles )
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

			if ( isFileInValidCondition(absolute ) ) {
				if ( entry.path == leftOversFromPreviousFailedOsmosisAttemptThatWillAnywaysBeErased and
						startsWith( entry.path.string(), leftOversFromPreviousFailedOsmosisAttemptThatWillAnywaysBeErasedPrefix ) )
					continue;
				if ( _ignores.parentOfAnIgnored( absolute ) )
					continue;

				if ( not boost::filesystem::exists( absolute ) and not boost::filesystem::symbolic_link_exists( absolute ) )
					continue;
				if ( areOneOrMoreAncestorsSymlinks( entry.path ) )
					continue;
			} else
				TRACE_WARNING( "Remove dangling file '" << absolute << "'" );

			try {
				boost::filesystem::remove_all( absolute );
			} catch ( boost::filesystem::filesystem_error &ex ) {
				if ( ex.code().value() == ENAMETOOLONG ) {
					TRACE_WARNING("Will not remove a directory which either has a path or "
									"contains a path too long:'" << relative << "'");
					continue;
				}
				TRACE_BOOST_EXCEPTION( ex, "While removing unknown files" );
				throw;
			} CATCH_TRACEBACK_EXCEPTION
		}
	BACKTRACE_END
}

void CheckOut::decideWhatToDo(    FetchFiles &                     fetchFiles,
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

} // namespace Client
} // namespace Osmosis
