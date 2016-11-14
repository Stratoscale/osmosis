#ifndef __OSMOSIS_CHAIN_CHECK_OUT_H__
#define __OSMOSIS_CHAIN_CHECK_OUT_H__

#include "Osmosis/Chain/ObjectStoreInterface.h"

namespace Osmosis {
namespace Chain
{

class CheckOut
{
public:
	CheckOut( CheckOut && other ) = default;

	CheckOut( std::vector< ObjectStoreInterface * > && objectStores, bool putIfMissing, bool touch );

	std::string getString( const Hash & hash );

	void verify( const Hash & hash );

	void getFile( const boost::filesystem::path & path, const Hash & hash );

	Hash getLabel( const std::string & label );

	typedef std::vector< unsigned > GetCountStats;

	const GetCountStats & getCount() const;

private:
	const std::vector< ObjectStoreInterface * >                       _objectStores;
	const bool                                                        _putIfMissing;
	const bool                                                        _touch;
	std::vector< std::unique_ptr< ObjectStoreConnectionInterface > >  _connections;
	GetCountStats                                                     _getCount;

	Hash getLabelFromConnection( unsigned objectStoreIndex, const std::string &label );

	ObjectStoreConnectionInterface & connection( unsigned objectStoreIndex );

	CheckOut( const CheckOut & rhs ) = delete;
	CheckOut & operator= ( const CheckOut & rhs ) = delete;
};

} // namespace Chain
} // namespace Osmosis

#endif // __OSMOSIS_CHAIN_CHECK_OUT_H__
