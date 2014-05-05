#ifndef __OSMOSIS_CLIENT_DIR_LIST_ENTRY_H__
#define __OSMOSIS_CLIENT_DIR_LIST_ENTRY_H__

namespace Osmosis {
namespace Client
{

struct DirListEntry
{
	boost::filesystem::path path;
	std::unique_ptr< Hash > hash;

	DirListEntry( const boost::filesystem::path path ) :
		path( path )
	{}

	DirListEntry( std::string line )
	{
		boost::trim( line );
		std::vector< std::string > split;
		boost::split( split, line, boost::is_any_of( "\t" ) );
		if ( split.size() != 2 )
			THROW( Error, "Line '" << line << "' must contain exactly 2 tab-sparated fields" );
		path = boost::trim_copy_if( split[ 0 ], boost::is_any_of( "\"" ) );
		if ( split[ 1 ] != "nohash" )
			hash.reset( new Hash( Hash::fromHex( split[ 1 ] ) ) );
	}

	friend std::ostream & operator<<( std::ostream & os, const DirListEntry & entry )
	{
		os << entry.path << '\t';
		if ( entry.hash )
			os << * entry.hash;
		else
			os << "nohash";
		os << '\n';
		return os;
	}

private:
	DirListEntry( const DirListEntry & rhs ) = delete;
	DirListEntry & operator= ( const DirListEntry & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_DIR_LIST_ENTRY_H__
