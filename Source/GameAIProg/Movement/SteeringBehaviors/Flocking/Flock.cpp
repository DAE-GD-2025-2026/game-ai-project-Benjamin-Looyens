#include "Flock.h"
#include "FlockingSteeringBehaviors.h"
#include "Shared/ImGuiHelpers.h"


Flock::Flock(
	UWorld* pWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize,
	float WorldSize,
	ASteeringAgent* const pAgentToEvade,
	bool bTrimWorld)
	: pWorld{pWorld}
	, FlockSize{ FlockSize }
	, pAgentToEvade{pAgentToEvade}
{
	// TEMP : Initializing to a smaller flock for initial testing!
	//Agents.SetNum(FlockSize);
	Agents.SetNum(20);
	
	// Initialize Behaviors
	m_pSeekBehavior = std::make_unique<Seek>();
	m_pWanderBehavior = std::make_unique<Wander>();
	m_pEvadeBehavior = std::make_unique<Evade>();
	m_pCohesionBehavior = std::make_unique<Cohesion>(this);

	// HACK : Feels very wrong to give the behavior of the agent to evade here
	pAgentToEvade->SetSteeringBehavior(m_pWanderBehavior.get());

	// Initialize Flock
	float appliedWorld = WorldSize / 2;
	double randX = FMath::RandRange(-appliedWorld, appliedWorld);
	double randY = FMath::RandRange(-appliedWorld, appliedWorld);

	for (int32 index{}; index < Agents.Num(); index++) {
		ASteeringAgent* pAgent = pWorld->SpawnActor<ASteeringAgent>(AgentClass, FVector{ randX, randY, 90 }, FRotator::ZeroRotator);

		// If spawn is invalid, generate a new spawn location within the world and reattempt
		constexpr int MAX_SPAWN_ATTEMPTS = 100;
		int spawnAttempts = 0;
		while (!IsValid(pAgent)) {
			randX = FMath::RandRange(-appliedWorld, appliedWorld);
			randY = FMath::RandRange(-appliedWorld, appliedWorld);

			pAgent = pWorld->SpawnActor<ASteeringAgent>(AgentClass, FVector{ randX, randY, 90 }, FRotator::ZeroRotator);

			if (spawnAttempts++ > MAX_SPAWN_ATTEMPTS) break;
		}

		if (IsValid(pAgent)) {
			Agents[index] = pAgent;
			pAgent->SetDebugRenderingEnabled(false);
			pAgent->SetActorTickEnabled(false);
			pAgent->SetSteeringBehavior(m_pCohesionBehavior.get());
		}
	}

	// Initialize Memory Pool (Neighbors)
	constexpr int MAX_NEIGHBORS = 10;
	Neighbors.SetNum(MAX_NEIGHBORS);
}

Flock::~Flock()
{
 // TODO: Cleanup any additional data
}

void Flock::Tick(float DeltaTime)
{
	// Update Targets
	m_pEvadeBehavior->SetTarget(pAgentToEvade->CreateTarget());
	
	// Update Flock Agents
	for (const auto& pAgent : Agents) {
		if (pAgent) {
			// Register Neighbors
			RegisterNeighbors(pAgent);
			
			pAgent->Tick(DeltaTime);
		}
	}

 // TODO: update the flock
 // TODO: for every agent:
  // TODO: register the neighbors for this agent (-> fill the memory pool with the neighbors for the currently evaluated agent)
  // TODO: update the agent (-> the steeringbehaviors use the neighbors in the memory pool)
  // TODO: trim the agent to the world
}

void Flock::RenderDebug()
{
	if (DebugRenderSteering) {
		for (const auto& pAgent : Agents) {
			// TEMP : Behavior not true flock behavior
			if (pAgent) {
				RegisterNeighbors(pAgent);
				m_pCohesionBehavior->DebugRender(*pAgent); 
			}
		}
	}
	if (DebugRenderNeighborhood) RenderNeighborhood();
}

void Flock::ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize)
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		bool bWindowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Gameplay Programming", &bWindowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scrollwheel: zoom cam.");
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Flocking");
		ImGui::Spacing();

  // TODO: implement ImGUI checkboxes for debug rendering here

		ImGui::Text("Behavior Weights");
		ImGui::Spacing();

  // TODO: implement ImGUI sliders for steering behavior weights here
		//End
		ImGui::End();
	}
#pragma endregion
#endif
}

void Flock::RenderNeighborhood()
{
	constexpr FColor SELF_COLOR{ 0, 100, 0 };
	constexpr FColor NEIGHBORHOOD_COLOR{ 0, 150, 0 };
	constexpr FColor NEIGHBOR_COLOR{ 0, 255, 0 };

	// HACK : Should this register them?
	RegisterNeighbors(Agents[0]);

	DrawDebugCircle(pWorld, FVector{ Agents[0]->GetPosition(), 30 }, NeighborhoodRadius, 15, NEIGHBORHOOD_COLOR, false, (-1.0f), (uint8)0U, (0.0F), { 1, 0, 0 }, { 0, 1, 0 }, false);
	DrawDebugPoint(pWorld, FVector{ Agents[0]->GetPosition(), 30 }, 20.0f, SELF_COLOR);
	for (int32 index{}; index < NrOfNeighbors; index++) {
		DrawDebugPoint(pWorld, FVector{ Neighbors[index]->GetPosition(), 30 }, 20.0f, NEIGHBOR_COLOR);
	}
}

#ifndef GAMEAI_USE_SPACE_PARTITIONING
void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
 // TODO: Implement
	NrOfNeighbors = 0;

	for (const auto& pOtherAgent : Agents) {
		if (NrOfNeighbors >= Neighbors.Num()) break;
		
		if (pOtherAgent && pAgent != pOtherAgent) { // Ensure other exists (and isnt self)
			if (FVector2D::DistSquared(pAgent->GetPosition(), pOtherAgent->GetPosition()) <= FMath::Square(NeighborhoodRadius)) {
				Neighbors[NrOfNeighbors++] = pOtherAgent;
			}
		}
	}
}
#endif

FVector2D Flock::GetAverageNeighborPos() const
{
	if (NrOfNeighbors == 0) return FVector2D::ZeroVector;

	//FVector2D avgPosition = FVector2D::ZeroVector;
	//const FVector2D avgPosition = std::accumulate(Neighbors.begin(), Neighbors.begin() + static_cast<int32>(NrOfNeighbors), FVector2D::ZeroVector);

	const FVector2D avgPosition = Algo::Accumulate(MakeArrayView(Neighbors).Slice(0, NrOfNeighbors), FVector2D::ZeroVector, 
		[](const FVector2D& Acc, const ASteeringAgent* pNeighbor) {
			return Acc + pNeighbor->GetPosition();
		});

	return avgPosition / NrOfNeighbors;
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	if (NrOfNeighbors == 0) return FVector2D::ZeroVector;

	const FVector2D avgVelocity = Algo::Accumulate(MakeArrayView(Neighbors).Slice(0, NrOfNeighbors), FVector2D::ZeroVector,
		[](const FVector2D& Acc, const ASteeringAgent* pNeighbor) {
			return Acc + pNeighbor->GetLinearVelocity();
		});

	return avgVelocity / NrOfNeighbors;
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
 // TODO: Implement
}

