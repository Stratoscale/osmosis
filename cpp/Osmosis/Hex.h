#ifndef __OSMOSIS_HEX_H__
#define __OSMOSIS_HEX_H__

namespace Osmosis {
namespace Hex
{

static inline void fromBuffer(  const unsigned char *  data,
				unsigned               length,
				char *                 buffer,
				unsigned               bufferLength )
{
	ASSERT( length < bufferLength * 2 );
	for ( unsigned i = 0; i < length; ++ i )
		snprintf( buffer + i * 2, bufferLength - i * 2, "%02x", data[ i ] );
}

static inline unsigned char singleCharacter( char character )
{
	if ( character >= '0' and character <= '9' )
		return character - '0';
	if ( character >= 'A' and character <= 'F' )
		return character - 'A' + 10;
	if ( character >= 'a' and character <= 'f' )
		return character - 'a' + 10;
	THROW( Error, "Character " << static_cast< unsigned >( character ) << " is not a hex character" );
}

static inline void toBuffer(    const char *         input,
		                unsigned             inputLength,
				unsigned char *      buffer,
				unsigned             bufferLength )
{
	if ( inputLength % 2 != 0 )
		THROW( Error, "String '" << std::string( input, inputLength ) << "' is not of even length, so it can't be a hex string" );
	if ( bufferLength < inputLength / 2 )
		THROW( Error, "String '" << std::string( input, inputLength ) << "' is too long to dehexlify" );
	bufferLength = inputLength / 2;
	while ( bufferLength > 0 ) {
		* buffer = ( singleCharacter( input[0] ) << 4 ) | singleCharacter( input[ 1 ] );
		++ buffer;
		input += 2;
		-- bufferLength;
	}
}

static inline void toBuffer(    const std::string &  input,
				unsigned char *      buffer,
				unsigned             bufferLength )
{
	toBuffer( input.c_str(), input.size(), buffer, bufferLength );
}

} // namespace Hex
} // namespace Osmosis

#endif // __OSMOSIS_HEX_H__
