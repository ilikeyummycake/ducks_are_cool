#include "World.h"
#include "Projectile.h"
#include "Pickup.h"
#include "Foreach.h"
#include "TextNode.h"

#include <SFML/Graphics/RenderWindow.hpp>

#include <algorithm>
#include <cmath>
#include <limits>


World::World(sf::RenderWindow& window, FontHolder& fonts)
: mWindow(window)
, mFonts(fonts)
, mWorldView(window.getDefaultView())
, mTextures() 
, mSceneGraph()
, mSceneLayers()
, mWorldBounds(0.f, 0.f, mWorldView.getSize().x, 3000.f)
, mSpawnPosition(mWorldView.getSize().x / 2.f, mWorldBounds.height - mWorldView.getSize().y / 2.f)
, mScrollSpeed(-30.f)
, mPlayerAnimal(nullptr)
, mEnemySpawnPoints()
, mActiveEnemies()
{
	loadTextures();
	buildScene();

	// Prepare the view
	mWorldView.setCenter(mSpawnPosition);
}

void World::update(sf::Time dt)
{
	// Scroll the world, reset player velocity
	//if player isn't moved, automatically moves down with background
	mWorldView.move(0.f, mScrollSpeed * dt.asSeconds());	
	mPlayerAnimal->setVelocity(0.f, 30.f);

	// Setup commands to destroy entities, and guide Quack
	destroyEntitiesOutsideView();
	guideQuack();

	// Forward commands to scene graph, adapt velocity (scrolling, diagonal correction)
	while (!mCommandQueue.isEmpty())
		mSceneGraph.onCommand(mCommandQueue.pop(), dt);
	adaptPlayerVelocity();

	
	// Collision detection and response (may destroy entities)
	handleCollisions();

		// Remove all destroyed entities, create new ones
	mSceneGraph.removeWrecks();
	spawnEnemies();

	// Regular update step, adapt position (correct if outside view)
	mSceneGraph.update(dt, mCommandQueue);
	adaptPlayerPosition();
}

void World::draw()
{
	mWindow.setView(mWorldView);
	mWindow.draw(mSceneGraph);
}

void World::loadTextures()
{
	mTextures.load(Textures::Duck, "Duck.png");
	mTextures.load(Textures::Water, "water.png");
	mTextures.load(Textures::Frog, "frog.png");

	mTextures.load(Textures::LaserBeam, "laser.png");
	mTextures.load(Textures::Quack, "quack.png");

	mTextures.load(Textures::HealthRefill, "HealthRefill.png");
	mTextures.load(Textures::QuackRefill, "QuackRefill.png");
	mTextures.load(Textures::FireSpread, "FireSpread.png");
	mTextures.load(Textures::FireRate, "FireRate.png"); 
	
}

bool World::hasAlivePlayer() const
{
	return !mPlayerAnimal->isMarkedForRemoval();
}

bool World::hasPlayerReachedEnd() const
{
	return !mWorldBounds.contains(mPlayerAnimal->getPosition());
}


bool matchesCategories(SceneNode::Pair& colliders, Category::Type type1, Category::Type type2)
{
	unsigned int category1 = colliders.first->getCategory();
	unsigned int category2 = colliders.second->getCategory();

	// Make sure first pair entry has category type1 and second has type2
	if (type1 & category1 && type2 & category2)
	{
		return true;
	}
	else if (type1 & category2 && type2 & category1)
	{
		std::swap(colliders.first, colliders.second);
		return true;
	}
	else
	{
		return false;
	}
}

void World::handleCollisions()
{
	std::set<SceneNode::Pair> collisionPairs;
	mSceneGraph.checkSceneCollision(mSceneGraph, collisionPairs);

	FOREACH(SceneNode::Pair pair, collisionPairs)
	{
		if (matchesCategories(pair, Category::PlayerAnimal, Category::EnemyAnimal))
		{
			auto& player = static_cast<Animal&>(*pair.first);
			auto& enemy = static_cast<Animal&>(*pair.second);

			// Collision: Player damage  = 1 
			//enemy doesn't die from collision with player
			player.damage(1);
		}

		else if (matchesCategories(pair, Category::PlayerAnimal, Category::Pickup))
		{
			auto& player = static_cast<Animal&>(*pair.first);
			auto& pickup = static_cast<Pickup&>(*pair.second);

			// Apply pickup effect to player, destroy projectile
			pickup.apply(player);
			pickup.destroy();
		}

		else if (matchesCategories(pair, Category::EnemyAnimal, Category::AlliedProjectile)
			  || matchesCategories(pair, Category::PlayerAnimal, Category::EnemyProjectile))
		{
			auto& animal = static_cast<Animal&>(*pair.first);
			auto& projectile = static_cast<Projectile&>(*pair.second);

			// Apply projectile damage to Animal, destroy projectile
			animal.damage(projectile.getDamage());
			projectile.destroy();
		}
	}
}

void World::buildScene()
{
	// Initialize the different layers
	for (std::size_t i = 0; i < LayerCount; ++i)
	{
		SceneNode::Ptr layer(new SceneNode());
		mSceneLayers[i] = layer.get();

		mSceneGraph.attachChild(std::move(layer));
	}

	// Prepare the tiled background
	sf::Texture& texture = mTextures.get(Textures::Water);
	sf::IntRect textureRect(mWorldBounds);
	texture.setRepeated(true);

	// Add the background sprite to the scene
	std::unique_ptr<SpriteNode> backgroundSprite(new SpriteNode(texture, textureRect));
	backgroundSprite->setPosition(mWorldBounds.left, mWorldBounds.top);
	mSceneLayers[Background]->attachChild(std::move(backgroundSprite));

	// Add player's duck
	std::unique_ptr<Animal> leader(new Animal(Animal::Duck, mTextures, mFonts));
	mPlayerAnimal = leader.get();
	mPlayerAnimal->setPosition(mSpawnPosition);
	mPlayerAnimal->setVelocity(30.f, mScrollSpeed);
	mSceneLayers[Air]->attachChild(std::move(leader));

	//add enemys
	addEnemies();

}

