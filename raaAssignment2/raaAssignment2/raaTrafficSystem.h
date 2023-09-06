#pragma once

#include <list>

typedef std::list<class raaCollisionTarget*> raaCollisionTargets;
typedef std::list<class TrafficLightFacarde*> raaTrafficLights;


class raaTrafficSystem
{
public:
	static void start();
	static void end();

	static void addTarget(raaCollisionTarget* pTarget);
	static void removeTarget(raaCollisionTarget* pTarget);
	static const raaCollisionTargets& colliders();

	static void addTrafficLight(TrafficLightFacarde* pTarget);
	static void removeTrafficLight(TrafficLightFacarde* pTarget);
	static const raaTrafficLights& trafficLights();
protected:
	static raaCollisionTargets sm_lColliders;
	static raaTrafficLights sm_lTrafficLights;
};

