#ifndef __OSMOSIS_CLIENT_IGNORES_H__
#define __OSMOSIS_CLIENT_IGNORES_H__

namespace Osmosis {
namespace Client
{

class Ignores
{
public:
	Ignores() {}

	Ignores( const std::vector< std::string > & ignores ) :
		_ignores( ignores )
	{}

	bool ignored( const boost::filesystem::path & path ) const
	{
		std::string asString = path.string();
		for ( auto & ignore : _ignores )
			if ( asString == ignore )
				return true;
		return false;
	}

	bool parentOfAnIgnored( const boost::filesystem::path & path ) const
	{
		std::string asString = path.string() + "/";
		for ( auto & ignore : _ignores )
			if ( asString.size() < ignore.size() and memcmp( ignore.c_str(), asString.c_str(), asString.size() ) == 0 )
				return true;
		return false;
	}

	void append( const std::string & toIgnore )
	{
		_ignores.push_back( toIgnore );
	}

private:
	std::vector< std::string >  _ignores;

	Ignores( const Ignores & rhs ) = delete;
	Ignores & operator= ( const Ignores & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_IGNORES_H__
