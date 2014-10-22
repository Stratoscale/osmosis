#ifndef __OSMOSIS_CLIENT_TRANSFER_H__
#define __OSMOSIS_CLIENT_TRANSFER_H__

#include "Osmosis/Client/TransferThread.h"

namespace Osmosis {
namespace Client
{

class Transfer
{
public:
	Transfer(       const std::string &            label,
			Chain::Chain &                 chain,
			Chain::ObjectStoreInterface &  destination ) :
		_label( label ),
		_chain( chain ),
		_destination( destination ),
		_toTransferQueue( 1 )
	{
		for ( unsigned i = 0; i < TRANSFER_THREADS; ++ i )
			_threads.push_back( std::thread(
				TransferThread::task, std::ref( _toTransferQueue ), std::ref( chain ), std::ref( destination ) ) );
	}

	~Transfer()
	{
		_toTransferQueue.abort();
		for ( auto & i : _threads )
			if ( i.joinable() )
				i.join();
	}

	void go()
	{
		auto connection = _destination.connect();
		checkLabelDoesNotExistInDestination( * connection );

		Chain::CheckOut checkOut( _chain.checkOut() );
		Hash labelHash = checkOut.getLabel( _label );
		DirList labelDirList = fetchDirList( labelHash, checkOut );

		populateTransferQueue( labelHash, labelDirList );
		for ( auto & i : _threads )
			i.join();
		connection->setLabel( labelHash, _label );
	}

private:
	enum {
		TRANSFER_THREADS = 10,
	};

	const std::string              _label;
	Chain::Chain &                 _chain;
	Chain::ObjectStoreInterface &  _destination;
	DigestedTaskQueue              _toTransferQueue;
	std::vector< std::thread >     _threads;

	void checkLabelDoesNotExistInDestination( Chain::ObjectStoreConnectionInterface & connection )
	{
		if ( connection.listLabels( "^" + _label + "$" ).size() > 0 )
			THROW( Error, "Label '" << _label << "' already exists at destination" );
	}

	void populateTransferQueue( const Hash & labelHash, const DirList & labelDirList )
	{
		_toTransferQueue.put( Digested( { boost::filesystem::path( "label" ), labelHash } ) );
		for ( auto & entry : labelDirList.entries() )
			if ( entry.hash )
				_toTransferQueue.put( Digested( { entry.path, * entry.hash } ) );
		_toTransferQueue.producerDone();
	}

	DirList fetchDirList( const Hash & labelHash, Chain::CheckOut & checkOut )
	{
		std::string dirListText = checkOut.getString( labelHash );
		if ( not CalculateHash::verify( dirListText.c_str(), dirListText.size(), labelHash ) )
			THROW( Error, "Dir list hash did not match contents" );
		std::istringstream dirListTextStream( std::move( dirListText ) );
		DirList labelDirList;
		dirListTextStream >> labelDirList;
		return std::move( labelDirList );
	}

	Transfer( const Transfer & rhs ) = delete;
	Transfer & operator= ( const Transfer & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_TRANSFER_H__
