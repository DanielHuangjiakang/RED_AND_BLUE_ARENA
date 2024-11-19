
#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// Callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	// TODO: A1 add a LightUp component
	ComponentContainer<DeathTimer> deathTimers;
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Player> players;
	ComponentContainer<Mesh*> meshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec3> colors;
	ComponentContainer<Block> blocks;
	ComponentContainer<Gravity> gravities;
	ComponentContainer<Bullet> bullets;
	ComponentContainer<Grenade> grenades;
	ComponentContainer<GunTimer> gunTimers;
	ComponentContainer<Item> items;

	ComponentContainer<AnimationFrame> animations;
	ComponentContainer<Text> texts;
	ComponentContainer<Background> backgrounds;


	ComponentContainer<Portal> portals;
  	ComponentContainer<Laser> lasers;
	ComponentContainer<Lifetime> lifetimes;


	// constructor that adds all containers for looping over them
	// IMPORTANT: Don't forget to add any newly added containers!
	ECSRegistry()
	{
		registry_list.push_back(&deathTimers);
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);
		registry_list.push_back(&players);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&blocks);
		registry_list.push_back(&gravities);
		registry_list.push_back(&bullets);
		registry_list.push_back(&gunTimers);
		registry_list.push_back(&items);
		registry_list.push_back(&grenades);

		registry_list.push_back(&animations);
		registry_list.push_back(&texts);
		registry_list.push_back(&backgrounds);

		registry_list.push_back(&portals);
    	registry_list.push_back(&lasers);
		registry_list.push_back(&lifetimes);

	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;