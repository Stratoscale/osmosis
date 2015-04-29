#ifndef __OSMOSIS_OBJECT_STORE_LABEL_LOG_ITERATOR_H__
#define __OSMOSIS_OBJECT_STORE_LABEL_LOG_ITERATOR_H__

namespace Osmosis {
namespace ObjectStore
{

class LabelLogIterator
{
public:
	LabelLogIterator( boost::filesystem::path rootPath )
	{
		rootPath /= DirectoryNames::LABEL_LOG;
		if ( boost::filesystem::is_directory( rootPath ) )
			for ( boost::filesystem::directory_iterator i( rootPath );
					i != boost::filesystem::directory_iterator(); ++ i ) {
				boost::filesystem::path path( i->path() );
				_logFiles.emplace( created( path ), path );
			}
		nextEntry();
	}

	LabelLogIterator( LabelLogIterator && other ):
		_logFiles( std::move( other._logFiles ) ),
		_entries( std::move( other._entries ) ),
		_entry( std::move( other._entry ) )
	{}

	const LabelLogEntry & operator * () const
	{
		ASSERT( _entry );
		return * _entry;
	}

	void next()
	{
		ASSERT( not done() );
		nextEntry();
	}

	bool done() const
	{
		return not _entry and _entries.empty() and _logFiles.empty();
	}

private:
	std::multimap< std::time_t, boost::filesystem::path >  _logFiles;
	std::multimap< std::time_t, LabelLogEntry >            _entries;
	std::unique_ptr< LabelLogEntry >                       _entry;
	
	static std::time_t created( const boost::filesystem::path & path )
	{
		std::string stem( path.stem().string() );
		size_t found = stem.find("__");
		if ( found == std::string::npos )
			THROW( Error, "Filename '" << path << "' does not contain '__'" );
		return std::stol( stem.substr( 0, found ) );
	}

	void readNextFile()
	{
		ASSERT( not _logFiles.empty() );
		auto last = -- _logFiles.end();
		boost::filesystem::path path( std::move( last->second ) );
		_logFiles.erase( last );

		std::ifstream input( path.string(), std::ios::binary );
		std::string line;
		while ( std::getline( input, line ) ) {
			LabelLogEntry entry( line );
			std::time_t timestamp = entry.time;
			_entries.emplace( timestamp, entry );
		}
	}

	void nextEntry()
	{
		_entry.reset( nullptr );
		while ( aFileWithFresherTimestampYetUnread() )
			readNextFile();
		while ( _entries.empty() and not _logFiles.empty() )
			readNextFile();
		if ( _entries.empty() )
			return;
		auto last = -- _entries.end();
		_entry.reset( new LabelLogEntry( std::move( last->second ) ) );
		_entries.erase( last );
	}

	bool aFileWithFresherTimestampYetUnread()
	{
		if ( _logFiles.empty() )
			return false;
		if ( _entries.empty() )
			return false;
		std::time_t freshest = ( -- _entries.end() )->first;
		return ( -- _logFiles.end() )->first >= freshest;
	}

	LabelLogIterator( const LabelLogIterator & rhs ) = delete;
	LabelLogIterator & operator= ( const LabelLogIterator & rhs ) = delete;
};

} // namespace ObjectStore
} // namespace Osmosis

#endif // __OSMOSIS_OBJECT_STORE_LABEL_LOG_ITERATOR_H__
