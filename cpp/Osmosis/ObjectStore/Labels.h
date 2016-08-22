#ifndef __OSMOSIS_OBJECT_STORE_LABELS_H__
#define __OSMOSIS_OBJECT_STORE_LABELS_H__

#include <fstream>
#include "Osmosis/ObjectStore/LabelsIterator.h"
#include "Osmosis/ObjectStore/LabelLogAppender.h"
#include "Osmosis/ObjectStore/Store.h"

namespace Osmosis {
namespace ObjectStore
{

class Labels
{
public:
	Labels( const boost::filesystem::path & rootPath, const Store & store );

	void label( const Hash & hash, const std::string & label );

	bool exists( const std::string & label ) const;

	Hash readLabelNoLog( const std::string & label ) const;

	Hash readLabel( const std::string & label ) const;

	void erase( const std::string & label );

	void rename( const std::string & from, const std::string & to );

	LabelsIterator list( const std::string & regex ) const;

	void flushLog();

private:
	boost::filesystem::path   _rootPath;
	const Store &             _store;
	boost::filesystem::path   _labelsPath;
	mutable LabelLogAppender  _log;

	boost::filesystem::path absoluteFilename( const std::string & label ) const;

	Labels( const Labels & rhs ) = delete;
	Labels & operator= ( const Labels & rhs ) = delete;
};

} // namespace ObjectStore
} // namespace Osmosis

#endif // __OSMOSIS_OBJECT_STORE_LABELS_H__
