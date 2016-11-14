#ifndef __OSMOSIS_DIR_LIST_H__
#define __OSMOSIS_DIR_LIST_H__

#include <unordered_map>
#include "Osmosis/DirListEntry.h"

namespace std
{

template<> struct hash< boost::filesystem::path >
{
	std::size_t operator()( const boost::filesystem::path & path ) const
	{
		return hash< std::string >()( path.native() );
	}
};

} // namespace std

namespace Osmosis
{

class DirList
{
public:
	DirList();

	DirList( DirList && rhs );

	void add( const boost::filesystem::path & path, const FileStatus & status );

	void setHash( boost::filesystem::path path, const Hash & hash );

	const DirListEntry * find( boost::filesystem::path path ) const;

	friend std::ostream & operator<<( std::ostream & os, const DirList & dirList )
	{
		for ( auto & entry : dirList._entries )
			os << entry;
		return os;
	}

	friend std::istream & operator>>( std::istream & is, DirList & dirList )
	{
		std::string line;
		while ( std::getline( is, line ) ) {
			dirList._entries.emplace_back( line );
			dirList._index[ dirList._entries.back().path ] = & dirList._entries.back();
		}
		return is;
	}

	inline size_t size() const { return _index.size(); }

	typedef std::list< DirListEntry > List;
	inline const List & entries() const { return _entries; }

private:
	typedef std::unordered_map< boost::filesystem::path, struct DirListEntry * > Index;

	List _entries;
	Index _index;

	DirList( const DirList & rhs ) = delete;
	DirList & operator= ( const DirList & rhs ) = delete;
};

} // namespace Osmosis

#endif // __OSMOSIS_DIR_LIST_H__
