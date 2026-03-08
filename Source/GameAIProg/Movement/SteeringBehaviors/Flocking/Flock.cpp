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
	: pWorld{ pWorld }
	, FlockSize{ FlockSize }
	, pAgentToEvade{ pAgentToEvade }
	, WorldSize{ WorldSize }
{
	// Initialize Agent Data
	Agents.SetNum(FlockSize);
	m_PrevPosIndices.SetNum(FlockSize);

	// Create Partitions
	m_pPartitionedSpace = std::make_unique<CellSpace>(pWorld, WorldSize * 2, WorldSize * 2, NrOfCellsX, NrOfCellsX);
	
	// Initialize Behaviors
	m_pSeekBehavior = std::make_unique<Seek>();
	m_pWanderBehavior = std::make_unique<Wander>();
	m_pEvadeBehavior = std::make_unique<Evade>();
	m_pSeparationBehavior = std::make_unique<Separation>(this);
	m_pCohesionBehavior = std::make_unique<Cohesion>(this);
	m_pVelMatchBehavior = std::make_unique<VelocityMatch>(this);

	std::vector<BlendedSteering::WeightedBehavior> weightedBehaviors{			 // Numbers with good results:
		BlendedSteering::WeightedBehavior{ m_pSeekBehavior.get(), 0.2f },		 // 0.27 Seek
		BlendedSteering::WeightedBehavior{ m_pCohesionBehavior.get(), 0.2f },	 // 0.24 Cohesion
		BlendedSteering::WeightedBehavior{ m_pSeparationBehavior.get(), 0.2f },	 // 0.43 Separation
		BlendedSteering::WeightedBehavior{ m_pVelMatchBehavior.get(), 0.2f },	 // 0.2 VelMatcvh
		BlendedSteering::WeightedBehavior{ m_pWanderBehavior.get(), 0.2f },		 // 0.24 wander
	};
	
	m_pBlendedSteering = std::make_unique<BlendedSteering>(weightedBehaviors);

	std::vector<ISteeringBehavior*> pPrioritizedBehaviors{
		m_pEvadeBehavior.get(),
		m_pBlendedSteering.get()
	};

	m_pPrioritySteering = std::make_unique<PrioritySteering>(pPrioritizedBehaviors);

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

			if (spawnAttempts++ > MAX_SPAWN_ATTEMPTS) {
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Could not spawn agent %d"), index));
				break;
			}
		}

		if (IsValid(pAgent)) {
			Agents[index] = pAgent;
			pAgent->SetDebugRenderingEnabled(false);
			pAgent->SetActorTickEnabled(false);
			pAgent->SetSteeringBehavior(m_pPrioritySteering.get());

			m_pPartitionedSpace->AddAgent(*pAgent);
		}
	}

	// Initialize Memory Pool (Neighbors)
	constexpr int MAX_NEIGHBORS = 20;
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
	for (int index{}; index < Agents.Num(); index++) {
		const auto& pAgent = Agents[index];
		int& oldIndex = m_PrevPosIndices[index];

		if (pAgent) {
			if (UsePartitioning) m_pPartitionedSpace->UpdateAgentCell(*pAgent, oldIndex);

			// Register Neighbors
			RegisterNeighbors(pAgent);

			pAgent->Tick(DeltaTime);
		}
	}
}

void Flock::RenderDebug()
{
	if (DebugRenderSteering) {
		if (DebugRenderOnlyFirstAgent && Agents[0]) {
			RegisterNeighbors(Agents[0]);

			m_pPrioritySteering->DebugRender(*(Agents[0]));
		}
		else {
			for (const auto& pAgent : Agents) {
				if (pAgent) {
					RegisterNeighbors(pAgent);

					m_pPrioritySteering->DebugRender(*pAgent);
				}
			}
		}
	}
	if (DebugRenderNeighborhood) RenderNeighborhood();

	if (UsePartitioning) {
		if (DebugRenderPartitions) m_pPartitionedSpace->RenderCells();
		if (DebugRenderPartitions && DebugRenderNeighborhood) m_pPartitionedSpace->RenderActiveCellsForAgent(*(Agents[0]), NeighborhoodRadius);
	}

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

		ImGui::Checkbox("Render Neighbors (Actor 0)", &DebugRenderNeighborhood);
		ImGui::Checkbox("Debug Render Steering", &DebugRenderSteering);
		ImGui::Checkbox("Debug Render Only Agent 0", &DebugRenderOnlyFirstAgent);

		ImGui::Text("Spatial Partitioning");
		ImGui::Spacing();

		ImGui::Checkbox("Use Partitioning", &UsePartitioning); // Maybe on enable partitioning, register them right away?
		ImGui::Checkbox("Debug Render Partitions", &DebugRenderPartitions);

		ImGui::Text("Behavior Weights");
		ImGui::Spacing();

		ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek",
			m_pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight, 0.f, 1.f,
			[this](float InVal) { m_pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight = InVal; }, "%.2f");
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Cohesion",
			m_pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight, 0.f, 1.f,
			[this](float InVal) { m_pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight = InVal; }, "%.2f");
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Separation",
			m_pBlendedSteering->GetWeightedBehaviorsRef()[2].Weight, 0.f, 1.f,
			[this](float InVal) { m_pBlendedSteering->GetWeightedBehaviorsRef()[2].Weight = InVal; }, "%.2f");
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Velocity Match",
			m_pBlendedSteering->GetWeightedBehaviorsRef()[3].Weight, 0.f, 1.f,
			[this](float InVal) { m_pBlendedSteering->GetWeightedBehaviorsRef()[3].Weight = InVal; }, "%.2f");
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Wander",
			m_pBlendedSteering->GetWeightedBehaviorsRef()[4].Weight, 0.f, 1.f,
			[this](float InVal) { m_pBlendedSteering->GetWeightedBehaviorsRef()[4].Weight = InVal; }, "%.2f");

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

	RegisterNeighbors(Agents[0]);

	DrawDebugCircle(pWorld, FVector{ Agents[0]->GetPosition(), 30 }, NeighborhoodRadius, 15, NEIGHBORHOOD_COLOR, false, (-1.0f), (uint8)0U, (0.0F), { 1, 0, 0 }, { 0, 1, 0 }, false);
	DrawDebugPoint(pWorld, FVector{ Agents[0]->GetPosition(), 30 }, 20.0f, SELF_COLOR);

	for (int32 index{}; index < NrOfNeighbors; index++) {
		DrawDebugPoint(pWorld, FVector{ Neighbors[index]->GetPosition(), 30 }, 20.0f, NEIGHBOR_COLOR);
	}
}

void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
	NrOfNeighbors = 0;

	if (UsePartitioning) {
		m_pPartitionedSpace->RegisterNeighbors(*pAgent, NeighborhoodRadius, Neighbors, NrOfNeighbors);
		return; // Do Not register again without partitions, so return
	}

	for (const auto& pOtherAgent : Agents) {
		if (NrOfNeighbors >= Neighbors.Num()) break;
		
		if (pOtherAgent && pAgent != pOtherAgent) { // Ensure other exists (and isnt self)
			if (FVector2D::DistSquared(pAgent->GetPosition(), pOtherAgent->GetPosition()) <= FMath::Square(NeighborhoodRadius)) {
				Neighbors[NrOfNeighbors++] = pOtherAgent;
			}
		}
	}
}

FVector2D Flock::GetAverageNeighborPos() const
{
	if (NrOfNeighbors == 0) return FVector2D::ZeroVector;

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
	m_pSeekBehavior->SetTarget(Target);
}