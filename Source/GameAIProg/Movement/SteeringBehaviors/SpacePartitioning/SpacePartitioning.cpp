#include "SpacePartitioning.h"

// --- Cell ---
// ------------
Cell::Cell(float Left, float Bottom, float Width, float Height)
{
	BoundingBox.Min = { Left, Bottom };
	BoundingBox.Max = { BoundingBox.Min.X + Width, BoundingBox.Min.Y + Height };
}

std::vector<FVector2D> Cell::GetRectPoints() const
{
	const float left = BoundingBox.Min.X;
	const float bottom = BoundingBox.Min.Y;
	const float width = BoundingBox.Max.X - BoundingBox.Min.X;
	const float height = BoundingBox.Max.Y - BoundingBox.Min.Y;

	std::vector<FVector2D> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(UWorld* pWorld, float Width, float Height, int Rows, int Cols)
	: pWorld{pWorld}
	, SpaceWidth{Width}
	, SpaceHeight{Height}
	, NrOfRows{Rows}
	, NrOfCols{Cols}
{
	//calculate bounds of a cell
	CellWidth = Width / Cols;
	CellHeight = Height / Rows;

	// Create Cells
	Cells.reserve(Rows * Cols);
	for (int index{}; index < Rows * Cols; index++) {
		Cells.emplace_back((index % Rows) * CellWidth - (Width / 2), (index / Cols) * CellHeight - (Height / 2), CellWidth, CellHeight); // Cell(float Left, float Bottom, float Width, float Height);
	}
}

void CellSpace::AddAgent(ASteeringAgent& Agent)
{
	const int indexToAdd = PositionToIndex(Agent.GetPosition());

	if (indexToAdd >= Cells.size()) {
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Agent that is being attempted to be added to cells is out of bounds!"));
		return;
	}

	Cells[indexToAdd].Agents.push_back(&Agent);
}

void CellSpace::UpdateAgentCell(ASteeringAgent& Agent, int& OldIndex_INOUT)
{
	// HACK : incredibly stupid work around
	// but the Tick -> Trimworld order cant be changed
	const float appliedWidth = (SpaceWidth / 2);
	const float appliedHeight = (SpaceHeight / 2);
	FVector2D curPos = Agent.GetPosition();
	if (curPos.X > appliedWidth)			curPos.X -= SpaceWidth;
	else if (curPos.X <= -appliedWidth)		curPos.X += SpaceWidth;
	if (curPos.Y > appliedHeight)			curPos.Y -= SpaceHeight;
	else if (curPos.Y <= -appliedHeight)	curPos.Y += SpaceHeight;

	const int newIndex = PositionToIndex(curPos);

	if (OldIndex_INOUT == newIndex) {
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Indexes are identical [%d]"), oldIndex));
		return;
	}
	if (newIndex >= Cells.size() || newIndex < 0) {
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, FString::Printf(TEXT("newIndex is out of bounds with oldIndex [%d]"), oldIndex));
		return;
	}
	if (OldIndex_INOUT >= Cells.size() || OldIndex_INOUT < 0) {
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, FString::Printf(TEXT("oldIndex is out of bounds with newIndex [%d]"), newIndex));
		return;
	}

	Cells[OldIndex_INOUT].Agents.remove(&Agent);
	Cells[newIndex].Agents.push_back(&Agent);

	OldIndex_INOUT = newIndex; // Update Prev Index
}

void CellSpace::RegisterNeighbors(ASteeringAgent& Agent, float QueryRadius, TArray<ASteeringAgent*>& neighbors_OUT, int& nrOfNeighbors_OUT)
{
	const FVector2D& pos = Agent.GetPosition();

	const FRect neighborBounds{ { pos.X - QueryRadius, pos.Y - QueryRadius },
								{ pos.X + QueryRadius, pos.Y + QueryRadius } };

	for (const auto& cell : Cells) {
		if (!DoRectsOverlap(cell.BoundingBox, neighborBounds)) continue;
			
		for (const auto& pOtherAgent : cell.Agents) {
			if (nrOfNeighbors_OUT >= neighbors_OUT.Num()) break;

			if (!pOtherAgent || pOtherAgent == &Agent) continue;
			if (FVector2D::DistSquared(pos, pOtherAgent->GetPosition()) > FMath::Square(QueryRadius)) continue;

			neighbors_OUT[nrOfNeighbors_OUT++] = pOtherAgent;
		}
	}
}

void CellSpace::EmptyCells()
{
	for (Cell& c : Cells)
		c.Agents.clear();
}

void CellSpace::RenderCells() const
{
	for (const auto& cell : Cells) {
		DrawRect(cell.BoundingBox);

		const FVector textLocation{ cell.BoundingBox.Min.X + (CellWidth / 2), cell.BoundingBox.Min.Y + (CellHeight / 2), 20.0f };
		DrawDebugString(pWorld, textLocation, FString::Printf(TEXT("%d"), cell.Agents.size()), 0, FColor::Blue, 0);
	}
}

void CellSpace::RenderActiveCellsForAgent(const ASteeringAgent& agent, float QueryRadius) const
{
	const FVector2D& pos = agent.GetPosition();

	const FRect neighborBounds{ { pos.X - QueryRadius, pos.Y - QueryRadius },
								{ pos.X + QueryRadius, pos.Y + QueryRadius } };

	for (const auto& cell : Cells) {
		if (!DoRectsOverlap(cell.BoundingBox, neighborBounds)) continue;

		DrawRect(cell.BoundingBox, FColor::Green, 0.1f);
	}
}

int CellSpace::PositionToIndex(FVector2D const & Pos) const
{
	int index{};

	const FVector2D offsetPos = Pos + FVector2D{ SpaceWidth / 2, SpaceHeight / 2 };

	// I feel like there could be a more elegant way to do this?
	for (int colIndex{}; colIndex < NrOfCols; colIndex++) {
		for (int rowIndex{}; rowIndex < NrOfRows; rowIndex++) {
			const bool xCheck = offsetPos.X >= (rowIndex * CellWidth) && 
								offsetPos.X < ((rowIndex + 1) * CellWidth);

			if (xCheck) {
				const bool yCheck = offsetPos.Y >= (colIndex * CellHeight) && 
									offsetPos.Y < ((colIndex + 1) * CellHeight);

				if (yCheck) return index;
			}
			index++;
		}
	}

	return index;
}

bool CellSpace::DoRectsOverlap(FRect const & RectA, FRect const & RectB) const
{
	// Check if the rectangles are separated on either axis
	if (RectA.Max.X < RectB.Min.X || RectA.Min.X > RectB.Max.X) return false;
	if (RectA.Max.Y < RectB.Min.Y || RectA.Min.Y > RectB.Max.Y) return false;
    
	// If they are not separated, they must overlap
	return true;
}

void CellSpace::DrawRect(const FRect& rect, const FColor& color, float heightOffset) const
{
	const FVector A(rect.Min.X, rect.Min.Y, 10.0f + heightOffset);
	const FVector B(rect.Max.X, rect.Min.Y, 10.0f + heightOffset);
	const FVector C(rect.Max.X, rect.Max.Y, 10.0f + heightOffset);
	const FVector D(rect.Min.X, rect.Max.Y, 10.0f + heightOffset);

	DrawDebugLine(pWorld, A, B, color, false);
	DrawDebugLine(pWorld, B, C, color, false);
	DrawDebugLine(pWorld, C, D, color, false);
	DrawDebugLine(pWorld, D, A, color, false);
}
