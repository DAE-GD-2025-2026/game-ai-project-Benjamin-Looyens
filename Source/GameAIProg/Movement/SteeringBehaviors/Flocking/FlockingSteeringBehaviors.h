#pragma once
#include "Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
class Flock;

//COHESION - FLOCKING
//*******************
class Cohesion final : public ISteeringBehavior
{
public:
	Cohesion(Flock* const pFlock) :pFlock(pFlock) {};

	//Cohesion Behavior
	SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& Agent) override;
	void DebugRender(ASteeringAgent& Agent) override;

private:
	Flock* pFlock = nullptr;
};

//SEPARATION - FLOCKING
//*********************
class Separation final : public ISteeringBehavior
{
public:
	Separation(Flock* const pFlock) :pFlock(pFlock) {};

	//Separation Behavior
	SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& Agent) override;
	void DebugRender(ASteeringAgent& Agent) override;

private:
	Flock* pFlock = nullptr;
};

//VELOCITY MATCH - FLOCKING
//************************
