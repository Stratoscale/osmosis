#ifndef __OSMOSIS_CHAIN_REMOTE_LABEL_OPS_H__
#define __OSMOSIS_CHAIN_REMOTE_LABEL_OPS_H__

#include <Osmosis/TCPSocket.h>
#include <Osmosis/Hash.h>

namespace Osmosis {
namespace Chain {
namespace Remote
{

class LabelOps
{
public:
	LabelOps( TCPSocket & socket );

	void set( const Hash & hash, const std::string & label );

	Hash get( const std::string & label );

	std::list< std::string > list( const std::string regex );

	void rename( const std::string & from, const std::string & to );

	void erase( const std::string & label );

	void sendLabelCommand( const std::string & label, Tongue::Opcode opcode );

private:
	TCPSocket &  _socket; 

	void sendRenameCommand( const std::string & from, const std::string & to );

	LabelOps( const LabelOps & rhs ) = delete;
	LabelOps & operator= ( const LabelOps & rhs ) = delete;
};

} // namespace Remote
} // namespace Chain
} // namespace Osmosis

#endif // __OSMOSIS_CHAIN_REMOTE_LABEL_OPS_H__
