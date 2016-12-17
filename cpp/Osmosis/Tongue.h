#ifndef __OSMOSIS_TONGUE_H__
#define __OSMOSIS_TONGUE_H__

namespace Osmosis {
namespace Tongue
{

enum { PROTOCOL_VERSION = 1000001 };

enum class Compression
{
	UNCOMPRESSED = 0,
	LZO = 1,
	GZIP = 2,
	XZ = 3,
};

struct Handshake
{
	unsigned protocolVersion;
	unsigned compression;
};

enum class Opcode
{
	GET = 1,
	PUT = 2,
	IS_EXISTS = 3,
	VERIFY = 4,

	SET_LABEL = 11,
	GET_LABEL = 12,
	LIST_LABELS = 13,
	ERASE_LABEL = 14,
	RENAME_LABEL = 15,
	WHO_HAS_LABEL = 16,

	ACK = 0xAC,
	MAX_OPCODE = ACK + 1
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
