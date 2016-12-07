#ifndef __OSMOSIS_CLIENT_FETCH_JOINT_DIRLIST_FROM_LABELS_H__
#define __OSMOSIS_CLIENT_FETCH_JOINT_DIRLIST_FROM_LABELS_H__

#include <vector>
#include <string>
#include "Osmosis/DirList.h"
#include "Osmosis/Chain/Chain.h"

namespace Osmosis {
namespace Client
{

class FetchJointDirlistFromLabels
{
public:
	FetchJointDirlistFromLabels( const std::vector< std::string > & labels, Chain::Chain & chain, bool chainTouch );

	DirList joined();

private:
	const std::vector< std::string >  _labels;
	Chain::CheckOut                   _checkOut;
	bool                              _chainTouch;

	DirList getLabelDirList( const std::string & label );

	std::string getLabelDirListText( const std::string & label );

	FetchJointDirlistFromLabels( const FetchJointDirlistFromLabels & rhs ) = delete;
	FetchJointDirlistFromLabels & operator= ( const FetchJointDirlistFromLabels & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_FETCH_JOINT_DIRLIST_FROM_LABELS_H__
