#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

// General
// HACK : Debug rendering does Agent.Z -85.0 since the mesh bottom is not the Actor position, but is not ideal
// HACK : Debug rendering based on stored values is not ideal, as behaviors may be applied to multiple agents

// Base Debug Render
void ISteeringBehavior::DebugRender(ASteeringAgent& Agent)
{
	constexpr FColor CUR_VELOCITY_COLOR{ 0, 180, 255 };

	UWorld* pWorld = Agent.GetWorld();

	const double agentZ = Agent.GetActorLocation().Z - 85.0;

	// Agent Velocity
	FVector2D curVel = Agent.GetLinearVelocity();
	DrawDebugLine(pWorld, FVector{ Agent.GetPosition(), agentZ }, FVector{ Agent.GetPosition() + curVel, agentZ }, CUR_VELOCITY_COLOR);
}

// Seek Behavior
SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = Target.Position - Agent.GetPosition(); // No need to normalize, as the AddMovementInput() fuction within the SteeringAgent::Tick() will normalize

	steering.LinearVelocity.Normalize();
	return steering;
}

void Seek::DebugRender(ASteeringAgent& Agent)
{
	ISteeringBehavior::DebugRender(Agent);
	

	constexpr FColor TO_TARGET_COLOR{ 255, 0, 0 };

	UWorld* pWorld = Agent.GetWorld();

	const double agentZ = Agent.GetActorLocation().Z - 85.0;

	// Line Towards Target
	DrawDebugLine(pWorld, FVector{ Agent.GetPosition(), agentZ }, FVector{Target.Position, agentZ }, TO_TARGET_COLOR);
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
	Seek::DebugRender(Agent);

	constexpr FColor AWAY_FROM_TARGET_COLOR{ 0, 255, 0 };

	const double agentZ = Agent.GetActorLocation().Z - 85.0;

	const FVector2D& agentPos = Agent.GetPosition();
	const FVector2D toTarget = Target.Position - agentPos;
	FVector2D direction = -toTarget;
	direction.Normalize();
	direction *= Agent.GetMaxLinearSpeed();

	UWorld* pWorld = Agent.GetWorld();
	DrawDebugLine(pWorld, FVector{ agentPos, agentZ }, FVector{ agentPos, agentZ } + FVector{ direction, 0.0f}, AWAY_FROM_TARGET_COLOR);
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

	const double agentZ = Agent.GetActorLocation().Z - 85.0;

	UWorld* pWorld = Agent.GetWorld();
	// Had to pass the default variables, since I needed to access the X and Y axes
	DrawDebugCircle(pWorld, FVector{ Agent.GetPosition(), agentZ }, m_SlowRadius, 15, SLOW_COLOR, false, (-1.0f), (uint8)0U, (0.0F), { 1, 0, 0 }, { 0, 1, 0 }, false);
	DrawDebugCircle(pWorld, FVector{ Agent.GetPosition(), agentZ }, m_StopRadius, 15, STOP_COLOR, false, (-1.0f), (uint8)0U, (0.0F), { 1, 0, 0 }, { 0, 1, 0 }, false);
}

// Face Behavior
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

// Pursuit Behavior
SteeringOutput Pursuit::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering{};

	const float estimatedTime = FVector2D::Distance(Agent.GetPosition(), Target.Position) / Agent.GetMaxLinearSpeed();
	m_PredictedPos = Target.Position + (Target.LinearVelocity * estimatedTime); // Store in class for Debug Render

	steering.LinearVelocity = m_PredictedPos - Agent.GetPosition();

	steering.LinearVelocity.Normalize();
	return steering;
}

void Pursuit::DebugRender(ASteeringAgent& Agent)
{
	ISteeringBehavior::DebugRender(Agent);

	constexpr float PREDICTED_POS_POINT_SIZE = 10.0f;
	constexpr FColor PREDICTED_POS_COLOR{ 100, 100, 200 };

	const double agentZ = Agent.GetActorLocation().Z - 85.0;

	UWorld* pWorld = Agent.GetWorld();

	DrawDebugPoint(pWorld, FVector{ m_PredictedPos, agentZ }, PREDICTED_POS_POINT_SIZE, PREDICTED_POS_COLOR);
}

// Evade Behavior
SteeringOutput Evade::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering = Pursuit::CalculateSteering(DeltaT, Agent);

	steering.LinearVelocity = -steering.LinearVelocity;

	if (FVector2D::DistSquared(Agent.GetPosition(), Target.Position) >= FMath::Square(m_EvasionRadius)) steering.IsValid = false;

	return steering;
}

void Evade::DebugRender(ASteeringAgent& Agent)
{
	Pursuit::DebugRender(Agent);

	constexpr FColor EVASION_RADIUS_COLOR{ 150, 0, 0 };

	const double agentZ = Agent.GetActorLocation().Z - 85.0;

	UWorld* pWorld = Agent.GetWorld();
	// Had to pass the default variables, since I needed to access the X and Y axes
	DrawDebugCircle(pWorld, FVector{ Agent.GetPosition(), agentZ }, m_EvasionRadius, 15, EVASION_RADIUS_COLOR, false, (-1.0f), (uint8)0U, (0.0F), { 1, 0, 0 }, { 0, 1, 0 }, false);
}

// Wander Behavior
SteeringOutput Wander::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering{};

	// TODO : Ensure the angle change is accurate
	// Angle does not seem to go outside of the top arc?
	const float angleChange = std::clamp((FMath::FRand() * 2.0f) - 1.0f, -m_MaxChange, m_MaxChange);
	m_CurrentAngle += angleChange;

	const FVector forward = Agent.GetActorForwardVector() * m_OffsetDistance;

	m_WanderPos = { FMath::Sin(m_CurrentAngle), FMath::Cos(m_CurrentAngle) };
	m_WanderPos *= m_Radius;
	m_WanderPos += Agent.GetPosition() + FVector2D{ forward.X, forward.Y };

	steering.LinearVelocity = m_WanderPos - Agent.GetPosition();

	steering.LinearVelocity.Normalize();
	return steering;
}

void Wander::DebugRender(ASteeringAgent& Agent)
{
	ISteeringBehavior::DebugRender(Agent);

	constexpr float WANDER_POINT_SIZE = 10.0f;
	constexpr FColor WANDER_POINT_COLOR{ 0, 0, 200 };
	constexpr FColor WANDER_CIRCLE_COLOR{ 0, 0, 100 };

	const double agentZ = Agent.GetActorLocation().Z - 85.0;

	UWorld* pWorld = Agent.GetWorld();

	// Circle
	DrawDebugCircle(pWorld, FVector{ Agent.GetPosition(), agentZ } + Agent.GetActorForwardVector() * m_OffsetDistance,
					m_Radius, 15, WANDER_CIRCLE_COLOR, false, (-1.0f), (uint8)0U, (0.0F), { 1, 0, 0 }, { 0, 1, 0 }, false);

	// Point on Circle
	DrawDebugPoint(pWorld, FVector{ m_WanderPos, agentZ + 0.1f }, WANDER_POINT_SIZE, WANDER_POINT_COLOR); // + 0.1f to ensure it is above the radius circle
}
