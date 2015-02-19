#ifndef __OSMOSIS_OBJECT_STORE_OBJECTS_ITERATOR_H__
#define __OSMOSIS_OBJECT_STORE_OBJECTS_ITERATOR_H__

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include "Osmosis/ObjectStore/DirectoryNames.h"

namespace Osmosis {
namespace ObjectStore
{

class ObjectsIterator
{
public:
	ObjectsIterator( boost::filesystem::path rootPath ):
		_iterator( rootPath ),
		_labelsPath( rootPath / DirectoryNames::LABELS ),
		_draftsPath( rootPath / DirectoryNames::DRAFTS ),
		_labelLogPath( rootPath / DirectoryNames::LABEL_LOG )
	{
		skipNonObjects();
	}

	Hash operator * () const
	{
		ASSERT( not done() );
		boost::filesystem::path path = _iterator->path();
		std::string last = path.filename().string();
		path.remove_filename();
		std::string middle = path.filename().string();
		path.remove_filename();
		std::string first = path.filename().string();
		try {
			return Hash( first + middle + last );
		} catch ( ... ) {
			TRACE_ERROR( "While converting '" << first << "' '" << middle << "' '" << last << "' to hex" );
			throw;
		}
	}

	void next()
	{
		ASSERT( not done() );
		++ _iterator;
		skipNonObjects();
	}

	bool done() const
	{
		return _iterator == boost::filesystem::recursive_directory_iterator();
	}

private:
	boost::filesystem::recursive_directory_iterator  _iterator;
	boost::filesystem::path                          _labelsPath;
	boost::filesystem::path                          _draftsPath;
	boost::filesystem::path                          _labelLogPath;

	void skipNonObjects()
	{
		while ( not done() and skip() )
			++ _iterator;
	}

	bool skip()
	{
		if ( _iterator->path() == _labelsPath or _iterator->path() == _draftsPath  or
				_iterator->path() == _labelLogPath ) {
			_iterator.no_push();
			return true;
		}
		return _iterator->status().type() != boost::filesystem::regular_file;
	}
};

} // namespace ObjectStore
} // namespace Osmosis

#endif // __OSMOSIS_OBJECT_STORE_OBJECTS_ITERATOR_H__