void World::adaptPlayerPosition()
{
	// Keep player's position inside the screen bounds, at least borderDistance units from the border
	sf::FloatRect viewBounds(mWorldView.getCenter() - mWorldView.getSize() / 2.f, mWorldView.getSize());
	const float borderDistance = 40.f;

	sf::Vector2f position = mPlayerAnimal->getPosition();
	position.x = std::max(position.x, viewBounds.left + borderDistance);
	position.x = std::min(position.x, viewBounds.left + viewBounds.width - borderDistance);
	position.y = std::max(position.y, viewBounds.top + borderDistance);
	position.y = std::min(position.y, viewBounds.top + viewBounds.height - borderDistance);
	mPlayerAnimal->setPosition(position);
}

void World::adaptPlayerVelocity()
{
	sf::Vector2f velocity = mPlayerAnimal->getVelocity();

	// If moving diagonally, reduce velocity (to have always same velocity)
	if (velocity.x != 0.f && velocity.y != 0.f)
		mPlayerAnimal->setVelocity(velocity / std::sqrt(2.f));

	// Add scrolling velocity
	mPlayerAnimal->accelerate(0.f, mScrollSpeed);
}
CommandQueue& World::getCommandQueue()
{
	return mCommandQueue;
}

void World::addEnemies()
{
	// Add enemies to the spawn point container
	addEnemy(Animal::Frog,    -300.f,  100.f);
	addEnemy(Animal::Frog,  -100.f, 100.f);
	addEnemy(Animal::Frog,     -100.f, 200.f);
	addEnemy(Animal::Frog,  -300.f, 300.f);
	addEnemy(Animal::Frog,  +100.f, 500.f);
	addEnemy(Animal::Frog,  -100.f, 800.f);
	addEnemy(Animal::Frog, +70.f, 1300.f);
	addEnemy(Animal::Frog, +270.f, 2100.f);
	

	// Sort all enemies according to their y value, such that lower enemies are checked first for spawning
	std::sort(mEnemySpawnPoints.begin(), mEnemySpawnPoints.end(), [] (SpawnPoint lhs, SpawnPoint rhs)
	{
		return lhs.y < rhs.y;
	});
}

void World::addEnemy(Animal::Type type, float relX, float relY)
{
	SpawnPoint spawn(type, mSpawnPosition.x + relX, mSpawnPosition.y - relY);
	mEnemySpawnPoints.push_back(spawn);
}

void World::spawnEnemies()
{
	// Spawn all enemies entering the view area (including distance) this frame
	while (!mEnemySpawnPoints.empty()
		&& mEnemySpawnPoints.back().y > getBattlefieldBounds().top)
	{
		SpawnPoint spawn = mEnemySpawnPoints.back();
		
		std::unique_ptr<Animal> enemy(new Animal(spawn.type, mTextures, mFonts));
		enemy->setPosition(spawn.x, spawn.y);
		enemy->setRotation(180.f);

		mSceneLayers[Air]->attachChild(std::move(enemy));

		// Enemy is spawned, remove from the list to spawn
		mEnemySpawnPoints.pop_back();
	}
}

void World::destroyEntitiesOutsideView()
{
	Command command;
	command.category = Category::Projectile | Category::EnemyAnimal;
	command.action = derivedAction<Entity>([this] (Entity& e, sf::Time)
	{
		if (!getBattlefieldBounds().intersects(e.getBoundingRect()))
			e.destroy();
	});

	mCommandQueue.push(command);
}

void World::guideQuack()
{
	// Setup command that stores all enemies in mActiveEnemies
	Command enemyCollector;
	enemyCollector.category = Category::EnemyAnimal;
	enemyCollector.action = derivedAction<Animal>([this] (Animal& enemy, sf::Time)
	{
		if (!enemy.isDestroyed())
			mActiveEnemies.push_back(&enemy);
	});

	// Setup command that guides all Quacks to the enemy which is currently closest to the player
	Command QuackGuider;
	QuackGuider.category = Category::AlliedProjectile;
	QuackGuider.action = derivedAction<Projectile>([this] (Projectile& Quack, sf::Time)
	{
		// Ignore unguided 
		if (!Quack.isGuided())
			return;

		float minDistance = std::numeric_limits<float>::max();
		Animal* closestEnemy = nullptr;

		// Find closest enemy
		FOREACH(Animal* enemy, mActiveEnemies)
		{
			float enemyDistance = distance(Quack, *enemy);

			if (enemyDistance < minDistance)
			{
				closestEnemy = enemy;
				minDistance = enemyDistance;
			}
		}

		if (closestEnemy)
			Quack.guideTowards(closestEnemy->getWorldPosition());
	});

	// Push commands, reset active enemies
	mCommandQueue.push(enemyCollector);
	mCommandQueue.push(QuackGuider);
	mActiveEnemies.clear();
}

sf::FloatRect World::getViewBounds() const
{
	return sf::FloatRect(mWorldView.getCenter() - mWorldView.getSize() / 2.f, mWorldView.getSize());
}

sf::FloatRect World::getBattlefieldBounds() const
{
	// Return view bounds + some area at top, where enemies spawn
	sf::FloatRect bounds = getViewBounds();
	bounds.top -= 100.f;
	bounds.height += 100.f;

	return bounds;
}

