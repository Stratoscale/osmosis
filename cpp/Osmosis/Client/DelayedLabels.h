#ifndef __OSMOSIS_CLIENT_DELAYED_LABELS_H__
#define __OSMOSIS_CLIENT_DELAYED_LABELS_H__

namespace Osmosis {
namespace Client
{

class DelayedLabels
{
public:
	DelayedLabels( const std::string & label )
	{
		if ( label == "+" )
			return;
		fromLabel( label );
	}

	const std::vector< std::string > & labels()
	{
		ASSERT( _labels.size() > 0 );
		return _labels;
	}

	void fetch()
	{
		if ( _labels.size() > 0 )
			return;
		TRACE_INFO( "Enter label name on next line:" );
		std::string line;
		std::getline( std::cin, line );
		if ( line.size() <= 0 )
			THROW( Error, "Invalid label from stdin: '" << line << "'" );
		TRACE_INFO( "Got label name: '" << line << "'" );
		fromLabel( line );
	}

private:
	std::vector< std::string > _labels;

	void fromLabel( const std::string & label )
	{
		boost::split( _labels, label, boost::is_any_of( "+" ) );
	}

	DelayedLabels( const DelayedLabels & rhs ) = delete;
	DelayedLabels & operator= ( const DelayedLabels & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_DELAYED_LABELS_H__
