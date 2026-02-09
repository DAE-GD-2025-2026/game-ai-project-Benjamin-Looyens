#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

// Seek Behavior
SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = Target.Position - Agent.GetPosition(); // No need to normalize, as the AddMovementInput() fuction within the SteeringAgent::Tick() will normalize

	// TODO : Add Debug Render

	return steering;
}

SteeringOutput Flee::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering = Seek::CalculateSteering(DeltaT, Agent);

	steering.LinearVelocity = -steering.LinearVelocity;

	// TODO : Override debug render from Seek?

	return steering;
}

