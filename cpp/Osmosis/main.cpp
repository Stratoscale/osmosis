#include <boost/program_options.hpp>
#include "Osmosis/Server/Server.h"
#include "Osmosis/Server/BroadcastServer.h"
#include "Osmosis/Client/CheckIn.h"
#include "Osmosis/Client/CheckOut.h"
#include "Osmosis/Client/Transfer.h"
#include "Osmosis/Client/LabelOps.h"
#include "Osmosis/Client/WhoHasLabel.h"
#include "Osmosis/ObjectStore/LabelLogIterator.h"
#include "Osmosis/ObjectStore/LeastRecentlyUsed.h"
#include "Osmosis/ObjectStore/Purge.h"
#include "Osmosis/FilesystemUtils.h"

std::mutex globalTraceLock;

void server( const boost::program_options::variables_map & options )
{
	boost::filesystem::path rootPath( options[ "objectStoreRootPath" ].as< std::string >() );
	unsigned short port = options[ "serverTCPPort" ].as< unsigned short >();
	boost::asio::ip::tcp::endpoint endpoint( boost::asio::ip::tcp::v4(), port );

	boost::asio::io_service ioService;
	Osmosis::Server::Server server( rootPath, endpoint, ioService );
	ioService.run();
}

void broadcastServer( const boost::program_options::variables_map & options )
{
	boost::filesystem::path rootPath( options[ "objectStoreRootPath" ].as< std::string >() );
	unsigned short port = options[ "serverUDPPort" ].as< unsigned short >();
	boost::asio::io_service ioService;
	Osmosis::Server::BroadcastServer server( ioService, rootPath, port );
	ioService.run();
}

boost::filesystem::path stripTrailingSlash( boost::filesystem::path path )
{
    auto str = path.string();
    if ( boost::algorithm::ends_with( str, "/" ) )
        return str.substr( 0, str.size() - 1 );
    else
        return path;
}

void checkIn( const boost::program_options::variables_map & options )
{
	BACKTRACE_BEGIN
	boost::filesystem::path reportFile = options[ "reportFile" ].as< std::string >();
	unsigned reportIntervalSeconds = options[ "reportIntervalSeconds" ].as< unsigned >();
	boost::filesystem::path workDir = stripTrailingSlash( options[ "arg1" ].as< std::string >() );
	std::string label = options[ "arg2" ].as< std::string >();
	Osmosis::Chain::Chain chain( options[ "objectStores" ].as< std::string >(), false, false );
	if ( chain.count() > 1 )
		THROW( Error, "--objectStores must contain one object store in a checkin operation" );
	bool md5 = options.count( "MD5" ) > 0;

	boost::filesystem::path draftsPath = workDir / Osmosis::ObjectStore::DirectoryNames::DRAFTS;
	if ( boost::filesystem::exists( draftsPath ) )
		THROW( Error, "workDir must not contain " << draftsPath );

	Osmosis::Client::CheckIn instance( workDir, label, chain.single(), md5, reportFile, reportIntervalSeconds );
	instance.go();
	BACKTRACE_END
}

void checkOut( const boost::program_options::variables_map & options )
{
	BACKTRACE_BEGIN
	boost::filesystem::path reportFile = options[ "reportFile" ].as< std::string >();
	unsigned reportIntervalSeconds = options[ "reportIntervalSeconds" ].as< unsigned >();
	boost::filesystem::path workDir = stripTrailingSlash( options[ "arg1" ].as< std::string >() );
	std::string label = options[ "arg2" ].as< std::string >();
	bool putIfMissing = options.count( "putIfMissing" ) > 0;
	bool chainTouch = options.count( "noChainTouch" ) == 0;
	Osmosis::Chain::Chain chain( options[ "objectStores" ].as< std::string >(), putIfMissing, chainTouch );
	bool md5 = options.count( "MD5" ) > 0;
	bool removeUnknownFiles = options.count( "removeUnknownFiles" ) > 0;
	bool myUIDandGIDcheckout = options.count( "myUIDandGIDcheckout" ) > 0;
	std::vector< std::string > ignores;
	if ( options.count( "ignore" ) > 0 )
		boost::split( ignores, options[ "ignore" ].as< std::string >(), boost::is_any_of( ":" ) );
	workDir = boost::filesystem::absolute( workDir );
	std::string workDirString = workDir.string();
	for ( auto & ignore : ignores ) {
		ignore = std::move( boost::filesystem::absolute( ignore ).string() );
		if ( ignore.size() < workDirString.size() or
				ignore.substr( 0, workDirString.size() ) != workDirString )
			THROW( Error, "ignore '" << ignore << "' is not under checkout path '" << workDirString << "'" );
		TRACE_INFO( "will ignore '" << ignore << "'" );
	}

	boost::filesystem::path draftsPath = workDir / Osmosis::ObjectStore::DirectoryNames::DRAFTS;
	boost::filesystem::remove_all( draftsPath );

	Osmosis::FilesystemUtils::clearUMask();
	Osmosis::Client::Ignores ignoresInstance( ignores );
	ignoresInstance.append( draftsPath.string() );
	Osmosis::Client::CheckOut instance( workDir, label, chain, md5, removeUnknownFiles,
			myUIDandGIDcheckout, ignoresInstance, reportFile, reportIntervalSeconds, chainTouch );
	instance.go();
	BACKTRACE_END
}

