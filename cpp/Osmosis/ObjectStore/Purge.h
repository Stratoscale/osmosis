#ifndef __OSMOSIS_OBJECT_STORE_PURGE_H__
#define __OSMOSIS_OBJECT_STORE_PURGE_H__

#include <unordered_set>
#include "Osmosis/DirList.h"

namespace Osmosis {
namespace ObjectStore
{

class Purge
{
public:
	Purge( Store & store, Labels & labels ):
		_store( store ),
		_labels( labels )
	{}

	void purge()
	{
		BACKTRACE_BEGIN
		startWithAllObjects();
		size_t before = _staleHashes.size();
		TRACE_INFO( "Found " << before << " objects" );
		takeOutAllLabels();
		size_t after = _staleHashes.size();
		TRACE_INFO( "Purge found " << after << " objects to purge (" <<
				( before - after ) << " remain)" );
		for ( auto & hash : _staleHashes )
			boost::filesystem::remove( _store.filenameForExisting( hash ) );
		BACKTRACE_END
	}

private:
	Store &                     _store;
	Labels &                    _labels;
	std::unordered_set< Hash >  _staleHashes;

	void startWithAllObjects()
	{
		BACKTRACE_BEGIN
		for ( auto i = _store.list(); not i.done(); i.next() )
			_staleHashes.emplace( * i );
		BACKTRACE_END
	}

	void takeOutAllLabels()
	{
		BACKTRACE_BEGIN
		for ( auto & i : listLabels() ) {
			Container< Hash > hash;
			try {
				hash.emplace( _labels.readLabelNoLog( i ) );
			} catch ( Error & error ) {
				TRACE_WARNING( "While purging, found a bad label '" << i << "', erasing (" << error.what() << ")" );
				_labels.erase( i );
				continue;
			}
			_staleHashes.erase( * hash );

			std::ifstream dirListFile( _store.filenameForExisting( * hash ).string() );
			takeOutDirListFile( dirListFile );
		}
		BACKTRACE_END
	}

	void takeOutDirListFile( std::ifstream & dirListFile )
	{
		BACKTRACE_BEGIN
		std::string line;
		while ( std::getline( dirListFile, line ) ) {
			Container< Hash > hash;
			DirListEntry::parseOnlyHashFromLine( line, hash );
			if ( hash.constructed() )
				_staleHashes.erase( * hash );
		}
		BACKTRACE_END
	}

	std::list< std::string > listLabels()
	{
		BACKTRACE_BEGIN
		std::list< std::string > labels;
		for ( auto i = _labels.list( "" ); not i.done(); i.next() )
			labels.push_back( * i );
		return std::move( labels );
		BACKTRACE_END
	}

	Purge( const Purge & rhs ) = delete;
	Purge & operator= ( const Purge & rhs ) = delete;
};

} // namespace ObjectStore
} // namespace Osmosis

#endif // __OSMOSIS_OBJECT_STORE_PURGE_H__
