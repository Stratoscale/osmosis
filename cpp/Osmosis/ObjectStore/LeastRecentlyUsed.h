#ifndef __OSMOSIS_OBJECT_STORE_LEAST_RECENTLY_USED_H__
#define __OSMOSIS_OBJECT_STORE_LEAST_RECENTLY_USED_H__

#include <unordered_set>
#include <string>
#include <boost/filesystem/path.hpp>
#include <Osmosis/ObjectStore/Store.h>
#include <Osmosis/ObjectStore/Labels.h>
#include "Osmosis/Debug.h"
#include "Common/Error.h"

namespace Osmosis {
namespace ObjectStore
{

class LeastRecentlyUsed
{
public:
	LeastRecentlyUsed(      const boost::filesystem::path &  rootPath,
				Store &                          store,
				Labels &                         labels,
				const std::string &              keepRegex,
				size_t                           maximumDiskUsage );

	void go();

private:
	enum { BLOCK_SIZE = 4096 };

	boost::filesystem::path            _rootPath;
	Store &                            _store;
	Labels &                           _labels;
	std::string                        _keepRegex;
	size_t                             _maximumDiskUsage;

	std::unordered_set< Hash >         _keptHashes;
	std::unordered_set< std::string >  _alreadyProcessedLabels;
	size_t                             _sizeOnDisk;
	std::time_t                        _startedAt;

	void addToKept( const std::list< Hash > & objects );

	std::list< Hash > newObjectsInLabel( const std::string & label );

	size_t totalSize( const std::list< Hash > & hashes );

	void keepAllLabelsMatchingKeepRegex();

	void keepLabelsFromLabelLogUntilExceedingMaximumDiskUsage();

	unsigned eraseAllNonProcessedLabels();

	unsigned eraseAllNonUsedHashes();

	LeastRecentlyUsed( const LeastRecentlyUsed & rhs ) = delete;
	LeastRecentlyUsed & operator= ( const LeastRecentlyUsed & rhs ) = delete;
};

} // namespace ObjectStore
} // namespace Osmosis

#endif // __OSMOSIS_OBJECT_STORE_LEAST_RECENTLY_USED_H__
