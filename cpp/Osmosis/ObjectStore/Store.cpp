#include "Osmosis/ObjectStore/Store.h"
#include "Osmosis/Hash.h"
#include "Osmosis/CalculateHash.h"
#include "Osmosis/ObjectStore/ObjectsIterator.h"

namespace Osmosis {
namespace ObjectStore
{

Store::Store( const boost::filesystem::path & rootPath ) :
	_rootPath( rootPath )
{}

bool Store::exists( const Hash & hash ) const
{
	BACKTRACE_BEGIN
	return boost::filesystem::exists( absoluteFilename( hash ) );
	BACKTRACE_END_VERBOSE( "Hash " << hash );
}

void Store::verifyOrDestroy( const Hash & hash ) const
{
	BACKTRACE_BEGIN
	boost::filesystem::path absolute = absoluteFilename( hash );
	if ( not boost::filesystem::exists( absolute ) ) {
		TRACE_WARNING( "Verify for a non existing object: " << hash );
		return;
	}
	if ( not CalculateHash::verify( absolute, hash ) ) {
		TRACE_ERROR( "Object " << hash << " is malformed, removing from object store" );
		boost::filesystem::remove( absolute );
	}
	BACKTRACE_END_VERBOSE( "Hash " << hash );
}

boost::filesystem::path Store::filenameForExisting( const Hash & hash ) const
{
	BACKTRACE_BEGIN
	ASSERT( exists( hash ) );
	return absoluteFilename( hash );
	BACKTRACE_END_VERBOSE( "Hash " << hash );
}

void Store::putExistingFile( const Hash & hash, const boost::filesystem::path & filename )
{
	BACKTRACE_BEGIN
	if ( not CalculateHash::verify( filename, hash ) )
		THROW( Error, "Will not put file that does not match it's hash " << hash );
	boost::filesystem::path absolute = absoluteFilename( hash );
	boost::filesystem::path dirPath = absolute.parent_path();
	recoverInCaseOfDirectoryCorruption( dirPath );
	boost::filesystem::create_directories( dirPath );
	boost::filesystem::rename( filename, absolute );
	BACKTRACE_END_VERBOSE( "Hash " << hash << " Filename " << filename );
}

void Store::erase( const Hash & hash )
{
	BACKTRACE_BEGIN
	boost::filesystem::path absolute = absoluteFilename( hash );
	boost::filesystem::remove( absolute );
	BACKTRACE_END_VERBOSE( "Hash " << hash );
}

ObjectsIterator Store::list() const
{
	ObjectsIterator iterator( _rootPath );
	return std::move( iterator );
}

boost::filesystem::path Store::absoluteFilename( const Hash & hash ) const
{
	return _rootPath / hash.relativeFilename();
}

void Store::recoverInCaseOfDirectoryCorruption( boost::filesystem::path dirPath ) {
	if ( not boost::filesystem::exists( dirPath ) )
		return;
	const boost::filesystem::file_status status = boost::filesystem::status(dirPath);
	const boost::filesystem::file_type fileType = status.type();
	if ( fileType != boost::filesystem::file_type::directory_file ) {
		TRACE_WARNING("Trying to recover from a possible corruption in the local cache;"
						<< " The following path is unexpectedly of type " << fileType <<
						" (instead of a directory): '" << dirPath << "'.");
		boost::filesystem::remove( dirPath );
	}
}

} // namespace ObjectStore
} // namespace Osmosis
