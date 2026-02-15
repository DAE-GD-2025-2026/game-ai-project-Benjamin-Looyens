#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

// Seek Behavior
SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = Target.Position - Agent.GetPosition(); // No need to normalize, as the AddMovementInput() fuction within the SteeringAgent::Tick() will normalize

	return steering;
}

void Seek::DebugRender(ASteeringAgent& Agent)
{
	constexpr FColor TO_TARGET_COLOR{ 255, 0, 0 };
	constexpr FColor CUR_VELOCITY_COLOR{ 0, 180, 255 };

	const FVector2D& agentPos = Agent.GetPosition();

	UWorld* pWorld = Agent.GetWorld();

	// Line Towards Target
	DrawDebugLine(pWorld, FVector{ agentPos, 1.0f }, FVector{ Target.Position, 1.0f }, TO_TARGET_COLOR);

	// Agent Velocity
	FVector2D curVel = Agent.GetLinearVelocity();
	DrawDebugLine(pWorld, FVector{ agentPos, 1.0f }, FVector{ agentPos + curVel, 1.0f}, CUR_VELOCITY_COLOR);
}

// Flee Behavior
SteeringOutput Flee::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering = Seek::CalculateSteering(DeltaT, Agent);

	steering.LinearVelocity = -steering.LinearVelocity;

	return steering;
}

void Flee::DebugRender(ASteeringAgent& Agent)
{
	// TODO : Override debug render from Seek
	Seek::DebugRender(Agent);

	constexpr FColor AWAY_FROM_TARGET_COLOR{ 0, 255, 0 };

	const FVector2D& agentPos = Agent.GetPosition();
	const FVector2D toTarget = Target.Position - agentPos;
	FVector2D direction = -toTarget;
	direction.Normalize();
	direction *= Agent.GetMaxLinearSpeed();

	UWorld* pWorld = Agent.GetWorld();
	DrawDebugLine(pWorld, FVector{ agentPos, 1.0f }, FVector{ agentPos, 1.0f } + FVector{ direction, 0.0f}, AWAY_FROM_TARGET_COLOR);
}

// Arrive Behavior
SteeringOutput Arrive::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	const float distance = FVector2D::Distance(Agent.GetPosition(), Target.Position);
	float speed = m_MaxSpeed;

	if (distance < m_SlowRadius) {
		const float lerpFactor = (distance - m_StopRadius) / (m_SlowRadius - m_StopRadius);
		speed = FMath::Lerp(0.0f, m_MaxSpeed, lerpFactor);
	}
	else if (distance < m_StopRadius) return SteeringOutput{}; // Within stop radius, do not move
	
	Agent.SetMaxLinearSpeed(speed);

	return Seek::CalculateSteering(DeltaT, Agent); // Same exact movement (direction) behavior as seek
}

void Arrive::DebugRender(ASteeringAgent& Agent)
{
	Seek::DebugRender(Agent);

	constexpr FColor SLOW_COLOR{ 0, 0, 255 };
	constexpr FColor STOP_COLOR{ 0, 0, 150 };

	UWorld* pWorld = Agent.GetWorld();
	// Had to pass the default variables, since I needed to access the X and Y axes
	DrawDebugCircle(pWorld, FVector{ Agent.GetPosition(), 1.0f }, m_SlowRadius, 15, SLOW_COLOR, false, (-1.0f), (uint8)0U, (0.0F), { 1, 0, 0 }, { 0, 1, 0 }, false);
	DrawDebugCircle(pWorld, FVector{ Agent.GetPosition(), 1.0f }, m_StopRadius, 15, STOP_COLOR, false, (-1.0f), (uint8)0U, (0.0F), { 1, 0, 0 }, { 0, 1, 0 }, false);
}

SteeringOutput Face::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering{};

	const float curRot = Agent.GetRotation();
	const float goalRot = FMath::Atan2(Target.Position.Y - Agent.GetPosition().Y, Target.Position.X - Agent.GetPosition().X);
	const float maxRot = Agent.GetMaxAngularSpeed() * DeltaT;
	const float difRot = FMath::FindDeltaAngleDegrees(curRot, FMath::RadiansToDegrees(goalRot)); // AddActorLocalRotation expects degrees

	steering.AngularVelocity = FMath::Clamp(difRot, -maxRot, maxRot);

	return steering;
}

void Face::DebugRender(ASteeringAgent& Agent)
{
	// TODO : Face Debug Render
}
