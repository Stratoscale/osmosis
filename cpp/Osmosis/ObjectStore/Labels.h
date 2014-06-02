#ifndef __OSMOSIS_OBJECT_STORE_LABELS_H__
#define __OSMOSIS_OBJECT_STORE_LABELS_H__

#include <fstream>
#include "Osmosis/FilesystemUtils.h"
#include "Osmosis/ObjectStore/LabelsIterator.h"
#include "Osmosis/ObjectStore/DirectoryNames.h"

namespace Osmosis {
namespace ObjectStore
{

class Labels
{
public:
	Labels( const boost::filesystem::path & rootPath, const Store & store ) :
		_rootPath( rootPath ),
		_store( store ),
		_labelsPath( rootPath / DirectoryNames::LABELS )
	{
		if ( not boost::filesystem::is_directory( _labelsPath ) )
			boost::filesystem::create_directories( _labelsPath );
	}

	void label( const Hash & hash, const std::string & label )
	{
		if ( not _store.exists( hash ) )
			THROW( Error, "Hash " << hash << " does not exist in store. Can't apply "
					"label '" << label << "'" );
		if ( not FilesystemUtils::safeFilename( label ) )
			THROW( Error, "Label '" << label << "' contains forbidden characters" );
		std::ofstream hashFile( absoluteFilename( label ).string() );
		hashFile << hash;
	}

	bool exists( const std::string & label ) const
	{
		if ( not FilesystemUtils::safeFilename( label ) )
			THROW( Error, "Label '" << label << "' contains forbidden characters" );
		return boost::filesystem::exists( absoluteFilename( label ) );
	}

	Hash readLabel( const std::string & label ) const
	{
		if ( not FilesystemUtils::safeFilename( label ) )
			THROW( Error, "Label '" << label << "' contains forbidden characters" );
		std::ifstream hashFile( absoluteFilename( label ).string() );
		std::string hex;
		hashFile >> hex;
		return Hash::fromHex( hex );
	}

	void erase( const std::string & label )
	{
		if ( not FilesystemUtils::safeFilename( label ) )
			THROW( Error, "Label '" << label << "' contains forbidden characters" );
		boost::filesystem::remove( absoluteFilename( label ) );
	}

	LabelsIterator list( const std::string & regex ) const
	{
		LabelsIterator iterator( _labelsPath, regex );
		return std::move( iterator );
	}

private:
	boost::filesystem::path  _rootPath;
	const Store &            _store;
	boost::filesystem::path  _labelsPath; 

	boost::filesystem::path absoluteFilename( const std::string & label ) const
	{
		return _labelsPath / label;
	}

	Labels( const Labels & rhs ) = delete;
	Labels & operator= ( const Labels & rhs ) = delete;
};

} // namespace ObjectStore
} // namespace Osmosis

#endif // __OSMOSIS_OBJECT_STORE_LABELS_H__
