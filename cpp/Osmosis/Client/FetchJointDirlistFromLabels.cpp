#include <vector>
#include "Osmosis/Client/FetchJointDirlistFromLabels.h"
#include "Common/Error.h"
#include "Osmosis/DirList.h"
#include "Osmosis/CalculateHash.h"
#include "Osmosis/Chain/Chain.h"

namespace Osmosis {
namespace Client
{

FetchJointDirlistFromLabels::FetchJointDirlistFromLabels( const std::vector< std::string > & labels, Chain::Chain & chain, bool chainTouch ):
	_labels( labels ),
	_checkOut( chain.checkOut() ),
	_chainTouch( chainTouch )
{}

DirList FetchJointDirlistFromLabels::joined()
{
	BACKTRACE_BEGIN
	DirList result;
	for ( auto & label : _labels ) {
		DirList dirList = getLabelDirList( label );
		for ( auto & entry : dirList.entries() ) {
			const DirListEntry * found = result.find( entry.path );
			if ( found == nullptr ) {
				result.add( entry.path, entry.status );
				if ( entry.hash )
					result.setHash( entry.path, * entry.hash );
			} else {
				if ( not entry.status.equalsButTime( found->status ) )
					THROW( Error, "The file '" << entry.path << "' exists with different " <<
						"statuses, while joining labels (current " << label << ")" );
				if ( entry.hash ) {
					ASSERT( found->hash );
					if ( * entry.hash != * found->hash )
						THROW( Error, "Check out aborted half way, while joining labels:" <<
							" the label '" << label << "' contains the file '" <<
							entry.path << "' with different contents than other labels" );
				} else {
					ASSERT( not found->hash );
				}
			}
		}
	}
	return std::move( result );
	BACKTRACE_END
}

DirList FetchJointDirlistFromLabels::getLabelDirList( const std::string & label )
{
	BACKTRACE_BEGIN
	std::istringstream dirListTextStream( std::move( getLabelDirListText( label ) ) );
	DirList labelDirList;
	dirListTextStream >> labelDirList;
	return std::move( labelDirList );
	BACKTRACE_END
}

std::string FetchJointDirlistFromLabels::getLabelDirListText( const std::string & label )
{
	BACKTRACE_BEGIN
	Hash hash = _checkOut.getLabel( label );
	std::string dirListText = _checkOut.getString( hash );
	if ( not CalculateHash::verify( dirListText.c_str(), dirListText.size(), hash ) )
		THROW( Error, "Dir list hash did not match contents" );
	TRACE_DEBUG( "Transferred dirList '" << label << "'" );
	return std::move( dirListText );
	BACKTRACE_END
}

} // namespace Client
} // namespace Osmosis
