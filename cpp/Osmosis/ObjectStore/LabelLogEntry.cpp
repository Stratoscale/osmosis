#include <ostream>
#include <boost/algorithm/string.hpp>
#include <Osmosis/ObjectStore/LabelLogEntry.h>
#include <Osmosis/Hash.h>
#include "Common/SplitString.h"

namespace Osmosis {
namespace ObjectStore
{

LabelLogEntry::LabelLogEntry( Operation operation, const std::string label, const Hash & hash ) :
	time( std::time( nullptr ) ),
	operation( operation ),
	label( label ),
	hash( new Hash( hash ) )
{}

LabelLogEntry::LabelLogEntry( const LabelLogEntry & other ) :
	time( other.time ),
	operation( other.operation ),
	label( other.label ),
	hash( new Hash( * other.hash ) )
{}

LabelLogEntry::LabelLogEntry( std::string line )
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
	hash.reset( new Hash( split.asString() ) );
	split.next();
	if ( not split.done() )
		THROW( Error, "'" << line << "' is in an invalid format for a label log entry" );
}

std::ostream & operator<<( std::ostream & os, const LabelLogEntry & entry )
{
	os << entry.time << '\t' << static_cast< char >( entry.operation ) << '\t' << entry.label;
	if ( entry.hash )
		os << '\t' << * entry.hash;
	os << '\n';
	return os;
}


} // namespace ObjectStore
} // namespace Osmosis
