#include "Player.h"
#include "CommandQueue.h"
#include "Animal.h"
#include "Foreach.h"

#include <map>
#include <string>
#include <algorithm>

using namespace std::placeholders;

struct AnimalMover
{
	AnimalMover(float vx, float vy)
	: velocity(vx, vy)
	{
	}

	void operator() (Animal& Animal, sf::Time) const
	{
		Animal.accelerate(velocity * Animal.getMaxSpeed());
	}

	sf::Vector2f velocity;
};

Player::Player()
{
	// Set initial key bindings
	mKeyBinding[sf::Keyboard::Left] = MoveLeft;
	mKeyBinding[sf::Keyboard::Right] = MoveRight;
	mKeyBinding[sf::Keyboard::Up] = MoveUp;
	mKeyBinding[sf::Keyboard::Down] = MoveDown;
	mKeyBinding[sf::Keyboard::Q] = Fire;
	mKeyBinding[sf::Keyboard::W] = LaunchQuack;

	// Set initial action bindings
	initializeActions();	

	// Assign all categories to player's Animal
	FOREACH(auto& pair, mActionBinding)
		pair.second.category = Category::PlayerAnimal;
}

void Player::handleEvent(const sf::Event& event, CommandQueue& commands)
{
	if (event.type == sf::Event::KeyPressed)
	{
		// Check if pressed key appears in key binding, trigger command if so
		auto found = mKeyBinding.find(event.key.code);
		if (found != mKeyBinding.end() && !isRealtimeAction(found->second))
			commands.push(mActionBinding[found->second]);
	}
}

void Player::handleRealtimeInput(CommandQueue& commands)
{
	// Traverse all assigned keys and check if they are pressed
	FOREACH(auto pair, mKeyBinding)
	{
		// If key is pressed, lookup action and trigger corresponding command
		if (sf::Keyboard::isKeyPressed(pair.first) && isRealtimeAction(pair.second))
			commands.push(mActionBinding[pair.second]);
	}
}

void Player::assignKey(Action action, sf::Keyboard::Key key)
{
	// Remove all keys that already map to action
	for (auto itr = mKeyBinding.begin(); itr != mKeyBinding.end(); )
	{
		if (itr->second == action)
			mKeyBinding.erase(itr++);
		else
			++itr;
	}

	// Insert new binding
	mKeyBinding[key] = action;
}

sf::Keyboard::Key Player::getAssignedKey(Action action) const
{
	FOREACH(auto pair, mKeyBinding)
	{
		if (pair.second == action)
			return pair.first;
	}

	return sf::Keyboard::Unknown;
}

void Player::initializeActions()
{
	const float playerSpeed = 200.f;

	mActionBinding[MoveLeft].action	 = derivedAction<Animal>(AnimalMover(-playerSpeed, 0.f));
	mActionBinding[MoveRight].action = derivedAction<Animal>(AnimalMover(+playerSpeed, 0.f));
	mActionBinding[MoveUp].action    = derivedAction<Animal>(AnimalMover(0.f, -playerSpeed));
	mActionBinding[MoveDown].action  = derivedAction<Animal>(AnimalMover(0.f, +playerSpeed));
	mActionBinding[Fire].action      = derivedAction<Animal>(std::bind(&Animal::fire, _1));
	mActionBinding[LaunchQuack].action = derivedAction<Animal>(std::bind(&Animal::launchQuack, _1));
}

void Player::setMissionStatus(MissionStatus status)
{
	mCurrentMissionStatus = status;
}

Player::MissionStatus Player::getMissionStatus() const
{
	return mCurrentMissionStatus;
}

bool Player::isRealtimeAction(Action action)
{
	switch (action)
	{
		case MoveLeft:
		case MoveRight:
		case MoveDown:
		case MoveUp:
		case Fire:
			return true;

		default:
			return false;
	}
}
