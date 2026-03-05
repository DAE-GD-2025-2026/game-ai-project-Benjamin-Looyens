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
CellSpace::CellSpace(UWorld* pWorld, float Width, float Height, int Rows, int Cols, int MaxEntities)
	: pWorld{pWorld}
	, SpaceWidth{Width}
	, SpaceHeight{Height}
	, NrOfRows{Rows}
	, NrOfCols{Cols}
	, NrOfNeighbors{0}
{
	Neighbors.SetNum(MaxEntities);
	
	//calculate bounds of a cell
	CellWidth = Width / Cols;
	CellHeight = Height / Rows;

	// Create Cells
	Cells.reserve(Rows * Cols);
	for (int index{}; index < Rows * Cols; index++) {
		// index / rows

		Cells.emplace_back((index % Rows) * CellWidth - (Width / 2), (index / Cols) * CellHeight - (Height / 2), CellWidth, CellHeight); // Cell(float Left, float Bottom, float Width, float Height);
	}
}

void CellSpace::AddAgent(ASteeringAgent& Agent)
{
	// TODO Add the agent to the correct cell
}

void CellSpace::UpdateAgentCell(ASteeringAgent& Agent, const FVector2D& OldPos)
{
	//TODO Check if the agent needs to be moved to another cell.
	//TODO Use the calculated index for oldPos and currentPos for this
}

void CellSpace::RegisterNeighbors(ASteeringAgent& Agent, float QueryRadius)
{
	// TODO Register the neighbors for the provided agent
	// TODO Only check the cells that are within the radius of the neighborhood
}

void CellSpace::EmptyCells()
{
	for (Cell& c : Cells)
		c.Agents.clear();
}

void CellSpace::RenderCells() const
{
	// TODO Render the cells with the number of agents inside of it

	for (const auto& cell : Cells) {
		const FVector A(cell.BoundingBox.Min.X, cell.BoundingBox.Min.Y, 10.0f);
		const FVector B(cell.BoundingBox.Max.X, cell.BoundingBox.Min.Y, 10.0f);
		const FVector C(cell.BoundingBox.Max.X, cell.BoundingBox.Max.Y, 10.0f);
		const FVector D(cell.BoundingBox.Min.X, cell.BoundingBox.Max.Y, 10.0f);

		DrawDebugLine(pWorld, A, B, FColor::Black, false);
		DrawDebugLine(pWorld, B, C, FColor::Black, false);
		DrawDebugLine(pWorld, C, D, FColor::Black, false);
		DrawDebugLine(pWorld, D, A, FColor::Black, false);
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

bool CellSpace::DoRectsOverlap(FRect const & RectA, FRect const & RectB)
{
	// Check if the rectangles are separated on either axis
	if (RectA.Max.X < RectB.Min.X || RectA.Min.X > RectB.Max.X) return false;
	if (RectA.Max.Y < RectB.Min.Y || RectA.Min.Y > RectB.Max.Y) return false;
    
	// If they are not separated, they must overlap
	return true;
}