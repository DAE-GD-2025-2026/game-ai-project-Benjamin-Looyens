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

SteeringOutput Arrive::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	const float distance = FVector2D::Distance(Agent.GetPosition(), Target.Position);
	float speed = m_MaxSpeed;

	if (distance < m_SlowRadius) {
		// Lerp factor needs to be:
		// 1.0f -> Distance = m_SlowRadius
		// 0.0f -> Distance = m_StopRadius

		// eg:
		// distance = 700
		// 700 - 300 = 400
		// 400 / 700 = 0.57...

		const float lerpFactor = (distance - m_StopRadius) / (m_SlowRadius - m_StopRadius);
		speed = FMath::Lerp(0.0f, m_MaxSpeed, lerpFactor);
	}
	else if (distance < m_StopRadius) return SteeringOutput{}; // Within stop radius, do not move
	
	Agent.SetMaxLinearSpeed(speed);

	return Seek::CalculateSteering(DeltaT, Agent); // Same exact movement (direction) behavior as seek
}
