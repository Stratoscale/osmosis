#ifndef __OSMOSIS_SOCKET_UTIL_H__
#define __OSMOSIS_SOCKET_UTIL_H__

#include <boost/asio.hpp>

namespace Osmosis {

std::ostream & operator << ( std::ostream & os, const boost::asio::ip::tcp::endpoint & endpoint )
{
	os << endpoint.address() << ':' << endpoint.port();
	return os;
}

} // namespace Osmosis

#endif // __OSMOSIS_SOCKET_UTIL_H__
