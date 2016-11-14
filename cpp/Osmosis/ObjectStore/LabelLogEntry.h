#ifndef __OSMOSIS_OBJECT_STORE_LABEL_LOG_ENTRY_H__
#define __OSMOSIS_OBJECT_STORE_LABEL_LOG_ENTRY_H__

#include <boost/algorithm/string.hpp>
#include <Osmosis/Hash.h>
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

	LabelLogEntry( Operation operation, const std::string label, const Hash & hash );

	LabelLogEntry( const LabelLogEntry & other );

	LabelLogEntry( std::string line );

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

std::ostream & operator<<( std::ostream & os, const LabelLogEntry & entry );

} // namespace ObjectStore
} // namespace Osmosis

#endif // __OSMOSIS_OBJECT_STORE_LABEL_LOG_ENTRY_H__
