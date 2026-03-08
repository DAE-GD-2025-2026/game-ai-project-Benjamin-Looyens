#pragma once

#include "FlockingSteeringBehaviors.h"
#include "Movement/SteeringBehaviors/SteeringAgent.h"
#include "Movement/SteeringBehaviors/SteeringHelpers.h"
#include "Movement/SteeringBehaviors/CombinedSteering/CombinedSteeringBehaviors.h"
#include <memory>
#include "imgui.h"
#include "../SpacePartitioning/SpacePartitioning.h"

class Flock final
{
public:
	Flock(
	UWorld* pWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize = 10, 
	float WorldSize = 100.f, 
	ASteeringAgent* const pAgentToEvade = nullptr, 
	bool bTrimWorld = false);

	~Flock();

	void Tick(float DeltaTime);
	void RenderDebug();
	void ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize);

	void RegisterNeighbors(ASteeringAgent* const Agent);
	int GetNrOfNeighbors() const { return NrOfNeighbors; }
	const TArray<ASteeringAgent*>& GetNeighbors() const { return Neighbors; }

	FVector2D GetAverageNeighborPos() const;
	FVector2D GetAverageNeighborVelocity() const;

	void SetTarget_Seek(FSteeringParams const & Target);

private:
	// For debug rendering purposes
	UWorld* pWorld{nullptr};
	
	// Core Flock
	int FlockSize{0};
	float WorldSize{0};
	TArray<ASteeringAgent*> Agents{};
	
	// Neighborhood
	TArray<ASteeringAgent*> Neighbors{};
	int NrOfNeighbors{0};
	float NeighborhoodRadius{200.f};
	
	// Spatial Partitioning
	std::unique_ptr<CellSpace> m_pPartitionedSpace{};
	int NrOfCellsX{ 10 };
	TArray<int> m_PrevPosIndices{};

	//Steering Behaviors
	std::unique_ptr<Separation> m_pSeparationBehavior{};
	std::unique_ptr<Cohesion> m_pCohesionBehavior{};
	std::unique_ptr<VelocityMatch> m_pVelMatchBehavior{};
	std::unique_ptr<Seek> m_pSeekBehavior{};
	std::unique_ptr<Wander> m_pWanderBehavior{};
	std::unique_ptr<Evade> m_pEvadeBehavior{};
	
	std::unique_ptr<BlendedSteering> m_pBlendedSteering{};
	std::unique_ptr<PrioritySteering> m_pPrioritySteering{};

	ASteeringAgent* pAgentToEvade{nullptr};

	// UI and rendering
	bool DebugRenderSteering{false};
	bool DebugRenderNeighborhood{true};
	bool DebugRenderPartitions{true};
	bool DebugRenderOnlyFirstAgent{true};
	bool UsePartitioning{ true };

	void RenderNeighborhood();

	// Default Behavior Weights
	static constexpr float s_STARTING_SEEK = 0.27f;
	static constexpr float s_STARTING_COHESION = 0.24f;
	static constexpr float s_STARTING_SEPARATION = 0.5f;
	static constexpr float s_STARTING_VEL_MATCH = 0.2f;
	static constexpr float s_STARTING_VEL_WANDER = 0.25f;
};