void transfer( const boost::program_options::variables_map & options )
{
	std::string label = options[ "arg1" ].as< std::string >();
	bool putIfMissing = options.count( "putIfMissing" ) > 0;
	bool chainTouch = options.count( "noChainTouch" ) == 0;
	Osmosis::Chain::Chain chain( options[ "objectStores" ].as< std::string >(), putIfMissing, chainTouch );
	auto destination = Osmosis::Chain::factory( options[ "transferDestination" ].as< std::string >() );
	Osmosis::Client::Transfer instance( label, chain, * destination );
	instance.go();
}

void listLabels( const boost::program_options::variables_map & options )
{
	std::string labelRegex = ".*";
	if ( options.count( "arg1" ) > 0 )
		labelRegex = options[ "arg1" ].as< std::string >();
	boost::regex testExpression( labelRegex );
	Osmosis::Chain::Chain chain( options[ "objectStores" ].as< std::string >(), false, false );
	if ( chain.count() > 1 )
		THROW( Error, "--objectStores must contain one object store in a list operation" );

	Osmosis::Client::LabelOps instance( chain.single() );
	std::list< std::string > result = instance.listLabels( labelRegex );
	for ( auto & i : result )
		std::cout << i << std::endl;
}

void eraseLabel( const boost::program_options::variables_map & options )
{
	std::string label = options[ "arg1" ].as< std::string >();
	Osmosis::Chain::Chain chain( options[ "objectStores" ].as< std::string >(), false, false );
	if ( chain.count() > 1 )
		THROW( Error, "--objectStores must contain one object store in a erase operation" );
	Osmosis::Client::LabelOps instance( chain.single() );
	instance.eraseLabel( label );
}

void purge( const boost::program_options::variables_map & options )
{
	boost::filesystem::path rootPath( options[ "objectStoreRootPath" ].as< std::string >() );
	Osmosis::ObjectStore::Store store( rootPath );
	Osmosis::ObjectStore::Labels labels( rootPath, store );
	Osmosis::ObjectStore::Purge purge( store, labels );
	purge.purge();
}

void renameLabel( const boost::program_options::variables_map & options )
{
	std::string currentLabel = options[ "arg1" ].as< std::string >();
	std::string renameLabelTo = options[ "arg2" ].as< std::string >();
	Osmosis::Chain::Chain chain( options[ "objectStores" ].as< std::string >(), false, false );
	if ( chain.count() > 1 )
		THROW( Error, "--objectStores must contain one object store in a rename operation" );
	Osmosis::Client::LabelOps instance( chain.single() );
	instance.renameLabel( currentLabel, renameLabelTo );
}

void dumpLabelLog( const boost::program_options::variables_map & options )
{
	boost::filesystem::path rootPath( options[ "objectStoreRootPath" ].as< std::string >() );
	Osmosis::ObjectStore::LabelLogIterator iterator( rootPath );
	for ( ; not iterator.done(); iterator.next() ) {
		auto & entry = * iterator;
		std::cout << entry.time << '\t' << static_cast< char >( entry.operation ) << '\t' << entry.label << std::endl;
	}
}

size_t parseSizeArgument( const std::string & argument )
{
	if ( argument.size() < ( sizeof( "1G" ) - sizeof( '\0' ) ) )
		THROW( Error, "Inavlid format for --maximumDiskUsage" );
	switch ( argument[ argument.size() - 1 ] ) {
		case 'G':
		case 'g':
			return std::stol( argument.substr( 0, argument.size() - 1 ) ) * 1024 * 1024 * 1024;
			break;
		case 'M':
		case 'm':
			return std::stol( argument.substr( 0, argument.size() - 1 ) ) * 1024 * 1024;
			break;
		case 'K':
		case 'k':
			return std::stol( argument.substr( 0, argument.size() - 1 ) ) * 1024;
			break;
		default:
			THROW( Error, "Unable to parse '" << argument << "': last letter is not G or M" );
	}
}

