#include <boost/program_options.hpp>
#include "Osmosis/Server/Server.h"
#include "Osmosis/Client/CheckIn.h"
#include "Osmosis/Client/CheckOut.h"
#include "Osmosis/Client/LabelOps.h"

std::mutex globalTraceLock;

void server( const boost::program_options::variables_map & options )
{
	boost::filesystem::path rootPath( options[ "objectStoreRootPath" ].as< std::string >() );
	unsigned short port = options[ "serverTCPPort" ].as< unsigned short >();
	boost::asio::ip::tcp::endpoint endpoint( boost::asio::ip::tcp::v4(), port );

	Osmosis::ObjectStore::Store store( rootPath );
	Osmosis::ObjectStore::Drafts drafts( rootPath );
	Osmosis::ObjectStore::Labels labels( rootPath, store );
	Osmosis::Server::Server server( rootPath, endpoint, store, drafts, labels );
	server.run();
}

void checkIn( const boost::program_options::variables_map & options )
{
	boost::filesystem::path workDir = options[ "arg1" ].as< std::string >();
	std::string label = options[ "arg2" ].as< std::string >();
	Osmosis::Chain::Chain chain( options[ "objectStores" ].as< std::string >(), false );
	if ( chain.count() > 1 )
		THROW( Error, "--objectStores must contain one object store in a checkin operation" );
	bool md5 = options.count( "MD5" ) > 0;

	Osmosis::Client::CheckIn instance( workDir, label, chain.single(), md5 );
	instance.go();
}

void checkOut( const boost::program_options::variables_map & options )
{
	boost::filesystem::path workDir = options[ "arg1" ].as< std::string >();
	std::string label = options[ "arg2" ].as< std::string >();
	bool putIfMissing = options.count( "putIfMissing" ) > 0;
	Osmosis::Chain::Chain chain( options[ "objectStores" ].as< std::string >(), putIfMissing );
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
	}

	Osmosis::FilesystemUtils::clearUMask();
	Osmosis::Client::CheckOut instance( workDir, label, chain, md5, removeUnknownFiles, myUIDandGIDcheckout, ignores );
	instance.go();
}

void listLabels( const boost::program_options::variables_map & options )
{
	std::string labelRegex = ".*";
	if ( options.count( "arg1" ) > 0 )
		labelRegex = options[ "arg1" ].as< std::string >();
	boost::regex testExpression( labelRegex );
	Osmosis::Chain::Chain chain( options[ "objectStores" ].as< std::string >(), false );
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
	Osmosis::Chain::Chain chain( options[ "objectStores" ].as< std::string >(), false );
	if ( chain.count() > 1 )
		THROW( Error, "--objectStores must contain one object store in a erase operation" );
	Osmosis::Client::LabelOps instance( chain.single() );
	instance.eraseLabel( label );
}

void renameLabel( const boost::program_options::variables_map & options )
{
	std::string currentLabel = options[ "arg1" ].as< std::string >();
	std::string renameLabelTo = options[ "arg2" ].as< std::string >();
	Osmosis::Chain::Chain chain( options[ "objectStores" ].as< std::string >(), false );
	if ( chain.count() > 1 )
		THROW( Error, "--objectStores must contain one object store in a rename operation" );
	Osmosis::Client::LabelOps instance( chain.single() );
	instance.renameLabel( currentLabel, renameLabelTo );
}

void testHash( const boost::program_options::variables_map & options )
{
	boost::filesystem::path fileToHash = options[ "arg1" ].as< std::string >();
	bool md5 = options.count( "MD5" ) > 0;

	Osmosis::Hash hash = md5 ? Osmosis::CalculateHash::MD5( fileToHash ) : Osmosis::CalculateHash::SHA1( fileToHash );
	std::cout << hash << std::endl;
}

void usage( const boost::program_options::options_description & optionsDescription )
{
	std::cout << "osmosis.bin <command> [workDir] [label] [options]" << std::endl;
	std::cout << std::endl;
	std::cout << "  command:  can be 'server', 'checkin', 'checkout', 'listlabels'," << std::endl;
	std::cout << "            'eraselabel' or 'renamelabel'" << std::endl;
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
			"Path where osmosis will store objects. relevant for 'server' command" )
		("serverTCPPort", boost::program_options::value< unsigned short >()->default_value( 1010 ),
			"the TCP port to bind to, if command is 'server'")
		( "objectStores", boost::program_options::value< std::string >()->default_value( "127.0.0.1:1010" ),
			"the object store to act againt. May be a '+' seperated list for 'checkout' command" )
		( "MD5", "use MD5, not SHA1 for hash in 'checkin' operation" )
		( "putIfMissing", "when command is 'checkout' this flag will cause any objects received not from the "
		        "nearest object store to be put into all objects stores up to the one it was fetched from" )
		( "removeUnknownFiles", "for checkout: remove files from disk that are not in the dirlist being checked out" )
		( "myUIDandGIDcheckout", "for checkout: use my uid and gid" )
		( "ignore", boost::program_options::value< std::string >(),
			"for checkout: ignore the existance of all files in this ':' seperated list. "
			"if a directory was specified, ignored everything under it as well. specified paths "
			"must reside inside the checkout path" );

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
		else if ( command == "listlabels" )
			listLabels( options );
		else if ( command == "eraselabel" )
			eraseLabel( options );
		else if ( command == "renamelabel" )
			renameLabel( options );
		else if ( command == "testhash" )
			testHash( options );
		else {
			TRACE_ERROR( "Unknown command '" << command << "'" );
			usage( optionsDescription );
			return 1;
		}
	} catch ( boost::exception & e ) {
		TRACE_BOOST_EXCEPTION( e, "Terminated on a boost exception" );
		return 1;
	} catch ( Error & e ) {
		TRACE_ERROR( "Terminated on 'Error' exception: '" << e.what() << "' at " << e.filename << ':' << e.line );
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
