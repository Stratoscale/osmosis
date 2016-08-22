#include "Osmosis/DirList.h"
#include "Osmosis/DirListEntry.h"

namespace Osmosis
{

DirList::DirList() {}

DirList::DirList( DirList && rhs ) :
	_entries( std::move( rhs._entries ) ),
	_index( std::move( rhs._index ) )
{}

void DirList::add( const boost::filesystem::path & path, const FileStatus & status )
{
	ASSERT( _index.find( path ) == _index.end() );
	_entries.emplace_back( path, status );
	_index[ path ] = & _entries.back();
}

void DirList::setHash( boost::filesystem::path path, const Hash & hash )
{
	ASSERT( _index.find( path ) != _index.end() );
	ASSERT( _index.at( path ) != nullptr );
	ASSERT( not _index.at( path )->hash );
	_index.at( path )->hash.reset( new Hash( hash ) );
}

const DirListEntry * DirList::find( boost::filesystem::path path ) const
{
	auto found = _index.find( path );
	if ( found == _index.end() )
		return nullptr;
	return found->second;
}

} // namespace Osmosis
