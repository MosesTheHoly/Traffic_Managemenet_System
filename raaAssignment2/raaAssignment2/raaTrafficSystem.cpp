#include "raaCollisionTarget.h"
#include "raaTrafficSystem.h"

raaCollisionTargets raaTrafficSystem::sm_lColliders;
raaTrafficLights raaTrafficSystem::sm_lTrafficLights;

void raaTrafficSystem::start()
{
	sm_lColliders.clear();
	sm_lTrafficLights.clear();
}

void raaTrafficSystem::end()
{
	sm_lColliders.clear();
	sm_lTrafficLights.clear();
}

#pragma region Colliders
void raaTrafficSystem::addTarget(raaCollisionTarget* pTarget)
{
	if (pTarget && std::find(sm_lColliders.begin(), sm_lColliders.end(), pTarget) == sm_lColliders.end()) 
		sm_lColliders.push_back(pTarget);
}

void raaTrafficSystem::removeTarget(raaCollisionTarget* pTarget)
{
	if (pTarget && std::find(sm_lColliders.begin(), sm_lColliders.end(), pTarget) != sm_lColliders.end())
		sm_lColliders.remove(pTarget);
}

const raaCollisionTargets& raaTrafficSystem::colliders()
{
	return sm_lColliders;
}
#pragma endregion Colliders

#pragma region Traffic_Lights
void raaTrafficSystem::addTrafficLight(TrafficLightFacarde* pTarget)
{
	if (pTarget && std::find(sm_lTrafficLights.begin(), sm_lTrafficLights.end(), pTarget) == sm_lTrafficLights.end())
		sm_lTrafficLights.push_back(pTarget);
}

void raaTrafficSystem::removeTrafficLight(TrafficLightFacarde* pTarget)
{
	if (pTarget && std::find(sm_lTrafficLights.begin(), sm_lTrafficLights.end(), pTarget) != sm_lTrafficLights.end())
		sm_lTrafficLights.remove(pTarget);
}

const raaTrafficLights& raaTrafficSystem::trafficLights()
{
	return sm_lTrafficLights;
}
#pragma endregion Traffic_Lights



