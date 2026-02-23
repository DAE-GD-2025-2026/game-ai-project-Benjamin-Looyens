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
	
	// TODO: initialize the flock and the memory pool
	float appliedWorld = WorldSize / 2;
	double randX = FMath::RandRange(-appliedWorld, appliedWorld);
	double randY = FMath::RandRange(-appliedWorld, appliedWorld);

	for (int32 index{}; index < Agents.Num(); index++) {
		ASteeringAgent* pAgent = pWorld->SpawnActor<ASteeringAgent>(AgentClass, FVector{ randX, randY, 90 }, FRotator::ZeroRotator);

		// If spawn is invalid, generate a new spawn location within the world and reattempt
		constexpr int MAX_ATTEMPTS = 100;
		int spawnAttempts = 0;
		while (!IsValid(pAgent)) {
			randX = FMath::RandRange(-appliedWorld, appliedWorld);
			randY = FMath::RandRange(-appliedWorld, appliedWorld);

			pAgent = pWorld->SpawnActor<ASteeringAgent>(AgentClass, FVector{ randX, randY, 90 }, FRotator::ZeroRotator);

			if (spawnAttempts++ > MAX_ATTEMPTS) break;
		}

		if (IsValid(pAgent)) {
			Agents[index] = pAgent;
		}
	}
}

Flock::~Flock()
{
 // TODO: Cleanup any additional data
}

void Flock::Tick(float DeltaTime)
{
	for (const auto& pAgent : Agents)
	{
		if (pAgent) pAgent->Tick(DeltaTime);
	}


 // TODO: update the flock
 // TODO: for every agent:
  // TODO: register the neighbors for this agent (-> fill the memory pool with the neighbors for the currently evaluated agent)
  // TODO: update the agent (-> the steeringbehaviors use the neighbors in the memory pool)
  // TODO: trim the agent to the world
}

void Flock::RenderDebug()
{
 // TODO: Render all the agents in the flock
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
 // TODO: Debugrender the neighbors for the first agent in the flock
}

#ifndef GAMEAI_USE_SPACE_PARTITIONING
void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
 // TODO: Implement
}
#endif

FVector2D Flock::GetAverageNeighborPos() const
{
	FVector2D avgPosition = FVector2D::ZeroVector;

 // TODO: Implement
	
	return avgPosition;
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	FVector2D avgVelocity = FVector2D::ZeroVector;

 // TODO: Implement

	return avgVelocity;
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
 // TODO: Implement
}

