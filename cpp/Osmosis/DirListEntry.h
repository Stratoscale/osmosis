#ifndef __OSMOSIS_DIR_LIST_ENTRY_H__
#define __OSMOSIS_DIR_LIST_ENTRY_H__

#include "Osmosis/Hash.h"
#include "Osmosis/FileStatus.h"
#include "Common/Container.h"

namespace Osmosis
{

struct DirListEntry
{
	boost::filesystem::path  path;
	FileStatus               status;
	std::unique_ptr< Hash >  hash;

	DirListEntry( const boost::filesystem::path & path, const FileStatus & status ) :
		path( path ),
		status( status )
	{}

	DirListEntry( std::string line )
	{
		BACKTRACE_BEGIN
		boost::trim( line );
		SplitString split( line, '\t' );
		path = boost::trim_copy_if( split.asString(), boost::is_any_of( "\"" ) );
		split.next();
		if ( split.done() )
			THROW( Error, "'" << line << "' is in an invalid format for a dir list entry" );
		FileStatus unserializedStatus( split.asString() );
		status = unserializedStatus;
		split.next();
		if ( split.done() )
			THROW( Error, "'" << line << "' is in an invalid format for a dir list entry" );
		std::string hashString = split.asString();
		if ( unserializedStatus.isRegular() ) {
			if ( hashString == "nohash" )
				THROW( Error, "'" << line << "' is in an invalid format for a dir list entry: " <<
						"regular files must have a hash" );
			hash.reset( new Hash( hashString ) );
		} else {
			if ( hashString != "nohash" )
				THROW( Error, "'" << line << "' is in an invalid format for a dir list entry: " <<
						"non regular files must not have a hash" );
		}
		split.next();
		if ( not split.done() )
			THROW( Error, "'" << line << "' is in an invalid format for a dir list entry" );
		BACKTRACE_END_VERBOSE( "Line '" << line << '\'' );
	}

	friend std::ostream & operator<<( std::ostream & os, const DirListEntry & entry )
	{
		os << entry.path << '\t' << entry.status << '\t';
		if ( entry.hash )
			os << * entry.hash;
		else
			os << "nohash";
		os << '\n';
		return os;
	}

	static void parseOnlyHashFromLine( const std::string & line, Container< Hash > & result )
	{
		BACKTRACE_BEGIN
		SplitString split( line, '\t' );
		if ( split.done() )
			THROW( Error, "'" << line << "' is in an invalid format for a dir list entry" );
		split.next();
		if ( split.done() )
			THROW( Error, "'" << line << "' is in an invalid format for a dir list entry" );
		split.next();
		if ( split.done() )
			THROW( Error, "'" << line << "' is in an invalid format for a dir list entry" );
		ASSERT( split.charCount() > 0 );
		if ( split.asCharPtr()[0] == 'n' ) //nohash
			return;
		result.emplace( split.asCharPtr(), split.charCount() );
		BACKTRACE_END_VERBOSE( "Line '" << line << '\'' );
	}

private:
	DirListEntry( const DirListEntry & rhs ) = delete;
	DirListEntry & operator= ( const DirListEntry & rhs ) = delete;
};

} // namespace Osmosis

#endif // __OSMOSIS_DIR_LIST_ENTRY_H__
