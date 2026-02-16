
#include "CombinedSteeringBehaviors.h"
#include <algorithm>
#include "../SteeringAgent.h"
#include <cassert>

BlendedSteering::BlendedSteering(const std::vector<WeightedBehavior>& WeightedBehaviors)
	:WeightedBehaviors(WeightedBehaviors)
{};

//****************
//BLENDED STEERING
SteeringOutput BlendedSteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput BlendedSteering = {};
	// TODO: Calculate the weighted average steeringbehavior

	if (WeightedBehaviors.size() > 1) {
		return WeightedBehaviors[1].pBehavior->CalculateSteering(DeltaT, Agent);
	}

	return BlendedSteering;
}

void BlendedSteering::DebugRender(ASteeringAgent& Agent)
{
	// TODO: All Behaviors Debug Render
	if (WeightedBehaviors.size() > 1) {
		WeightedBehaviors[1].pBehavior->DebugRender(Agent);
	}
}

void BlendedSteering::SetTargetAllBlends(FTargetData& target)
{
	SetTarget(target);

	for (auto& behavior : WeightedBehaviors) {
		behavior.pBehavior->SetTarget(target);
	}
}

float* BlendedSteering::GetWeight(ISteeringBehavior* const SteeringBehavior)
{
	auto it = find_if(WeightedBehaviors.begin(),
		WeightedBehaviors.end(),
		[SteeringBehavior](const WeightedBehavior& Elem)
		{
			return Elem.pBehavior == SteeringBehavior;
		}
	);

	if(it!= WeightedBehaviors.end())
		return &it->Weight;
	
	return nullptr;
}

//*****************
//PRIORITY STEERING
SteeringOutput PrioritySteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering = {};

	for (ISteeringBehavior* const pBehavior : m_PriorityBehaviors)
	{
		Steering = pBehavior->CalculateSteering(DeltaT, Agent);

		if (Steering.IsValid)
			break;
	}

	//If non of the behavior return a valid output, last behavior is returned
	return Steering;
}

void PrioritySteering::DebugRender(ASteeringAgent& Agent)
{
}
