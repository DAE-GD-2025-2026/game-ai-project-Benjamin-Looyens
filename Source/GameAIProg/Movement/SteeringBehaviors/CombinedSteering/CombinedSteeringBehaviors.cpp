
#include "CombinedSteeringBehaviors.h"
#include <algorithm>
#include "../SteeringAgent.h"
#include <cassert>
#include <numeric>

BlendedSteering::BlendedSteering(const std::vector<WeightedBehavior>& WeightedBehaviors)
	:WeightedBehaviors(WeightedBehaviors)
{};

//****************
//BLENDED STEERING
SteeringOutput BlendedSteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering{};

	const float totalWeight = std::accumulate(begin(WeightedBehaviors), end(WeightedBehaviors), 0.0f, 
							[](float prev, const WeightedBehavior& behavior) { return prev + behavior.Weight; });
	if (totalWeight <= 0.0f) return steering;

	for (const auto& behavior : WeightedBehaviors) {
		const SteeringOutput weighted = (behavior.Weight / totalWeight) * behavior.pBehavior->CalculateSteering(DeltaT, Agent);
		//SteeringOutput weighted = behavior.Weight * behavior.pBehavior->CalculateSteering(DeltaT, Agent);

		steering.LinearVelocity += weighted.LinearVelocity;
		steering.AngularVelocity += weighted.AngularVelocity;
		steering.IsValid = steering.IsValid && weighted.IsValid;
	}

	return steering;
}

void BlendedSteering::DebugRender(ASteeringAgent& Agent)
{
	for (const auto& behavior : WeightedBehaviors) {
		behavior.pBehavior->DebugRender(Agent);
	}
}

void BlendedSteering::SetTargetAllBlends(const FTargetData& target)
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

void PrioritySteering::SetTargetAllPriorities(const FTargetData& target)
{
	SetTarget(target);

	for (const auto& pBehavior : m_PriorityBehaviors) {
		pBehavior->SetTarget(target);
	}
}
