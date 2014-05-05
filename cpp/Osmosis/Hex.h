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

static inline void toBuffer(    const std::string &  input,
				unsigned char *      buffer,
				unsigned             bufferLength )
{
	if ( input.size() % 2 != 0 )
		THROW( Error, "String '" << input << "' is not of even length, so it can't be a hex string" );
	if ( bufferLength < input.size() / 2 )
		THROW( Error, "String '" << input << "' is too long to dehexlify" );
	bufferLength = input.size() / 2;
	for ( unsigned i = 0; i < bufferLength; ++ i )
		buffer[ i ] = ( singleCharacter( input[ i * 2 ] ) << 4 ) + singleCharacter( input[ i * 2 + 1 ] );
}

} // namespace Hex
} // namespace Osmosis

#endif // __OSMOSIS_HEX_H__
