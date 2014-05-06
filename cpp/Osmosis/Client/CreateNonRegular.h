#ifndef __OSMOSIS_CLIENT_CREATE_NON_REGULAR_H__
#define __OSMOSIS_CLIENT_CREATE_NON_REGULAR_H__

namespace Osmosis {
namespace Client
{

class CreateNonRegular
{
public:
	CreateNonRegular( const boost::filesystem::path & path, const FileStatus & status ):
		_path( path ),
		_status( status )
	{
		ASSERT( not status.syncContent() );
	}

	void create()
	{
		if ( _status.isSymlink() )
			boost::filesystem::create_symlink( _status.symlink(), _path );
		else if ( _status.isDirectory() ) {
			boost::filesystem::create_directory( _path );
			chmod();
		} else if ( _status.isCharacter() or
				_status.isBlock() or
				_status.isFIFO() or
				_status.isSocket() )
			mknod();
		else
			ASSERT_VERBOSE( false, "Unknown non regular file" );

		chown();
	}

private:
	const boost::filesystem::path &  _path;
	const FileStatus &               _status;

	void mknod()
	{
		ASSERT( _status.isBlock() or _status.isCharacter() );
		int result = ::mknod( _path.string().c_str(), _status.mode(), _status.dev() );
		if ( result != 0 )
			THROW_BOOST_ERRNO_EXCEPTION( errno, "Unable to mknod " << _path );
	}

	void chown()
	{
		int result = ::lchown( _path.string().c_str(), _status.uid(), _status.gid() );
		if ( result != 0 )
			THROW_BOOST_ERRNO_EXCEPTION( errno, "Unable to chown " << _path );
	}

	void chmod()
	{
		ASSERT( not _status.isSymlink() );
		int result = ::chmod( _path.string().c_str(), _status.mode() );
		if ( result != 0 )
			THROW_BOOST_ERRNO_EXCEPTION( errno, "Unable to chmod " << _path );
	}

	CreateNonRegular( const CreateNonRegular & rhs ) = delete;
	CreateNonRegular & operator= ( const CreateNonRegular & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_CREATE_NON_REGULAR_H__
