#ifndef __OSMOSIS_CHAIN_CHECK_OUT_H__
#define __OSMOSIS_CHAIN_CHECK_OUT_H__

namespace Osmosis {
namespace Chain
{

class CheckOut
{
public:
	CheckOut( CheckOut && other ) = default;

	CheckOut( std::vector< ObjectStoreInterface * > && objectStores, bool putIfMissing ) :
		_objectStores( objectStores ),
		_putIfMissing( putIfMissing ),
		_connections( objectStores.size() )
	{
#ifdef DEBUG
		for ( auto objectStore : _objectStores )
			ASSERT( objectStore != nullptr );
#endif // DEBUG
	}

	std::string getString( const Hash & hash )
	{
		for ( unsigned i = 0; i < _connections.size(); ++ i ) {
			if ( connection( i ).exists( hash ) ) {
				std::string content = connection( i ).getString( hash );
				if ( _putIfMissing ) {
					for ( int j = i - 1; j >= 0; -- j )
						connection( j ).putString( content, hash );
				}
				return std::move( content );
			}
		}
		THROW( Error, "The hash '" << hash << "' does not exist in any of the object stores" );
	}

	void getFile( const boost::filesystem::path & path, const Hash & hash )
	{
		for ( unsigned i = 0; i < _connections.size(); ++ i ) {
			if ( connection( i ).exists( hash ) ) {
				connection( i ).getFile( path, hash );
				if ( _putIfMissing ) {
					for ( int j = i - 1; j >= 0; -- j )
						connection( j ).putFile( path, hash );
				}
				return;
			}
		}
		THROW( Error, "The hash '" << hash << "' does not exist in any of the object stores" );
	}

	Hash getLabel( const std::string & label )
	{
		for ( unsigned i = 0; i < _connections.size(); ++ i ) {
			bool exists = connection( i ).listLabels( label ).size() > 0;
			if ( exists ) {
				Hash hash = connection( i ).getLabel( label );
				if ( _putIfMissing ) {
					std::string content = connection( i ).getString( hash );
					for ( int j = i - 1; j >= 0; -- j ) {
						connection( j ).putString( content, hash );
						connection( j ).setLabel( hash, label );
					}
				}
				return hash;
			}
		}
		THROW( Error, "The label '" << label << "' does not exist in any of the object stores" );
	}

private:
	const std::vector< ObjectStoreInterface * >                       _objectStores;
	const bool                                                        _putIfMissing;
	std::vector< std::unique_ptr< ObjectStoreConnectionInterface > >  _connections;

	ObjectStoreConnectionInterface & connection( unsigned objectStoreIndex )
	{
		ASSERT( objectStoreIndex < _connections.size() );
		if ( not _connections[ objectStoreIndex ] ) {
			ASSERT( objectStoreIndex < _objectStores.size() );
			ASSERT( _objectStores[ objectStoreIndex ] != nullptr );
			_connections[ objectStoreIndex ] = std::move( _objectStores[ objectStoreIndex ]->connect() );
		}
		return * _connections[ objectStoreIndex ];
	}

	CheckOut( const CheckOut & rhs ) = delete;
	CheckOut & operator= ( const CheckOut & rhs ) = delete;
};

} // namespace Chain
} // namespace Osmosis

#endif // __OSMOSIS_CHAIN_CHECK_OUT_H__
