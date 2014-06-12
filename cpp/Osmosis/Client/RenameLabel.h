#ifndef __OSMOSIS_CLIENT_RENAME_LABEL_H__
#define __OSMOSIS_CLIENT_RENAME_LABEL_H__

namespace Osmosis {
namespace Client
{

class RenameLabel
{
public:
	RenameLabel(    const std::string &  currentLabel,
			const std::string &  renameLabelTo,
			const std::string &  hostname,
			unsigned short       port ) :
		_currentLabel( currentLabel ),
		_renameLabelTo( renameLabelTo ),
		_connection( hostname, port ),
		_labelOps( _connection.socket() )
	{}

	void go()
	{
		sendRenameCommand( _currentLabel, _renameLabelTo );
		Stream::AckOps( _connection.socket() ).wait( "Rename operation" );
	}

private:
	const std::string         _currentLabel;
	const std::string         _renameLabelTo;
	Connect                   _connection;
	LabelOps                  _labelOps;

	void sendRenameCommand( const std::string & from, const std::string & to )
	{
		unsigned char buffer[ 2048 ];
		struct Tongue::Header * header = reinterpret_cast< struct Tongue::Header * >( buffer );
		struct Tongue::Label * label1Header = reinterpret_cast< struct Tongue::Label * >( header + 1 );
		unsigned char * label1Text = reinterpret_cast< unsigned char * >( label1Header + 1 );
		struct Tongue::Label * label2Header = reinterpret_cast< struct Tongue::Label * >( label1Text + from.size() );
		unsigned char * label2Text = reinterpret_cast< unsigned char * >( label2Header + 1 );
		size_t size = reinterpret_cast< unsigned char * >( label2Text + to.size() ) - buffer;
		if ( size > sizeof( buffer ) ) 
			THROW( Error, "Label too long" );

		header->opcode = static_cast< unsigned char >( Tongue::Opcode::RENAME_LABEL );

		ASSERT( from.size() < ( 1 << 16 ) );
		label1Header->length = static_cast< unsigned short >( from.size() );
		memcpy( label1Text, from.c_str(), from.size() );

		ASSERT( to.size() < ( 1 << 16 ) );
		label2Header->length = static_cast< unsigned short >( to.size() );
		memcpy( label2Text, to.c_str(), to.size() );

		_connection.socket().sendAll( buffer, size );
	}

	RenameLabel( const RenameLabel & rhs ) = delete;
	RenameLabel & operator= ( const RenameLabel & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_RENAME_LABEL_H__
