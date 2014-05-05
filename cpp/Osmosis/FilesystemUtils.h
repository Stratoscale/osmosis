#ifndef __OSMOSIS_FILESYSTEM_UTILS_H__
#define __OSMOSIS_FILESYSTEM_UTILS_H__

namespace Osmosis {
namespace FilesystemUtils
{

static inline bool safeFilename( const std::string & filename )
{
	return filename.find( "\\" ) == std::string::npos and
		filename.find( "/" ) == std::string::npos and
		filename.find( "<" ) == std::string::npos and
		filename.find( ">" ) == std::string::npos and
		filename.find( "|" ) == std::string::npos and
		filename.find( "\"" ) == std::string::npos and
		filename.find( ":" ) == std::string::npos and
		filename.find( "?" ) == std::string::npos and
		filename.find( "*" ) == std::string::npos and
		filename.find( "\t" ) == std::string::npos;
}

} // namespace FilesystemUtils
} // namespace Osmosis

#endif // __OSMOSIS_FILESYSTEM_UTILS_H__
