#ifndef __OSMOSIS_OBJECT_STORE_LABEL_LOG_ENTRY_H__
#define __OSMOSIS_OBJECT_STORE_LABEL_LOG_ENTRY_H__

#include <boost/algorithm/string.hpp>
#include "Common/SplitString.h"

namespace Osmosis {
namespace ObjectStore
{

struct LabelLogEntry
{
	enum Operation { GET = 'G', SET = 'S', REMOVE = 'R' };

	std::time_t              time;
	Operation                operation;
	std::string              label;
	std::unique_ptr< Hash >  hash;

	LabelLogEntry( Operation operation, const std::string label, const Hash & hash ) :
		time( std::time( nullptr ) ),
		operation( operation ),
		label( label ),
		hash( new Hash( hash ) )
	{}

	LabelLogEntry( const LabelLogEntry & other ) :
		time( other.time ),
		operation( other.operation ),
		label( other.label ),
		hash( new Hash( * other.hash ) )
	{}

	LabelLogEntry( std::string line )
	{
		boost::trim( line );
		SplitString split( line, '\t' );
		time = std::stol( split.asString() );
		split.next();
		if ( split.done() )
			THROW( Error, "'" << line << "' is in an invalid format for a label log entry" );
		operation = parseOperation( split.asString() );
		split.next();
		if ( split.done() )
			THROW( Error, "'" << line << "' is in an invalid format for a label log entry" );
		label = std::move( split.asString() );
		split.next();
		if ( split.done() )
			return;
		hash.reset( new Hash( Hash::fromHex( split.asString() ) ) );
		split.next();
		if ( not split.done() )
			THROW( Error, "'" << line << "' is in an invalid format for a label log entry" );
	}

	friend std::ostream & operator<<( std::ostream & os, const LabelLogEntry & entry )
	{
		os << entry.time << '\t' << static_cast< char >( entry.operation ) << '\t' << entry.label;
		if ( entry.hash )
			os << '\t' << * entry.hash;
		os << '\n';
		return os;
	}

private:
	static Operation parseOperation( const std::string & input )
	{
		if ( input.size() != 1 )
			THROW( Error, "'" << input << "' is not a valid operation" );
		if ( input[ 0 ] != static_cast< char >( GET ) and
			input[ 0 ] != static_cast< char >( SET ) and
			input[ 0 ] != static_cast< char >( REMOVE ) )
			THROW( Error, "'" << input << "' is not a valid operation" );
		return static_cast< Operation >( input[ 0 ] );
	}

	LabelLogEntry & operator= ( const LabelLogEntry & rhs ) = delete;
};

} // namespace ObjectStore
} // namespace Osmosis

#endif // __OSMOSIS_OBJECT_STORE_LABEL_LOG_ENTRY_H__
