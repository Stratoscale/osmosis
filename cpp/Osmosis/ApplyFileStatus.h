#ifndef __OSMOSIS_APPLY_FILE_STATUS_H__
#define __OSMOSIS_APPLY_FILE_STATUS_H__

namespace Osmosis
{

class ApplyFileStatus
{
public:
	ApplyFileStatus( const boost::filesystem::path & path, const FileStatus & status ):
		_path( path ),
		_status( status )
	{}

	void applyExistingRegular()
	{
		chmod();
		chown();
	}

	void createNonRegular()
	{
		ASSERT( not _status.syncContent() );
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

	ApplyFileStatus( const ApplyFileStatus & rhs ) = delete;
	ApplyFileStatus & operator= ( const ApplyFileStatus & rhs ) = delete;
};

} // namespace Osmosis

#endif // __OSMOSIS_APPLY_FILE_STATUS_H__
