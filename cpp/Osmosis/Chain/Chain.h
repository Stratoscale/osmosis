#ifndef __OSMOSIS_CHAIN_CHAIN_H__
#define __OSMOSIS_CHAIN_CHAIN_H__

#include "Osmosis/Chain/Factory.h"
#include "Osmosis/Chain/CheckOut.h"

namespace Osmosis {
namespace Chain
{

class Chain
{
public:
	Chain( const std::string & objectStoresArgument, bool putIfMissing, bool chainTouch ):
		_putIfMissing( putIfMissing ),
		_chainTouch( chainTouch )
	{
		std::vector< std::string > locations;
		boost::split( locations, objectStoresArgument, boost::is_any_of( "+" ) );
		for ( auto & location : locations )
			_objectStores.push_back( std::move( factory( location ) ) );
	}

	unsigned count() const
	{
		return _objectStores.size();
	}

	ObjectStoreInterface & single()
	{
		ASSERT( _objectStores.size() >= 1 );
		return * _objectStores[ 0 ];
	}

	CheckOut checkOut()
	{
		std::vector< ObjectStoreInterface * > objectStores;
		for ( auto & objectStore : _objectStores )
			objectStores.push_back( objectStore.get() );
		return std::move( CheckOut( std::move( objectStores ), _putIfMissing, _chainTouch ) );
	}

private:
	const bool                                              _putIfMissing;
	const bool                                              _chainTouch;
	std::vector< std::unique_ptr< ObjectStoreInterface > >  _objectStores;

	Chain( const Chain & rhs ) = delete;
	Chain & operator= ( const Chain & rhs ) = delete;
};

} // namespace Chain
} // namespace Osmosis

#endif // __OSMOSIS_CHAIN_CHAIN_H__
