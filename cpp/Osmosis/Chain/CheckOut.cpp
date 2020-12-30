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
	_getCount( objectStores.size() )
{
	for ( auto & count : _getCount )
		count = 0;
#ifdef DEBUG
	for ( auto objectStore : _objectStores )
		ASSERT( objectStore != nullptr );
#endif // DEBUG
}

std::string CheckOut::getString( const Hash & hash )
{
	BACKTRACE_BEGIN
	for ( unsigned i = 0; i < _connections.size(); ++ i ) {
		if ( connection( i ).exists( hash ) ) {
			std::string content = connection( i ).getString( hash );
			_getCount[ i ] += 1;
			if ( _putIfMissing ) {
				for ( int j = i - 1; j >= 0; -- j )
					connection( j ).putString( content, hash );
			}
			return content;
		}
	}
	BACKTRACE_END_VERBOSE( "Hash " << hash );
	THROW( Error, "The hash '" << hash << "' does not exist in any of the object stores" );
}

void CheckOut::verify( const Hash & hash )
{
	BACKTRACE_BEGIN
	for ( unsigned i = 0; i < _connections.size(); ++ i )
		connection( i ).verify( hash );
	BACKTRACE_END
}

void CheckOut::getFile( const boost::filesystem::path & path, const Hash & hash )
{
	BACKTRACE_BEGIN
	for ( unsigned i = 0; i < _connections.size(); ++ i ) {
		if ( connection( i ).exists( hash ) ) {
			connection( i ).getFile( path, hash );
			_getCount[ i ] += 1;
			if ( _putIfMissing ) {
				for ( int j = i - 1; j >= 0; -- j )
					connection( j ).putFile( path, hash );
			}
			return;
		}
	}
	BACKTRACE_END_VERBOSE( "Path " << path << " Hash " << hash );
	THROW( Error, "The hash '" << hash << "' does not exist in any of the object stores" );
}

Hash CheckOut::getLabel( const std::string & label )
{
	BACKTRACE_BEGIN
	for ( unsigned i = 0; i < _connections.size(); ++ i ) {
		bool exists = connection( i ).listLabels( "^" + label + "$" ).size() > 0;
		if ( exists ) {
			try {
				Hash hash = getLabelFromConnection( i, label );
				return hash;
			} catch ( LabelFileIsCorrupted& ex ) {
				TRACE_WARNING( "Label file was corrupted in connection #" << i << "."
								" Trying next connection..." );
			}
		}
	}
	BACKTRACE_END_VERBOSE( "Label " << label );
	THROW( Error, "The label '" << label << "' does not exist in any of the object stores" );
}

const CheckOut::GetCountStats & CheckOut::getCount() const { return _getCount; }

Hash CheckOut::getLabelFromConnection( unsigned objectStoreIndex, const std::string &label )
{
	Hash hash = connection( objectStoreIndex ).getLabel( label );
	if ( _putIfMissing ) {
		std::string content = connection( objectStoreIndex ).getString( hash );
		for ( int j = objectStoreIndex - 1; j >= 0; -- j ) {
			if ( not connection( j ).exists( hash ) )
				connection( j ).putString( content, hash );
			connection( j ).setLabel( hash, label );
		}
	}
	if ( _touch ) {
		TRACE_INFO( "Touching chain for label: " << label );
		for ( unsigned j = objectStoreIndex + 1; j < _connections.size(); ++ j )
			try {
				connection( j ).getLabel( label );
			} CATCH_ALL_IGNORE( "While touching label on object store " << j << ", ignoring" );
	}
	return hash;
}
ObjectStoreConnectionInterface & CheckOut::connection( unsigned objectStoreIndex )
{
	ASSERT( objectStoreIndex < _connections.size() );
	if ( not _connections[ objectStoreIndex ] ) {
		ASSERT( objectStoreIndex < _objectStores.size() );
		ASSERT( _objectStores[ objectStoreIndex ] != nullptr );
		_connections[ objectStoreIndex ] = std::move( _objectStores[ objectStoreIndex ]->connect() );
	}
	return * _connections[ objectStoreIndex ];
}

} // namespace Chain
} // namespace Osmosis
