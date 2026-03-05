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
SteeringOutput Separation::CalculateSteering(float deltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering{};

	const FVector2D& ownPos = Agent.GetPosition();

	const auto& pNeighbors = pFlock->GetNeighbors();
	const int numNeighbors = pFlock->GetNrOfNeighbors();
	for (int32 index{}; index < numNeighbors; index++) {
		const FVector2D& neighborPos = pNeighbors[index]->GetPosition();
	
		//const FVector2D toNeighbor = neighborPos - ownPos;
		const FVector2D fromNeighbor = ownPos - neighborPos;
		const float sqrDist = fromNeighbor.SquaredLength();
	
		steering.LinearVelocity += fromNeighbor / sqrDist;
	}

	steering.LinearVelocity.Normalize();
	return steering;
}
void Separation::DebugRender(ASteeringAgent& Agent)
{
	// TODO : Separation Debug Render
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering{};

	const FVector2D averageVel = pFlock->GetAverageNeighborVelocity();
	if (averageVel.SquaredLength() > 0.0) steering.LinearVelocity = averageVel;

	steering.LinearVelocity.Normalize();
	return steering;
}
void VelocityMatch::DebugRender(ASteeringAgent& Agent) 
{
	// TODO : velocity match debug render somehow?
}
