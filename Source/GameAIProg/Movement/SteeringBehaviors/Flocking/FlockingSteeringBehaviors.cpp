#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering{};

	if (pFlock->GetNrOfNeighbors() > 0) {
		Target.Position = pFlock->GetAverageNeighborPos();
		steering = Seek::CalculateSteering(deltaT, Agent);
	}

	steering.LinearVelocity.Normalize();
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

	m_CurSeparationDirection = steering.LinearVelocity;

	steering.LinearVelocity.Normalize();
	return steering;
}
void Separation::DebugRender(ASteeringAgent& Agent)
{
	constexpr FColor SEPARATION_DIR_COLOR{ 45, 200, 0 };

	UWorld* pWorld = Agent.GetWorld();
	const double agentZ = Agent.GetActorLocation().Z - 85.0;

	if (m_CurSeparationDirection.SquaredLength() > 0.0) {
		DrawDebugLine(pWorld, FVector{ Agent.GetPosition(), agentZ }, FVector{ Agent.GetPosition() + m_CurSeparationDirection, agentZ }, SEPARATION_DIR_COLOR);
	}
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
	constexpr FColor AVG_VELOCITY_COLOR{ 110, 180, 90 };

	UWorld* pWorld = Agent.GetWorld();
	const double agentZ = Agent.GetActorLocation().Z - 85.0;

	const FVector2D averageVel = pFlock->GetAverageNeighborVelocity();
	if (averageVel.SquaredLength() > 0.0) {
		DrawDebugLine(pWorld, FVector{ Agent.GetPosition(), agentZ }, FVector{ Agent.GetPosition() + averageVel, agentZ }, AVG_VELOCITY_COLOR);
	}
}
