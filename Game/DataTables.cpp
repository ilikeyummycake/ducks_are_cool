#include "DataTables.h"
#include "Animal.h"
#include "Projectile.h"
#include "Pickup.h"


// For std::bind() placeholders _1, _2, ...
using namespace std::placeholders;

std::vector<AnimalData> initializeAnimalData()
{
	std::vector<AnimalData> data(Animal::TypeCount);

	data[Animal::Duck].hitpoints = 100;
	data[Animal::Duck].speed = 1.f;
	data[Animal::Duck].fireInterval = sf::seconds(1);
	data[Animal::Duck].texture = Textures::Duck;

	

	data[Animal::Frog].hitpoints = 40;
	data[Animal::Frog].speed = 0.f;
	data[Animal::Frog].texture = Textures::Frog;
	data[Animal::Frog].fireInterval = sf::seconds(2);

	return data;
}

std::vector<ProjectileData> initializeProjectileData()
{
	std::vector<ProjectileData> data(Projectile::TypeCount);

	data[Projectile::AlliedLaser].damage = 10;
	data[Projectile::AlliedLaser].speed = 300.f;
	data[Projectile::AlliedLaser].texture = Textures::LaserBeam;

	data[Projectile::EnemyLaser].damage = 10;
	data[Projectile::EnemyLaser].speed = 300.f;
	data[Projectile::EnemyLaser].texture = Textures::LaserBeam;

	data[Projectile::Quack].damage = 200;
	data[Projectile::Quack].speed = 150.f;
	data[Projectile::Quack].texture = Textures::Quack;

	return data;
}

std::vector<PickupData> initializePickupData()
{
	std::vector<PickupData> data(Pickup::TypeCount);
	
	data[Pickup::HealthRefill].texture = Textures::HealthRefill;
	data[Pickup::HealthRefill].action = std::bind(&Animal::repair, _1, 25);
	
	data[Pickup::QuackRefill].texture = Textures::QuackRefill;
	data[Pickup::QuackRefill].action = std::bind(&Animal::collectQuack, _1, 3);
	
	data[Pickup::FireSpread].texture = Textures::FireSpread;
	data[Pickup::FireSpread].action = std::bind(&Animal::increaseSpread, _1);
	
	data[Pickup::FireRate].texture = Textures::FireRate;
	data[Pickup::FireRate].action = std::bind(&Animal::increaseFireRate, _1);

	return data;
}
