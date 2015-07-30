#ifndef BOOK_PICKUP_HPP
#define BOOK_PICKUP_HPP

#include "Entity.h"
#include "Command.h"
#include "resourceIdentifiers.h"

#include <SFML/Graphics/Sprite.hpp>


class Animal;

class Pickup : public Entity
{
	public:
		enum Type
		{
			HealthRefill,
			QuackRefill,
			FireSpread,
			FireRate,
			TypeCount
		};


	public:
								Pickup(Type type, const TextureHolder& textures);

		virtual unsigned int	getCategory() const;
		virtual sf::FloatRect	getBoundingRect() const;

		void 					apply(Animal& player) const;


	protected:
		virtual void			drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;


	private:
		Type 					mType;
		sf::Sprite				mSprite;
};

#endif 