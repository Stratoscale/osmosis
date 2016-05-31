#ifndef __OSMOSIS_HANDLE_SIGNAL_H__
#define __OSMOSIS_HANDLE_SIGNAL_H__

#include <unordered_map>
#include <signal.h>

namespace Osmosis
{

class HandleSignal
{
public:
	static void registerHandler( int signalNumber, std::function< void() > handler )
	{
		ASSERT( handlers().find( signalNumber ) == handlers().end() );
		sighandler_t result = signal( signalNumber, &HandleSignal::wrapper );
		if ( result == SIG_ERR )
			THROW_BOOST_ERRNO_EXCEPTION( errno, "Unable to register signal handler" );
		handlers()[ signalNumber ] = handler;
	}

private:
	static std::unordered_map< int, std::function< void() > > & handlers()
	{
		static std::unordered_map< int, std::function< void() > > handlers;
		return handlers;
	}

	static void wrapper( int signalNumber )
	{
		ASSERT( handlers().find( signalNumber ) != handlers().end() );
		handlers()[ signalNumber ]();
	}

	HandleSignal( const HandleSignal & rhs ) = delete;
	HandleSignal & operator= ( const HandleSignal & rhs ) = delete;
};

} // namespace Osmosis

#endif // __OSMOSIS_HANDLE_SIGNAL_H__
