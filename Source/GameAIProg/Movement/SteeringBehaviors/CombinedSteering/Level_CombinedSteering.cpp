#include "Level_CombinedSteering.h"

#include "imgui.h"


// Sets default values
ALevel_CombinedSteering::ALevel_CombinedSteering()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_CombinedSteering::BeginPlay()
{
	Super::BeginPlay();

	m_pWander = std::make_unique<Wander>();
	m_pSeek = std::make_unique<Seek>();
	m_pEvade = std::make_unique<Evade>();

	m_pBlendedActor = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, FVector{ -100, 0, 90 }, FRotator::ZeroRotator);
	if (IsValid(m_pBlendedActor)) {
		std::vector<BlendedSteering::WeightedBehavior> weightedBehaviors {
			BlendedSteering::WeightedBehavior{ m_pWander.get(), 0.5f },
			BlendedSteering::WeightedBehavior{ m_pSeek.get(), 0.5f }
		};

		m_pBlendedSteering = std::make_unique<BlendedSteering>(weightedBehaviors);
		m_pBlendedSteering->SetTarget(MouseTarget);

		m_pBlendedActor->SetSteeringBehavior(m_pBlendedSteering.get());
		m_pBlendedActor->SetDebugRenderingEnabled(CanDebugRender);
	}

	m_pPriorityActor = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, FVector{ 100, 0, 90 }, FRotator::ZeroRotator);
	if (IsValid(m_pPriorityActor)) {
		std::vector<ISteeringBehavior*> pPrioritizedBehaviors{
			m_pEvade.get(),
			m_pWander.get()
		};

		m_pPrioritySteering = std::make_unique<PrioritySteering>(pPrioritizedBehaviors);

		m_pPriorityActor->SetSteeringBehavior(m_pPrioritySteering.get());
		m_pPriorityActor->SetDebugRenderingEnabled(CanDebugRender);
	}
}

void ALevel_CombinedSteering::BeginDestroy()
{
	Super::BeginDestroy();

}

// Called every frame
void ALevel_CombinedSteering::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
#pragma region UI
	//UI
	{
		//Setup
		bool windowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Game AI", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	
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
		ImGui::Spacing();
	
		ImGui::Text("Flocking");
		ImGui::Spacing();
		ImGui::Spacing();
	
		if (ImGui::Checkbox("Debug Rendering", &CanDebugRender))
		{
			m_pBlendedActor->SetDebugRenderingEnabled(CanDebugRender);
			m_pPriorityActor->SetDebugRenderingEnabled(CanDebugRender);
		}
		ImGui::Checkbox("Trim World", &TrimWorld->bShouldTrimWorld);
		if (TrimWorld->bShouldTrimWorld)
		{
			ImGuiHelpers::ImGuiSliderFloatWithSetter("Trim Size",
				TrimWorld->GetTrimWorldSize(), 1000.f, 3000.f,
				[this](float InVal) { TrimWorld->SetTrimWorldSize(InVal); });
		}
		
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
	
		ImGui::Text("Behavior Weights");
		ImGui::Spacing();

		ImGuiHelpers::ImGuiSliderFloatWithSetter("Wander",
			m_pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight, 0.f, 1.f,
			[this](float InVal) { m_pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight = InVal; }, "%.2f");
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek",
			m_pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight, 0.f, 1.f,
			[this](float InVal) { m_pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight = InVal; }, "%.2f");
	
		//End
		ImGui::End();
	}
#pragma endregion
	
	// Combined Steering Update
	m_pBlendedSteering->SetTargetAllBlends(MouseTarget);

	// Priority Steering Update
	// TODO: implement Make sure to also evade the wanderer
	m_pPrioritySteering->SetTargetAllPriorities(m_pBlendedActor->CreateTarget());
}
