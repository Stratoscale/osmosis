#ifndef __OSMOSIS_OBJECT_STORE_PURGE_H__
#define __OSMOSIS_OBJECT_STORE_PURGE_H__

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
		takeOutAllLabels();
		size_t after = _staleHashes.size();
		TRACE_INFO( "Purge found " << after << " objects to purge (" <<
				( before - after ) << " remain)" );
		for ( auto & hash : _staleHashes )
			boost::filesystem::remove( _store.filenameForExisting( hash ) );
	}

private:
	Store &           _store;
	Labels &          _labels;
	std::set< Hash >  _staleHashes;

	void startWithAllObjects()
	{
		for ( auto i = _store.list(); not i.done(); i.next() )
			_staleHashes.emplace( * i );
	}

	void takeOutAllLabels()
	{
		for ( auto i = _labels.list( "" ); not i.done(); i.next() ) {
			Hash hash = _labels.readLabel( * i );
			_staleHashes.erase( hash );
			DirList dirList;
			std::ifstream dirListFile( _store.filenameForExisting( hash ).string() );
			dirListFile >> dirList;
			takeOutAllHashesFromDirList( dirList );
		}
	}

	void takeOutAllHashesFromDirList( const DirList & dirList )
	{
		for ( auto & entry : dirList.entries() )
			if ( entry.hash )
				_staleHashes.erase( * entry.hash );
	}

	Purge( const Purge & rhs ) = delete;
	Purge & operator= ( const Purge & rhs ) = delete;
};

} // namespace ObjectStore
} // namespace Osmosis

#endif // __OSMOSIS_OBJECT_STORE_PURGE_H__
