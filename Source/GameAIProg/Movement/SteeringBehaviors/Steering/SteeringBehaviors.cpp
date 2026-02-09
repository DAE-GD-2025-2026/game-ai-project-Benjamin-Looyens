#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

// Seek Behavior
SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent& agent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = Target.Position - agent.GetPosition(); // No need to normalize, as the AddMovementInput() fuction within the SteeringAgent::Tick() will normalize

	// TODO : Add Debug Render

	return steering;
}