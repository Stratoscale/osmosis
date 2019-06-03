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
	std::vector< ObjectStoreInterface * >                             _objectStores;
	const bool                                                        _putIfMissing;
	const bool                                                        _touch;
	std::vector< std::unique_ptr< ObjectStoreConnectionInterface > >  _connections;
	GetCountStats                                                     _getCount;
	short                                                             _idxOfLastUsedConnection;
	std::vector< int >                                                _connectionIdxToGetCountIdx;
	enum {
		TRY_ALL_CONNECTIONS = -1
	};

	Hash getLabelFromConnection( unsigned objectStoreIndex, const std::string &label );

	Hash tryGetLabel( const std::string & label );

	void removeConnection( int connIdx );

	bool tryForEachConnection(
		std::function< bool( ObjectStoreConnectionInterface & connection ) > func,
		int idxOfConnectionToStartFrom=0,
		short nrConnectionsToTry=TRY_ALL_CONNECTIONS,
		bool removeConnectionOnError=true,
		bool stopOnFirstSuccess=false );

	bool tryForEachConnectionUntilSuccess(
		std::function< bool( ObjectStoreConnectionInterface & connection ) > func,
		int idxOfConnectionToStartFrom=0 );

	ObjectStoreConnectionInterface & connection( unsigned objectStoreIndex );

	CheckOut( const CheckOut & rhs ) = delete;
	CheckOut & operator= ( const CheckOut & rhs ) = delete;
};

} // namespace Chain
} // namespace Osmosis

#endif // __OSMOSIS_CHAIN_CHECK_OUT_H__
