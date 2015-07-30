#ifndef H_Animal
#define H_Animal

#include "Entity.h"
#include "ResourceIdentifiers.h"
#include "Command.h"
#include "Projectile.h"
#include "TextNode.h"

#include <SFML/Graphics/Sprite.hpp>


class Animal : public Entity
{
	public:
		enum Type
		{
			Duck,
			//enemies
			Frog,
			TypeCount
		};


	public:
							Animal(Type type, const TextureHolder& textures, const FontHolder& fonts);

		virtual void		drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
		virtual void 			updateCurrent(sf::Time dt, CommandQueue& commands);
		virtual unsigned int	getCategory() const;

		virtual sf::FloatRect	getBoundingRect() const;
		virtual bool 			isMarkedForRemoval() const;
		bool					isAllied() const;
		float					getMaxSpeed() const;

		void					increaseFireRate();
		void					increaseSpread();
		void					collectQuack(unsigned int count);

		void 					fire();
		void					launchQuack();


	private:
		void					updateMovementPattern(sf::Time dt);
		void					checkPickupDrop(CommandQueue& commands);
		void					checkProjectileLaunch(sf::Time dt, CommandQueue& commands);

		void					createLasers(SceneNode& node, const TextureHolder& textures) const;
		void					createProjectile(SceneNode& node, Projectile::Type type, float xOffset, float yOffset, const TextureHolder& textures) const;
		void					createPickup(SceneNode& node, const TextureHolder& textures) const;

		void					updateTexts();



	private:
		Type				mType;
		sf::Sprite			mSprite;
		Command 				mFireCommand;
		Command					mQuackCommand;
		sf::Time				mFireCountdown;
		bool 					mIsFiring;
		bool					mIsLaunchingQuack;
		bool 					mIsMarkedForRemoval;

		int						mFireRateLevel;
		int						mSpreadLevel;
		int						mQuackAmmo;

		Command 				mDropPickupCommand;
		float					mTravelledDistance;
		std::size_t				mDirectionIndex;
		TextNode*				mHealthDisplay;
		TextNode*				mQuackDisplay;
};

#endif 
