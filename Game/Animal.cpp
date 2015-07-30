#include "Animal.h"
#include "ResourceHolder.h"
#include "DataTables.h"
#include "Utility.h"
#include "Pickup.h"
#include "CommandQueue.h"
#include "SpriteNode.h"


#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include <cmath>

namespace
{
	const std::vector<AnimalData> Table = initializeAnimalData();
}




Animal::Animal(Type type, const TextureHolder& textures, const FontHolder& fonts)
: Entity(Table[type].hitpoints)
, mType(type)
, mFireCommand()
, mQuackCommand()
, mFireCountdown(sf::Time::Zero)
, mIsFiring(false)
, mIsLaunchingQuack(false)
, mIsMarkedForRemoval(false)
, mSprite(textures.get(Table[type].texture))
, mFireRateLevel(1)
, mSpreadLevel(1)
, mQuackAmmo(2)
, mDropPickupCommand()
, mTravelledDistance(0.f)
, mDirectionIndex(0)
, mQuackDisplay(nullptr)
{
	centerOrigin(mSprite);

	mFireCommand.category = Category::SceneAirLayer;
	mFireCommand.action   = [this, &textures] (SceneNode& node, sf::Time)
	{
		createLasers(node, textures);
	};

	mQuackCommand.category = Category::SceneAirLayer;
	mQuackCommand.action   = [this, &textures] (SceneNode& node, sf::Time)
	{
		createProjectile(node, Projectile::Quack, 0.f, 0.5f, textures);
	};

	mDropPickupCommand.category = Category::SceneAirLayer;
	mDropPickupCommand.action   = [this, &textures] (SceneNode& node, sf::Time)
	{
		createPickup(node, textures);
	};

	std::unique_ptr<TextNode> healthDisplay(new TextNode(fonts, ""));
	mHealthDisplay = healthDisplay.get();
	attachChild(std::move(healthDisplay));

	if (getCategory() == Category::PlayerAnimal)
	{
		std::unique_ptr<TextNode> QuackDisplay(new TextNode(fonts, ""));
		QuackDisplay->setPosition(0, 30);
		mQuackDisplay = QuackDisplay.get();
		attachChild(std::move(QuackDisplay));
	}

	updateTexts();
}


void Animal::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(mSprite, states);
}

void Animal::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	// Entity has been destroyed: Possibly drop pickup, mark for removal
	if (isDestroyed())
	{
		checkPickupDrop(commands);

		mIsMarkedForRemoval = true;
		return;
	}

	// Check if Lasers or Quack are fired
	checkProjectileLaunch(dt, commands);

	// Update enemy movement pattern; apply velocity
	updateMovementPattern(dt);
	Entity::updateCurrent(dt, commands);

	// Update texts
	updateTexts();
}

unsigned int Animal::getCategory() const
{//this may be bad
	if(isAllied())
		return Category::PlayerAnimal;
	else
		return Category::EnemyAnimal;
	
}

sf::FloatRect Animal::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

bool Animal::isMarkedForRemoval() const
{
	return mIsMarkedForRemoval;
}

bool Animal::isAllied() const
{
	return mType == Duck;
}

float Animal::getMaxSpeed() const
{
	return Table[mType].speed;
}

void Animal::increaseFireRate()
{
	if (mFireRateLevel < 10)
		++mFireRateLevel;
}

void Animal::increaseSpread()
{
	if (mSpreadLevel < 3)
		++mSpreadLevel;
}

void Animal::collectQuack(unsigned int count)
{
	mQuackAmmo += count;
}

void Animal::fire()
{
	// Only animals with fire interval != 0 are able to fire
	if (Table[mType].fireInterval != sf::Time::Zero)
		mIsFiring = true;
}

void Animal::launchQuack()
{
	if (mQuackAmmo > 0)
	{
		mIsLaunchingQuack = true;
		--mQuackAmmo;
	}
}

void Animal::updateMovementPattern(sf::Time dt)
{
	
}

void Animal::checkPickupDrop(CommandQueue& commands)
{
	if (!isAllied() && randomInt(3) == 0)
		commands.push(mDropPickupCommand);
}

void Animal::checkProjectileLaunch(sf::Time dt, CommandQueue& commands)
{
	// Enemies try to fire all the time
	if (!isAllied())
		fire();

	// Check for automatic gunfire, allow only in intervals
	if (mIsFiring && mFireCountdown <= sf::Time::Zero)
	{
		// Interval expired: We can fire a new Laser
		commands.push(mFireCommand);
		mFireCountdown += Table[mType].fireInterval / (mFireRateLevel + 1.f);
		mIsFiring = false;
	}
	else if (mFireCountdown > sf::Time::Zero)
	{
		// Interval not expired: Decrease it further
		mFireCountdown -= dt;
		mIsFiring = false;
	}

	// Check for Quack launch
	if (mIsLaunchingQuack)
	{
		commands.push(mQuackCommand);
		mIsLaunchingQuack = false;
	}
}

void Animal::createLasers(SceneNode& node, const TextureHolder& textures) const
{
	Projectile::Type type = isAllied() ? Projectile::AlliedLaser : Projectile::EnemyLaser;

	switch (mSpreadLevel)
	{
		case 1:
			createProjectile(node, type, 0.0f, 0.5f, textures);
			break;

		case 2:
			createProjectile(node, type, -0.33f, 0.33f, textures);
			createProjectile(node, type, +0.33f, 0.33f, textures);
			break;

		case 3:
			createProjectile(node, type, -0.5f, 0.33f, textures);
			createProjectile(node, type,  0.0f, 0.5f, textures);
			createProjectile(node, type, +0.5f, 0.33f, textures);
			break;
	}
}

void Animal::createProjectile(SceneNode& node, Projectile::Type type, float xOffset, float yOffset, const TextureHolder& textures) const
{
	std::unique_ptr<Projectile> projectile(new Projectile(type, textures));

	sf::Vector2f offset(xOffset * mSprite.getGlobalBounds().width, yOffset * mSprite.getGlobalBounds().height);
	sf::Vector2f velocity(0, projectile->getMaxSpeed());

	float sign = isAllied() ? -1.f : +1.f;
	projectile->setPosition(getWorldPosition() + offset * sign);
	projectile->setVelocity(velocity * sign);
	node.attachChild(std::move(projectile));
}

void Animal::createPickup(SceneNode& node, const TextureHolder& textures) const
{
	auto type = static_cast<Pickup::Type>(randomInt(Pickup::TypeCount));

	std::unique_ptr<Pickup> pickup(new Pickup(type, textures));
	pickup->setPosition(getWorldPosition());
	pickup->setVelocity(0.f, 1.f);
	node.attachChild(std::move(pickup));
}

void Animal::updateTexts()
{
	mHealthDisplay->setString(toString(getHitpoints()) + " HP");
	mHealthDisplay->setRotation(-getRotation());
	mHealthDisplay->setPosition(0.f, 50.f);

	if (mQuackDisplay)
	{
		if (mQuackAmmo == 0)
			mQuackDisplay->setString("");
		else
			mQuackDisplay->setString("QUACKS: " + toString(mQuackAmmo));
	}
}
