#include <boost/algorithm/string.hpp>
#include "Osmosis/Client/DelayedLabels.h"
#include "Common/Debug.h"
#include "Common/Error.h"

namespace Osmosis {
namespace Client
{

DelayedLabels::DelayedLabels( const std::string & label )
{
	if ( label == "+" )
		return;
	fromLabel( label );
}

const std::vector< std::string > & DelayedLabels::labels()
{
	ASSERT( _labels.size() > 0 );
	return _labels;
}

void DelayedLabels::fetch()
{
	BACKTRACE_BEGIN
	if ( _labels.size() > 0 )
		return;
	TRACE_INFO( "Enter label name on next line:" );
	std::string line;
	std::getline( std::cin, line );
	if ( line.size() <= 0 )
		THROW( Error, "Invalid label from stdin: '" << line << "'" );
	TRACE_INFO( "Got label name: '" << line << "'" );
	fromLabel( line );
	BACKTRACE_END
}

void DelayedLabels::fromLabel( const std::string & label )
{
	boost::split( _labels, label, boost::is_any_of( "+" ) );
}

} // namespace Client
} // namespace Osmosis
