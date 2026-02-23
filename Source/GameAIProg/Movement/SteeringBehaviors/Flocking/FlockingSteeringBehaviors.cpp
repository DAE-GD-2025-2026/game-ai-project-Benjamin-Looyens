#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering{};

	const FVector2D averagePos = pFlock->GetAverageNeighborPos();
	if (averagePos.SquaredLength() > 0.0) steering.LinearVelocity = averagePos - Agent.GetPosition();

	return steering;
}

void Cohesion::DebugRender(ASteeringAgent& Agent)
{
	ISteeringBehavior::DebugRender(Agent);

	constexpr FColor AVERAGE_POS_COLOR{ 100, 100, 200 };

	const FVector2D averagePos = pFlock->GetAverageNeighborPos();
	const double agentZ = Agent.GetActorLocation().Z - 85.0;

	UWorld* pWorld = Agent.GetWorld();

	DrawDebugPoint(pWorld, FVector{ averagePos, agentZ }, 10.0f, AVERAGE_POS_COLOR);
}

//*********************
//SEPARATION (FLOCKING)

//*************************
//VELOCITY MATCH (FLOCKING)
