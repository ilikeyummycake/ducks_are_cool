#ifndef H_CATEGORY
#define H_CATEGORY


// Entity/scene node category, used to dispatch commands
namespace Category
{
	enum Type
	{
		None				= 0,
		SceneAirLayer		= 1 << 0,
		PlayerAnimal		= 1 << 1,
		AlliedAnimal		= 1 << 2,
		EnemyAnimal			= 1 << 3,
		Pickup				= 1 << 4,
		AlliedProjectile	= 1 << 5,
		EnemyProjectile		= 1 << 6,

		Animal = PlayerAnimal | AlliedAnimal | EnemyAnimal,
		Projectile = AlliedProjectile | EnemyProjectile,
	};
}

#endif 
