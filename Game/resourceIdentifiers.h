#ifndef H_RESOURCEIDENTIFIERS
#define H_RESOURCEIDENTIFIERS


// Forward declaration of SFML classes
namespace sf
{
	class Texture;
	class Font;
}

namespace Textures
{
	enum ID
	{
		Duck,
		Duckling,
		Frog,
		LaserBeam,
		Quack, 
		HealthRefill,
		QuackRefill,
		FireSpread,
		FireRate,
		Water,
		TitleScreen
		
	};
}
namespace Fonts
{
	enum ID
	{
		Main,
	};
}

// Forward declaration and a few type definitions
template <typename Resource, typename Identifier>
class ResourceHolder;

typedef ResourceHolder<sf::Texture, Textures::ID> TextureHolder;
typedef ResourceHolder<sf::Font, Fonts::ID>			FontHolder;

#endif
