#ifndef __OSMOSIS_CLIENT_FETCH_JOINT_DIRLIST_FROM_LABELS_H__
#define __OSMOSIS_CLIENT_FETCH_JOINT_DIRLIST_FROM_LABELS_H__

namespace Osmosis {
namespace Client
{

class FetchJointDirlistFromLabels
{
public:
	FetchJointDirlistFromLabels( const std::vector< std::string > & labels, Chain::Chain & chain ):
		_labels( labels ),
		_checkOut( chain.checkOut() )
	{}

	DirList joined()
	{
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
	}

private:
	const std::vector< std::string >  _labels;
	Chain::CheckOut                   _checkOut;

	DirList getLabelDirList( const std::string & label )
	{
		std::istringstream dirListTextStream( std::move( getLabelDirListText( label ) ) );
		DirList labelDirList;
		dirListTextStream >> labelDirList;
		return std::move( labelDirList );
	}

	std::string getLabelDirListText( const std::string & label )
	{
		Hash hash = _checkOut.getLabel( label );
		std::string dirListText = _checkOut.getString( hash );
		if ( not CalculateHash::verify( dirListText.c_str(), dirListText.size(), hash ) )
			THROW( Error, "Dir list hash did not match contents" );
		TRACE_DEBUG( "Transferred dirList '" << label << "'" );
		return std::move( dirListText );
	}

	FetchJointDirlistFromLabels( const FetchJointDirlistFromLabels & rhs ) = delete;
	FetchJointDirlistFromLabels & operator= ( const FetchJointDirlistFromLabels & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_FETCH_JOINT_DIRLIST_FROM_LABELS_H__
