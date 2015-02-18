#ifndef __OSMOSIS_OBJECT_STORE_LABELS_H__
#define __OSMOSIS_OBJECT_STORE_LABELS_H__

#include <fstream>
#include "Osmosis/FilesystemUtils.h"
#include "Osmosis/ObjectStore/LabelsIterator.h"
#include "Osmosis/ObjectStore/DirectoryNames.h"
#include "Osmosis/ObjectStore/LabelLogAppender.h"

namespace Osmosis {
namespace ObjectStore
{

class Labels
{
public:
	Labels( const boost::filesystem::path & rootPath, const Store & store ) :
		_rootPath( rootPath ),
		_store( store ),
		_labelsPath( rootPath / DirectoryNames::LABELS ),
		_log( rootPath )
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
		{
			std::ofstream hashFile( absoluteFilename( label ).string() );
			hashFile << hash;
		}
		_log.set( label, hash );
	}

	bool exists( const std::string & label ) const
	{
		if ( not FilesystemUtils::safeFilename( label ) )
			THROW( Error, "Label '" << label << "' contains forbidden characters" );
		bool result = boost::filesystem::exists( absoluteFilename( label ) );
		if ( result )
			_log.get( label, readLabelNoLog( label ) );
		return result;
	}

	Hash readLabelNoLog( const std::string & label ) const
	{
		ASSERT( FilesystemUtils::safeFilename( label ) );
		std::ifstream hashFile( absoluteFilename( label ).string() );
		std::string hex;
		hashFile >> hex;
		return Hash( hex );
	}

	Hash readLabel( const std::string & label ) const
	{
		if ( not FilesystemUtils::safeFilename( label ) )
			THROW( Error, "Label '" << label << "' contains forbidden characters" );
		Hash hash( readLabelNoLog( label ) );
		_log.get( label, hash );
		return hash;
	}

	void erase( const std::string & label )
	{
		if ( not FilesystemUtils::safeFilename( label ) )
			THROW( Error, "Label '" << label << "' contains forbidden characters" );
		if ( not boost::filesystem::exists( absoluteFilename( label ) ) ) {
			TRACE_INFO( "Not Erasing label '" << label << "', does not exist" );
			return;
		}
		TRACE_INFO("Erasing label '" << label << "'");
		_log.remove( label, readLabelNoLog( label ) );
		boost::filesystem::remove( absoluteFilename( label ) );
	}

	void rename( const std::string & from, const std::string & to )
	{
		ASSERT( exists( from ) );
		ASSERT( not exists( to ) );
		ASSERT( FilesystemUtils::safeFilename( from ) );
		ASSERT( FilesystemUtils::safeFilename( to ) );
		boost::filesystem::rename( absoluteFilename( from ), absoluteFilename( to ) );
		Hash hash( readLabelNoLog( to ) );
		_log.remove( from, hash );
		_log.set( to, hash );
	}

	LabelsIterator list( const std::string & regex ) const
	{
		LabelsIterator iterator( _labelsPath, regex );
		return std::move( iterator );
	}

	void flushLog()
	{
		_log.write();
	}

private:
	boost::filesystem::path   _rootPath;
	const Store &             _store;
	boost::filesystem::path   _labelsPath;
	mutable LabelLogAppender  _log;

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
