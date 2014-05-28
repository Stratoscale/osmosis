#ifndef __OSMOSIS_OBJECT_STORE_LABELS_ITERATOR_H__
#define __OSMOSIS_OBJECT_STORE_LABELS_ITERATOR_H__

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>

namespace Osmosis {
namespace ObjectStore
{

class LabelsIterator
{
public:
	LabelsIterator( boost::filesystem::path directory, const std::string & expression ):
		_iterator( directory ),
		_regex( expression )
	{
		advanceTillMatchesRegex();
	}

	std::string operator * () const
	{
		ASSERT( not done() );
		return std::move( _iterator->path().filename().string() );
	}

	void next()
	{
		ASSERT( not done() );
		++ _iterator;
		advanceTillMatchesRegex();
	}

	bool done() const
	{
		return _iterator == boost::filesystem::directory_iterator();
	}

private:
	boost::filesystem::directory_iterator  _iterator;
	boost::regex                           _regex;

	void advanceTillMatchesRegex()
	{
		while ( not done() and not matchesRegex() )
			++ _iterator;
	}

	bool matchesRegex() const
	{
		std::string label = * * this;
		boost::sregex_iterator matched( label.begin(), label.end(), _regex );
		return matched != boost::sregex_iterator();
	}
};

} // namespace ObjectStore
} // namespace Osmosis

#endif // __OSMOSIS_OBJECT_STORE_LABELS_ITERATOR_H__
