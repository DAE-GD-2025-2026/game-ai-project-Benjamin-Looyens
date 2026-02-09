#pragma once

#include <Movement/SteeringBehaviors/SteeringHelpers.h>
#include "Kismet/KismetMathLibrary.h"

class ASteeringAgent;

// SteeringBehavior base, all steering behaviors should derive from this.
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	// Override to implement your own behavior
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) = 0;
	virtual void DebugRender(ASteeringAgent& Agent) {};

	void SetTarget(const FTargetData& NewTarget) { Target = NewTarget; }
	
	template<class T, std::enable_if_t<std::is_base_of_v<ISteeringBehavior, T>>* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	FTargetData Target;
};

// Seek Behavior
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() override = default;

	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
	virtual void DebugRender(ASteeringAgent& Agent) override;
};

// Flee Behavior
class Flee : public Seek
{
public:
	Flee() = default;
	virtual ~Flee() override = default;

	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
	virtual void DebugRender(ASteeringAgent& Agent) override;
};

// Arrive Behavior
class Arrive : public Seek
{
public:
	Arrive() = default;
	virtual ~Arrive() override = default;

	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
	virtual void DebugRender(ASteeringAgent& Agent) override;

protected:
	float m_MaxSpeed = 600.0f; // Default ACharacter speed
	float m_SlowRadius = 1000.0f;
	float m_StopRadius = 150.0f;
};