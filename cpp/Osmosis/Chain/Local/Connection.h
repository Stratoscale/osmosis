#ifndef __OSMOSIS_CHAIN_LOCAL_CONNECTION_H__
#define __OSMOSIS_CHAIN_LOCAL_CONNECTION_H__

#include "Osmosis/Chain/ObjectStoreConnectionInterface.h"
#include "Osmosis/Stream/CopyFile.h"
#include "Osmosis/ObjectStore/Store.h"
#include "Osmosis/ObjectStore/Drafts.h"
#include "Osmosis/ObjectStore/Labels.h"

namespace Osmosis {
namespace Chain {
namespace Local
{

class Connection : public ObjectStoreConnectionInterface
{
public:
	Connection(     Osmosis::ObjectStore::Store &   store,
			Osmosis::ObjectStore::Drafts &  drafts,
			Osmosis::ObjectStore::Labels &  labels ) :
		_store( store ),
		_drafts( drafts ),
		_labels( labels )
	{}

	void putString( const std::string & blob, const Hash & hash ) override
	{
		BACKTRACE_BEGIN
		boost::filesystem::path draft = _drafts.allocateFilename();
		Stream::WriteFile write( draft.string().c_str() );
		size_t offset = 0;
		static const unsigned long max_length = 4096;
		do {
			ASSERT( blob.size() >= offset );
			unsigned length = std::min( max_length, static_cast< unsigned long >( blob.size() - offset ) );
			write.write( offset, blob.c_str() + offset, length );
			offset += length;
		} while ( offset < blob.size() );
		_store.putExistingFile( hash, draft );
		BACKTRACE_END_VERBOSE( "Hash " << hash );
	}

	std::string getString( const Hash & hash ) override
	{
		BACKTRACE_BEGIN
		boost::filesystem::path original = _store.filenameForExisting( hash );
		std::ostringstream accumulator;
		Stream::ReadFile read( original.string().c_str() );
		while ( not read.done() ) {
			accumulator << std::string( reinterpret_cast< const char * >( read.buffer() ), read.length() );
			read.next();
		}
		return accumulator.str();
		BACKTRACE_END_VERBOSE( "Hash " << hash );
	}

	void putFile( const boost::filesystem::path & path, const Hash & hash ) override
	{
		BACKTRACE_BEGIN
		boost::filesystem::path draft = _drafts.allocateFilename();
		Stream::CopyFile( path.string().c_str(), draft.string().c_str() ).copy();
		_store.putExistingFile( hash, draft );
		BACKTRACE_END_VERBOSE( "Path " << path << " Hash " << hash );
	}

	void getFile( const boost::filesystem::path & path, const Hash & hash ) override
	{
		BACKTRACE_BEGIN
		boost::filesystem::path original = _store.filenameForExisting( hash );
		Stream::CopyFile( original.string().c_str(), path.string().c_str() ).copy();
		BACKTRACE_END_VERBOSE( "Path " << path << " Hash " << hash );
	}

	bool exists( const Hash & hash ) override
	{
		return _store.exists( hash );
	}

	void verify( const Hash & hash ) override
	{
		_store.verifyOrDestroy( hash );
	}

	void eraseLabel( const std::string & label ) override
	{
		_labels.erase( label );
	}

	void setLabel( const Hash & hash, const std::string & label ) override
	{
		_labels.label( hash, label );
	}

	Hash getLabel( const std::string & label ) override
	{
		return _labels.readLabel( label );
	}

	void renameLabel( const std::string & currentLabel, const std::string & renameLabelTo ) override
	{
		_labels.rename( currentLabel, renameLabelTo );
	}

	std::list< std::string > listLabels( const std::string & regex ) override
	{
		BACKTRACE_BEGIN
		std::list< std::string > result;
		for ( auto i = _labels.list( regex ); not i.done(); i.next() )
			result.emplace_back( * i );
		return result;
		BACKTRACE_END_VERBOSE( "Regex " << regex );
	}

private:
	Osmosis::ObjectStore::Store &   _store;
	Osmosis::ObjectStore::Drafts &  _drafts;
	Osmosis::ObjectStore::Labels &  _labels;

	Connection( const Connection & rhs ) = delete;
	Connection & operator= ( const Connection & rhs ) = delete;
};

} // namespace Local
} // namespace Chain
} // namespace Osmosis

#endif // __OSMOSIS_CHAIN_LOCAL_CONNECTION_H__