void leastRecentlyUsed( const boost::program_options::variables_map & options )
{
	boost::filesystem::path rootPath( options[ "objectStoreRootPath" ].as< std::string >() );
	std::string keepRegex = options[ "keep" ].as< std::string >();
	boost::regex testExpression( keepRegex );
	size_t maximumDiskUsage = parseSizeArgument( options[ "maximumDiskUsage" ].as< std::string >() );

	Osmosis::ObjectStore::Store store( rootPath );
	Osmosis::ObjectStore::Labels labels( rootPath, store );
	Osmosis::ObjectStore::LeastRecentlyUsed leastRecentlyUsed( rootPath, store, labels, keepRegex, maximumDiskUsage );
	leastRecentlyUsed.go();
}

void testHash( const boost::program_options::variables_map & options )
{
	boost::filesystem::path fileToHash = options[ "arg1" ].as< std::string >();
	bool md5 = options.count( "MD5" ) > 0;

	Osmosis::Hash hash = md5 ? Osmosis::CalculateHash::MD5( fileToHash ) : Osmosis::CalculateHash::SHA1( fileToHash );
	std::cout << hash << std::endl;
}

void whoHasLabel( const boost::program_options::variables_map & options )
{
	if ( options.count( "arg1" ) == 0 )
		THROW( Error, "Label must be supplied as first argument" );
	const std::string label = options[ "arg1" ].as< std::string >();
	const unsigned short timeout = options[ "timeout" ].as< unsigned short >();
	unsigned short serverUDPPort = options[ "serverUDPPort" ].as< unsigned short >();
	bool broadcastToLocalhost = options.count( "broadcastToLocalhost" ) > 0;
	Osmosis::Client::WhoHasLabel instance( label, timeout, serverUDPPort, broadcastToLocalhost );
	std::list< std::string > result = std::move( instance.go() );
	for ( auto & i : result )
		std::cout << i << std::endl;
}

void usage( const boost::program_options::options_description & optionsDescription )
{
	std::cout << "osmosis.bin <command> [workDir] [label] [options]" << std::endl;
	std::cout << std::endl;
	std::cout << "  command:  can be 'server', 'checkin', 'checkout', 'transfer', " << std::endl;
	std::cout << "            'listlabels', 'eraselabel', 'renamelabel', 'purge', " << std::endl;
	std::cout << "            'labellog' or 'leastrecentlyused', 'broadcastserver" << std::endl;
	std::cout << "  workDir:  must be present if command is 'checkin' or 'checkout'" << std::endl;
	std::cout << "            workDir is the path to check in from or check out to" << std::endl;
	std::cout << "  label:    must be present if command is 'checkin', 'checkout' or" << std::endl;
	std::cout << "            'eraselabel'. label is the name of the dirlist to checkin" << std::endl;
	std::cout << "            or checkout." << std::endl;
	std::cout << "            for checkout operation, the special value of '+' means" << std::endl;
	std::cout << "            to read the label name from the first line of standard" << std::endl;
	std::cout << "            input. This is efficient as a form of standby mode, since" << std::endl;
	std::cout << "            hashing of the local working directory is performed while" << std::endl;
	std::cout << "            waiting for the label name." << std::endl;
	std::cout << "            optional for 'listlabels' command, in which case is" << std::endl;
	std::cout << "            treated as a regex" << std::endl;
	std::cout << "            for 'renamelabel', two labels are expected: current and" << std::endl;
	std::cout << "            target label. target label must not already exist." << std::endl;
	std::cout << "  testhash: just hash a localfile" << std::endl;
	std::cout << std::endl;
	std::cout << optionsDescription << std::endl;
}

