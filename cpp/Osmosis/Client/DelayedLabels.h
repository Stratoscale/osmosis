#ifndef __OSMOSIS_CLIENT_DELAYED_LABELS_H__
#define __OSMOSIS_CLIENT_DELAYED_LABELS_H__

#include <string>
#include <vector>

namespace Osmosis {
namespace Client
{

class DelayedLabels
{
public:
	DelayedLabels( const std::string & label );

	const std::vector< std::string > & labels();

	void fetch();

private:
	std::vector< std::string > _labels;

	void fromLabel( const std::string & label );

	DelayedLabels( const DelayedLabels & rhs ) = delete;
	DelayedLabels & operator= ( const DelayedLabels & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_DELAYED_LABELS_H__
