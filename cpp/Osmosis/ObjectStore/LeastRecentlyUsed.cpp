#include <unordered_set>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <Osmosis/ObjectStore/LeastRecentlyUsed.h>
#include <Osmosis/ObjectStore/LabelLogIterator.h>
#include <Osmosis/DirListEntry.h>
#include "Osmosis/Debug.h"

namespace Osmosis {
namespace ObjectStore
{

LeastRecentlyUsed::LeastRecentlyUsed(      const boost::filesystem::path &  rootPath,
			Store &                          store,
			Labels &                         labels,
			const std::string &              keepRegex,
			size_t                           maximumDiskUsage ) :
	_rootPath( rootPath ),
	_store( store ),
	_labels( labels ),
	_keepRegex( keepRegex ),
	_maximumDiskUsage( maximumDiskUsage ),
	_sizeOnDisk( 0 ),
	_startedAt( std::time( nullptr ) )
{}

void LeastRecentlyUsed::go()
{
	BACKTRACE_BEGIN
	keepAllLabelsMatchingKeepRegex();
	TRACE_INFO( "All labels matching keep regex: " << _alreadyProcessedLabels.size() << " on disk: " << _sizeOnDisk );
	keepLabelsFromLabelLogUntilExceedingMaximumDiskUsage();
	TRACE_INFO( "Keeping: " << _alreadyProcessedLabels.size() << " labels, on disk: " << _sizeOnDisk );
	unsigned count = eraseAllNonProcessedLabels();
	TRACE_INFO( "Erased " << count << " non recently used labels" );
	TRACE_INFO( "Scanning excessive objects, keeping " << _keptHashes.size() << " objects" );
	count = eraseAllNonUsedHashes();
	TRACE_INFO( "Erased " << count << " objects" );
	BACKTRACE_END
}

void LeastRecentlyUsed::addToKept( const std::list< Hash > & objects )
{
	for ( auto & hash: objects )
		_keptHashes.emplace( hash );
}

std::list< Hash > LeastRecentlyUsed::newObjectsInLabel( const std::string & label )
{
	BACKTRACE_BEGIN
	std::list< Hash > result;
	Hash hash = _labels.readLabelNoLog( label );
	if ( _keptHashes.find( hash ) == _keptHashes.end() )
		result.emplace_back( hash );

	std::ifstream dirListFile( _store.filenameForExisting( hash ).string() );
	std::string line;
	while ( std::getline( dirListFile, line ) ) {
		Container< Hash > hash;
		DirListEntry::parseOnlyHashFromLine( line, hash );
		if ( not hash.constructed() )
			continue;
		if ( _keptHashes.find( * hash ) == _keptHashes.end() )
			result.emplace_back( * hash );
	}
	return result;
	BACKTRACE_END_VERBOSE( "Label " << label );
}

size_t LeastRecentlyUsed::totalSize( const std::list< Hash > & hashes )
{
	size_t result = 0;
	for ( auto & hash: hashes ) {
		size_t bytes = boost::filesystem::file_size( _store.filenameForExisting( hash ) );
		size_t rounded = ( bytes + BLOCK_SIZE - 1 ) / BLOCK_SIZE * BLOCK_SIZE;
		result += rounded;
	}
	return result;
}

void LeastRecentlyUsed::keepAllLabelsMatchingKeepRegex()
{
	BACKTRACE_BEGIN
	for ( auto label = _labels.list( _keepRegex ); not label.done(); label.next() ) {
		auto newObjects = newObjectsInLabel( * label );
		size_t newObjectsSize = totalSize( newObjects );
		addToKept( newObjects );
		_sizeOnDisk += newObjectsSize;
		_alreadyProcessedLabels.emplace( * label );
	}
	BACKTRACE_END
}

void LeastRecentlyUsed::keepLabelsFromLabelLogUntilExceedingMaximumDiskUsage()
{
	BACKTRACE_BEGIN
	for ( LabelLogIterator iterator( _rootPath ); not iterator.done(); iterator.next() ) {
		const LabelLogEntry & entry = * iterator;
		if ( entry.operation == LabelLogEntry::REMOVE ) {
			_alreadyProcessedLabels.emplace( entry.label );
			continue;
		}
		if ( _alreadyProcessedLabels.find( entry.label ) != _alreadyProcessedLabels.end() )
			continue;

		auto newObjects = newObjectsInLabel( entry.label );
		size_t newObjectsSize = totalSize( newObjects );
		if ( _sizeOnDisk + newObjectsSize > _maximumDiskUsage )
			break;
		addToKept( newObjects );
		_sizeOnDisk += newObjectsSize;
		_alreadyProcessedLabels.emplace( entry.label );
	}
	BACKTRACE_END
}

unsigned LeastRecentlyUsed::eraseAllNonProcessedLabels()
{
	BACKTRACE_BEGIN
	std::list< std::string > toErase;
	for ( auto label = _labels.list( ".*" ); not label.done(); label.next() ) {
		if ( _alreadyProcessedLabels.find( * label ) != _alreadyProcessedLabels.end() )
			continue;
		toErase.emplace_back( * label );
	}
	for ( auto & label: toErase )
		_labels.erase( label );
	return toErase.size();
	BACKTRACE_END
}

unsigned LeastRecentlyUsed::eraseAllNonUsedHashes()
{
	BACKTRACE_BEGIN
	std::list< Hash > toErase;
	for ( auto i = _store.list(); not i.done(); i.next() ) {
		if ( _keptHashes.find( * i ) != _keptHashes.end() )
			continue;
		boost::filesystem::path filename( _store.filenameForExisting( * i ) );
		if ( boost::filesystem::last_write_time( filename ) > _startedAt )
			continue;
		toErase.emplace_back( * i );
	}
	for ( auto & hash: toErase )
		_store.erase( hash );
	return toErase.size();
	BACKTRACE_END
}

} // namespace ObjectStore
} // namespace Osmosis