int main( int argc, char * argv [] )
{
	boost::program_options::options_description optionsDescription( "options" );
	optionsDescription.add_options()
		("help", "produce help message")
		("objectStoreRootPath", boost::program_options::value< std::string >()->default_value( "/var/lib/osmosis/objectstore" ),
			"Path where osmosis will store objects. relevant for 'server', 'purge', 'labellog' and 'leastrecentlyused' commands" )
		("serverTCPPort", boost::program_options::value< unsigned short >()->default_value( 1010 ),
			"the TCP port to bind to, if command is 'server'")
		("serverUDPPort", boost::program_options::value< unsigned short >()->default_value( 2020 ),
			"the UDP port to bind to, if command is 'broadcastserver'")
		( "objectStores", boost::program_options::value< std::string >()->default_value( "127.0.0.1:1010" ),
			"the object store to act againt. May be a '+' seperated list for 'checkout' command" )
		( "MD5", "use MD5, not SHA1 for hash in 'checkin' operation" )
		( "putIfMissing", "when command is 'checkout' or 'transfer', this flag will cause any objects received not from the "
		        "nearest object store to be put into all objects stores up to the one it was fetched from" )
		( "removeUnknownFiles", "for checkout: remove files from disk that are not in the dirlist being checked out" )
		( "myUIDandGIDcheckout", "for checkout: use my uid and gid" )
		( "ignore", boost::program_options::value< std::string >(),
			"for checkout: ignore the existance of all files in this ':' seperated list. "
			"if a directory was specified, ignored everything under it as well. specified paths "
			"must reside inside the checkout path" )
		( "transferDestination", boost::program_options::value< std::string >(),
			"destination object store to transfer the label into" )
		( "reportFile", boost::program_options::value< std::string >()->default_value( "" ),
			"periodically write report in JSON format into this file" )
		( "reportIntervalSeconds", boost::program_options::value< unsigned >()->default_value( 15 ),
			"period to report progress" )
		( "noChainTouch", "avoid touching fetched label in all object stores in chain (used for label bookeeping)" )
		( "keep", boost::program_options::value< std::string >()->default_value( "keepforever|bootstrap" ),
		  	"regular expression for labels to never erase. Only relevant under 'leastrecentlyused' command" )
		( "maximumDiskUsage", boost::program_options::value< std::string >(),
		  	"<number>M or <number>G for the amount of storage used for label objects before 'leastrecentlyused' starts erasing labels")
		( "broadcastToLocalhost", "Use this to broadcast to 127.0.0.7" )
		("timeout", boost::program_options::value< unsigned short >()->default_value( 1000 ),
			"Timeout in seconds, for the 'whohaslabel' command");

	boost::program_options::options_description positionalDescription( "positionals" );
	positionalDescription.add_options()
		( "command", boost::program_options::value< std::string >() )
		( "arg1", boost::program_options::value< std::string >() )
		( "arg2", boost::program_options::value< std::string >() );

	boost::program_options::positional_options_description positionalMapping;
	positionalMapping.add( "command", 1 ).add( "arg1", 1 ).add( "arg2", 1 );

	boost::program_options::options_description allOptions;
	allOptions.add( optionsDescription ).add( positionalDescription );

	boost::program_options::variables_map options;
	try {
		boost::program_options::store(
			boost::program_options::command_line_parser( argc, argv ).
				positional( positionalMapping ).options( allOptions ).
				run(),
			options );
		boost::program_options::notify( options );
	} catch ( boost::exception & e ) {
		TRACE_BOOST_EXCEPTION( e, "Unable to parse command line" );
		usage( optionsDescription );
		return 1;
	}

	if ( options.count( "help" ) ) {
		usage( optionsDescription );
		return 1;
	}

	try {
		std::string command = options[ "command" ].as< std::string >();
		if ( command == "server" ) {
			if ( options.count( "arg1" ) > 0 or options.count( "arg2" ) > 0 ) {
				TRACE_ERROR( "'workDir' or 'label' must not be present in command line"
						"if 'server' is specified as the command" );
				usage( optionsDescription );
				return 1;
			}
			server( options );
		} else if ( command == "checkin" )
			checkIn( options );
		else if ( command == "checkout" )
			checkOut( options );
		else if ( command == "transfer" )
			transfer( options );
		else if ( command == "listlabels" )
			listLabels( options );
		else if ( command == "eraselabel" )
			eraseLabel( options );
		else if ( command == "purge" )
			purge( options );
		else if ( command == "renamelabel" )
			renameLabel( options );
		else if ( command == "labellog" )
			dumpLabelLog( options );
		else if ( command == "leastrecentlyused" )
			leastRecentlyUsed( options );
		else if ( command == "testhash" )
			testHash( options );
		else if ( command == "broadcastserver" )
			broadcastServer( options );
		else if ( command == "whohaslabel" )
			whoHasLabel( options );
		else {
			TRACE_ERROR( "Unknown command '" << command << "'" );
			usage( optionsDescription );
			return 1;
		}
	} catch ( boost::exception & e ) {
		TRACE_BOOST_EXCEPTION( e, "Terminated on a boost exception" );
		return 1;
	} catch ( Error & e ) {
		TRACE_ERROR( "Terminated on 'Error' exception: '" << e.what() << "' from " << e.backtrace() );
		return 1;
	} catch ( std::exception & e ) {
		TRACE_ERROR( "Terminated on std::exception: '" << e.what() );
		return 1;
	} catch ( ... ) {
		TRACE_ERROR( "Terminated on unknown exception" );
		return 1;
	}

	return 0;
}
