#ifndef __OSMOSIS_OBJECT_STORE_LABELS_H__
#define __OSMOSIS_OBJECT_STORE_LABELS_H__

#include <fstream>
#include "Osmosis/FilesystemUtils.h"
#include "Osmosis/ObjectStore/LabelsIterator.h"
#include "Osmosis/ObjectStore/DirectoryNames.h"
#include "Osmosis/ObjectStore/LabelLogAppender.h"
#include "Common/Container.h"

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
		BACKTRACE_BEGIN
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
		BACKTRACE_END_VERBOSE( "Hash " << hash << " Label " << label );
	}

	bool exists( const std::string & label ) const
	{
		BACKTRACE_BEGIN
		if ( not FilesystemUtils::safeFilename( label ) )
			THROW( Error, "Label '" << label << "' contains forbidden characters" );
		bool result = boost::filesystem::exists( absoluteFilename( label ) );
		if ( result )
			_log.get( label, readLabelNoLog( label ) );
		return result;
		BACKTRACE_END_VERBOSE( "Label " << label );
	}

	Hash readLabelNoLog( const std::string & label ) const
	{
		BACKTRACE_BEGIN
		ASSERT( FilesystemUtils::safeFilename( label ) );
		std::string absolute =  absoluteFilename( label ).string();
		std::string hex;
		try {
			std::ifstream hashFile( absolute );
			hashFile >> hex;
		} catch ( std::exception &ex ) {
			TRACE_ERROR( "Cannot read label file '" << absolute << "'." );
			throw ex;
		}
		if ( hex.empty() ) {
			TRACE_WARNING("Label '" << label << "' file is empty. Erasing it.");
			boost::filesystem::remove( absoluteFilename( label ) );
			THROW( LabelFileIsCorrupted, "Empty label file" );
		}
		return Hash( hex );
		BACKTRACE_END_VERBOSE( "Label " << label );
	}

	Hash readLabel( const std::string & label ) const
	{
		BACKTRACE_BEGIN
		if ( not FilesystemUtils::safeFilename( label ) )
			THROW( Error, "Label '" << label << "' contains forbidden characters" );
		if ( not boost::filesystem::exists( absoluteFilename( label ) ) )
			THROW( Error, "Label '" << label << "' does not exist in object store." );
		Hash hash( readLabelNoLog( label ) );
		_log.get( label, hash );
		return hash;
		BACKTRACE_END_VERBOSE( "Label " << label );
	}

	void erase( const std::string & label )
	{
		BACKTRACE_BEGIN
		if ( not FilesystemUtils::safeFilename( label ) )
			THROW( Error, "Label '" << label << "' contains forbidden characters" );
		if ( not boost::filesystem::exists( absoluteFilename( label ) ) ) {
			TRACE_INFO( "Not Erasing label '" << label << "', does not exist" );
			return;
		}
		TRACE_INFO("Erasing label '" << label << "'");
		Container< Hash > hash;
		try {
		    hash.emplace( readLabelNoLog( label ) );
		} catch ( Error & e ) {
		    TRACE_WARNING( "Unable to read label hash when erasing, filling in with FFFF hash" );
		    hash.emplace( std::string( "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF" ) );
		}
		_log.remove( label, * hash );
		boost::filesystem::remove( absoluteFilename( label ) );
		BACKTRACE_END_VERBOSE( "Label " << label );
	}

	void rename( const std::string & from, const std::string & to )
	{
		BACKTRACE_BEGIN
		ASSERT( exists( from ) );
		ASSERT( not exists( to ) );
		ASSERT( FilesystemUtils::safeFilename( from ) );
		ASSERT( FilesystemUtils::safeFilename( to ) );
		boost::filesystem::rename( absoluteFilename( from ), absoluteFilename( to ) );
		Hash hash( readLabelNoLog( to ) );
		_log.remove( from, hash );
		_log.set( to, hash );
		BACKTRACE_END_VERBOSE( "From " << from << " To " << to );
	}

	LabelsIterator list( const std::string & regex ) const
	{
		LabelsIterator iterator( _labelsPath, regex );
		return std::move( iterator );
	}

	void flushLog()
	{
		BACKTRACE_BEGIN
		_log.write();
		BACKTRACE_END
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
