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
		startWithAllObjects();
		size_t before = _staleHashes.size();
		TRACE_INFO( "Found " << before << " objects" );
		takeOutAllLabels();
		size_t after = _staleHashes.size();
		TRACE_INFO( "Purge found " << after << " objects to purge (" <<
				( before - after ) << " remain)" );
		for ( auto & hash : _staleHashes )
			boost::filesystem::remove( _store.filenameForExisting( hash ) );
	}

private:
	Store &                     _store;
	Labels &                    _labels;
	std::unordered_set< Hash >  _staleHashes;

	void startWithAllObjects()
	{
		for ( auto i = _store.list(); not i.done(); i.next() )
			_staleHashes.emplace( * i );
	}

	void takeOutAllLabels()
	{
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
	}

	void takeOutDirListFile( std::ifstream & dirListFile )
	{
		std::string line;
		while ( std::getline( dirListFile, line ) ) {
			Container< Hash > hash;
			DirListEntry::parseOnlyHashFromLine( line, hash );
			if ( hash.constructed() )
				_staleHashes.erase( * hash );
		}
	}

    std::list< std::string > listLabels()
    {
        std::list< std::string > labels;
		for ( auto i = _labels.list( "" ); not i.done(); i.next() )
            labels.push_back( * i );
        return std::move( labels );
    }

	Purge( const Purge & rhs ) = delete;
	Purge & operator= ( const Purge & rhs ) = delete;
};

} // namespace ObjectStore
} // namespace Osmosis

#endif // __OSMOSIS_OBJECT_STORE_PURGE_H__
