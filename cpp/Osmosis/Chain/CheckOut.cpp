#include <boost/filesystem.hpp>
#include "Osmosis/Chain/CheckOut.h"
#include "Osmosis/Debug.h"

namespace Osmosis {
namespace Chain
{

CheckOut::CheckOut( std::vector< ObjectStoreInterface * > && objectStores, bool putIfMissing, bool touch ) :
	_objectStores( objectStores ),
	_putIfMissing( putIfMissing ),
	_touch( touch ),
	_connections( objectStores.size() ),
	_getCount( objectStores.size() ),
	_idxOfLastUsedConnection( -1 )
{
	for ( auto & count : _getCount )
		count = 0;
#ifdef DEBUG
	for ( auto objectStore : _objectStores )
		ASSERT( objectStore != nullptr );
#endif // DEBUG
	for ( uint connIdx = 0; connIdx < _connections.size(); ++connIdx )
		_connectionIdxToGetCountIdx.push_back( connIdx );
}

std::string CheckOut::getString( const Hash & hash )
{
	BACKTRACE_BEGIN
	std::string content;
	const bool doesFileExist = tryForEachConnectionUntilSuccess(
			[ &hash, &content ] ( ObjectStoreConnectionInterface & connection ) {
		if ( connection.exists( hash ) ) {
			content = std::move( connection.getString( hash ) );
			return true;
		}
		return false;
	} );
	if ( not doesFileExist )
		THROW( Error, "The hash '" << hash << "' does not exist in any of the object stores" );
	_getCount[ _connectionIdxToGetCountIdx[ _idxOfLastUsedConnection ] ] += 1;
	if ( _putIfMissing ) {
		const int nrConnectionsToPutIn = _idxOfLastUsedConnection;
		tryForEachConnection( [ &hash, &content ] ( ObjectStoreConnectionInterface & connection ) {
			connection.putString( content, hash );
			return true;
		},
		0,
		nrConnectionsToPutIn );
	}
	return content;
	BACKTRACE_END_VERBOSE( "Hash " << hash );
}

bool CheckOut::tryForEachConnection(
	std::function< bool( ObjectStoreConnectionInterface & connection ) > func,
	int idxOfConnectionToStartFrom,
	short nrConnectionsToTry,
	bool removeConnectionOnError,
	bool stopOnFirstSuccess )
{
	BACKTRACE_BEGIN
	if ( nrConnectionsToTry == TRY_ALL_CONNECTIONS )
		nrConnectionsToTry = _connections.size();
	unsigned short connIdx = idxOfConnectionToStartFrom;
	unsigned short trialCounter = 0;
	bool isSuccessful = false;
	while( connIdx < _connections.size() and trialCounter++ < nrConnectionsToTry ) {
		bool conserveIdxInNextIteration = false;
		bool hasAnErrorOccurred = false;
		std::string errorMessage;
		try {
			isSuccessful = func( connection( connIdx ) );
		} catch ( boost::system::system_error & ex ) {
			errorMessage = ex.what();
			hasAnErrorOccurred = true;
		} catch ( LabelFileIsCorrupted & ex ) {
			errorMessage = ex.what();
			hasAnErrorOccurred = true;
		} catch ( Error & ex ) {
			errorMessage = ex.backtrace() + ex.what();
			hasAnErrorOccurred = true;
		}
		if ( isSuccessful and stopOnFirstSuccess ) {
			break;
		} else if ( hasAnErrorOccurred ) {
			TRACE_WARNING( "Error when using object store " << connIdx << ": " << errorMessage );
			if ( removeConnectionOnError ) {
				TRACE_WARNING( "Discarding errornous connection #" << connIdx );
				removeConnection( connIdx );
				conserveIdxInNextIteration = true;
			} else
				TRACE_WARNING( "Ignoring." );
		}
		if ( not conserveIdxInNextIteration ) {
			++connIdx;
		}
	}
	return isSuccessful;
	BACKTRACE_END
}

bool CheckOut::tryForEachConnectionUntilSuccess(
	std::function< bool( ObjectStoreConnectionInterface & connection ) > func,
		int idxOfConnectionToStartFrom )
{
	BACKTRACE_BEGIN
	return tryForEachConnection( func, idxOfConnectionToStartFrom, TRY_ALL_CONNECTIONS, true, true );
	BACKTRACE_END
}

void CheckOut::verify( const Hash & hash )
{
	BACKTRACE_BEGIN
	tryForEachConnection( [ &hash ] ( ObjectStoreConnectionInterface & connection ) {
		connection.verify( hash );
		return true;
	} );
	BACKTRACE_END
}

void CheckOut::getFile( const boost::filesystem::path & path, const Hash & hash )
{
	BACKTRACE_BEGIN
	const bool doesFileExist = tryForEachConnectionUntilSuccess(
			[ &hash, &path ] ( ObjectStoreConnectionInterface & connection ) {
		if ( connection.exists( hash ) ) {
			boost::filesystem::remove( path );
			connection.getFile( path, hash );
			return true;
		}
		return false;
	} );
	if ( not doesFileExist )
		THROW( Error, "The hash '" << hash << "' does not exist in any of the object stores" );
	_getCount[ _connectionIdxToGetCountIdx[ _idxOfLastUsedConnection ] ] += 1;
	if ( _putIfMissing ) {
		const int nrConnectionsToPutIn = _idxOfLastUsedConnection;
		tryForEachConnection( [ &hash, &path ] ( ObjectStoreConnectionInterface & connection ) {
			connection.putFile( path, hash );
			return true;
		},
		0,
		nrConnectionsToPutIn );
	}
	BACKTRACE_END_VERBOSE( "Path " << path << " Hash " << hash );
}

Hash CheckOut::getLabel( const std::string & label )
{
	BACKTRACE_BEGIN
	Hash hash;
	const bool doesLabelExist = tryForEachConnectionUntilSuccess(
			[ &label, &hash ] ( ObjectStoreConnectionInterface & connection ) {
		bool exists = connection.listLabels( "^" + label + "$" ).size() > 0;
		if ( exists ) {
			hash = connection.getLabel( label );
			return true;
		}
		return false;
	} );
	if ( not doesLabelExist )
		THROW( Error, "The label '" << label << "' does not exist in any of the object stores" );
	int objectStoreIndex = _idxOfLastUsedConnection;
	if ( _putIfMissing ) {
		std::string content;
		const bool doesStringExist = tryForEachConnectionUntilSuccess(
				[ &content, &hash ] ( ObjectStoreConnectionInterface & connection ) {
			content = connection.getString( hash );
			return true;
		},
		objectStoreIndex );
		if ( not doesStringExist )
			THROW( Error, "The label dirlist hash '" << hash << "' does not exist in any of the object "
			              " stores" );
		const int nrConnectionsToPutIn = objectStoreIndex;
		tryForEachConnection( [ &hash, &content, &label ] ( ObjectStoreConnectionInterface & connection ) {
			if ( not connection.exists( hash ) ) {
				connection.putString( content, hash );
			}
			connection.setLabel( hash, label );
			return true;
		},
		0,
		nrConnectionsToPutIn );
	}
	if ( _touch ) {
		TRACE_INFO( "Touching chain for label: " << label );
		const short *connIdxPtr = & _idxOfLastUsedConnection;
		tryForEachConnection( [ & label, connIdxPtr ]
		                      ( ObjectStoreConnectionInterface & connection ) {
			try {
				connection.getLabel( label );
			} catch( ... ) {
			}
			return true;
		},
		objectStoreIndex + 1,
		TRY_ALL_CONNECTIONS,
		false,
		false );
	}
	return hash;
	BACKTRACE_END_VERBOSE( "Label " << label );
}

const CheckOut::GetCountStats & CheckOut::getCount() const { return _getCount; }

ObjectStoreConnectionInterface & CheckOut::connection( unsigned objectStoreIndex )
{
	_idxOfLastUsedConnection = static_cast< short > ( objectStoreIndex );
	ASSERT( objectStoreIndex < _connections.size() );
	if ( not _connections[ objectStoreIndex ] ) {
		ASSERT( objectStoreIndex < _objectStores.size() );
		ASSERT( _objectStores[ objectStoreIndex ] != nullptr );
		_connections[ objectStoreIndex ] = std::move( _objectStores[ objectStoreIndex ]->connect() );
	}
	return * _connections[ objectStoreIndex ];
}

void CheckOut::removeConnection( int connIdx )
{
	ASSERT( connIdx >= 0 );
	TRACE_WARNING( "Discarding connection #" << connIdx );
	_connections.erase( _connections.begin() + connIdx );
	_objectStores.erase( _objectStores.begin() + connIdx );
	_connectionIdxToGetCountIdx.erase( _connectionIdxToGetCountIdx.begin() + connIdx );
}

} // namespace Chain
} // namespace Osmosis
