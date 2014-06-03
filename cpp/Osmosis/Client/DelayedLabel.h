#ifndef __OSMOSIS_CLIENT_DELAYED_LABEL_H__
#define __OSMOSIS_CLIENT_DELAYED_LABEL_H__

namespace Osmosis {
namespace Client
{

class DelayedLabel
{
public:
	DelayedLabel( const std::string & label ):
		_label( label )
	{}

	const std::string & label()
	{
		ASSERT( _label != "+" );
		return _label;
	}

	void fetch()
	{
		if ( _label != "+" )
			return;
		TRACE_INFO( "Enter label name on next line:" );
		std::getline( std::cin, _label );
		if ( _label.size() <= 0 )
			THROW( Error, "Invalid label from stdin: '" << _label << "'" );
		TRACE_INFO( "Got label name: '" << _label << "'" );
	}

private:
	std::string _label;

	DelayedLabel( const DelayedLabel & rhs ) = delete;
	DelayedLabel & operator= ( const DelayedLabel & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_DELAYED_LABEL_H__
