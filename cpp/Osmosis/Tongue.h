#ifndef __OSMOSIS_TONGUE_H__
#define __OSMOSIS_TONGUE_H__

namespace Osmosis {
namespace Tongue
{

enum class Opcode
{
	GET = 1,
	PUT = 2,
	IS_EXISTS = 3,
	SET_LABEL = 4,
	GET_LABEL = 5,
	ACK = 0xAC,
};

enum class HashAlgorithm 
{
	MD5 = 1,
	SHA1 = 2,
};

enum class IsExists
{
	YES = 1,
	NO = 2,
};

struct Header
{
	unsigned char opcode;
} __attribute__((packed));

struct Hash
{
	unsigned char hashAlgorithm;
	unsigned char hash[ 20 ];
} __attribute__((packed));

struct Label
{
	unsigned short length;
	char label[ 0 ];
} __attribute__((packed));

struct Chunk
{
	size_t offset;
	unsigned short bytes; // zero is EOF, in which case offset not used matter
	unsigned char payload[ 0 ];
} __attribute__((packed));

struct IsExistsResponse 
{
	unsigned char response;
} __attribute__((packed));

} // namespace Tongue
} // namespace Osmosis

#endif // __OSMOSIS_TONGUE_H__
