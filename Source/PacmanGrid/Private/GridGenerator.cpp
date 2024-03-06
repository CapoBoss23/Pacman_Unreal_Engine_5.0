// Fill out your copyright notice in the Description page of Project Settings.


#include "GridGenerator.h"
#include "TestGridGameMode.h"

constexpr int MapSizeX = 36;
// y cordinate has 28 elements + 1 (end of string character)
constexpr int MapSizeY = 29;

/*
Legend:
O -> OutsideLabyrinthNode
# -> WallNode
" "-> LabyrinthNode
G -> GateNode
.-> PointNode
B-> PowerNode
T -> TunnelNode

Useful coordinates:

GhostExitNode			(21,14)
PacManSpawnNode			(9,13)
GhostRespawnNode		(21,13)
BlinkySpawnNode		(RED)	(21,10)
PinkySpawnNode		(PINK)	(21,11)
InkySpawnNode		(BLUE)	(21,16)
ClydeSpawnNode		(ORANGE)(21,17)

PLAYER NUMBER: (REMOVED FROM MATRIX AS NUMBER ARE SAVED IN SPECIFIC CLASSES)
->  PacMan			(9,13)
->	Blinky		(RED)	(21,10)
->	Pinky		(PINK)	(21,11)
->	Inky		(BLUE)	(21,16)
->	Clyde		(ORANGE)(21,17)

GHOST SCATTER POINT:

Blinky  -> (35,25)
Pinky	-> (35,2)
Inky	-> (1,27)
Clyde	-> (1,0)
*/
const char Map[MapSizeX][MapSizeY] = { 
				"OOOOOOOOOOOOOOOOOOOOOOOOOOOO",
				"OOOOOOOOOOOOOOOOOOOOOOOOOOOO",
				"############################",
				"#..........................#",
				"#.##########.##.##########.#",
				"#.##########.##.##########.#",
				"#......##....##....##......#",
				"###.##.##.########.##.##.###",
				"###.##.##.########.##.##.###",
				"#B..##.......  .......##..B#",
				"#.####.#####.##.#####.####.#",
				"#.####.#####.##.#####.####.#",
				"#............##............#",
				"######.## ######## ##.######",
				"OOOOO#.## ######## ##.#OOOOO",
				"OOOOO#.##          ##.#OOOOO",
				"OOOOO#.## ######## ##.#OOOOO",
				"######.## #HHHHHH# ##.######",
				"TTTTTT.   #HHHHHH#   .TTTTTT",
				"######.## #HHHHHH# ##.######",
				"OOOOO#.## ###GG### ##.#OOOOO",
				"OOOOO#.##    RE    ##.#OOOOO",
				"OOOOO#.##### ## #####.#OOOOO",
				"######.##### ## #####.######",
				"#......##....##....##......#",
				"#.####.##.########.##.####.#",
				"#.####.##.########.##.####.#",
				"#..........................#",
				"#.####.#####.##.#####.####.#",
				"#B#OO#.#OOO#.##.#OOO#.#OO#B#",
				"#.####.#####.##.#####.####.#",
				"#............##............#",
				"############################",
				"OOOOOOOOOOOOOOOOOOOOOOOOOOOO",
				"OOOOOOOOOOOOOOOOOOOOOOOOOOOO",
				"OOOOOOOOOOOOOOOOOOOOOOOOOOOO" };

// Sets default values
AGridGenerator::AGridGenerator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	TileSize = 100.0f;
	SpawnOffset = FVector(TileSize);
}

// Called when the game starts or when spawned
void AGridGenerator::BeginPlay()
{
	Super::BeginPlay();
	GameMode = (ATestGridGameMode*)(GetWorld()->GetAuthGameMode());
	GenerateGrid();
}

bool AGridGenerator::isWalkableChar(const char& node)
{
	// only types of nodes that can BE CROSS NODES
	return (node == '.' || node == ' ' || node == 'B' || node == 'T' || node == 'R' || node == 'E' || node == 'H');
}


// Called every frame
void AGridGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

TMap<FVector2D, AGridBaseNode*> AGridGenerator::GetTileMAp()
{
	return TileMap;
}

void AGridGenerator::GenerateGrid()
{
	//used to check if a node is a CROSSNODE
	int NearWalkableNodes = 0;

	for (int x = 0; x < MapSizeX; x++)
	{
		for (int y = 0; y < MapSizeY - 1; y++)
		{
			// spawn logic
			const char MapTile = Map[x][y];

			FVector OffsetVector(x * SpawnOffset.X, y * SpawnOffset.Y, 0);
			// set the spatial coordinates for the tile spawn function
			const FVector CurrentSpawnPosition = GetActorLocation() + OffsetVector;
			// this function spawns a new tile
			const auto SpawnedNode = SpawnNodeActorById(MapTile, CurrentSpawnPosition);
			// assign grid coordinates to the tile
			SpawnedNode->TileGridPosition = (FVector2D(x, y));
			// assign spatial coordinates to the tile
			SpawnedNode->TileCoordinatesPosition = CurrentSpawnPosition;
			// add the reference to the created tile to the data structures
			Grid.Add(SpawnedNode);
			TileMap.Add(FVector2D(x, y), SpawnedNode);


			//----------------------------------------------------------
			// check and save if IT' S CROSSNODES -> INFO USED BY GHOSTS
			// IF IT' S CROSS NODE -> path computation is made

			if (x >= 3 && x <= 31 && y >= 1 && y <= 27)
			{
				// cross nodes types: 
				// NB: | and - MEANS WALKABLE NODE
				//
				// 2 nodes  
				//             |    |
				//    O-  -O   O-  -O
				//    |    |
				// 
				// 3 nodes:
				//	  |    |   |
				//	  O-  -O  -O-  -O-
				//    	  |    |        |
				//
				// 4 nodes:
				//   |
				//  -O-
				//   |
				//
				// NB: X is ROW index in matrix, 0 is the first row UP and grows DOWNWARDS
				//	   Y is COLUMN index in matrix, 0 is the first column LEFT and grows RIGHTWARDS

				// 2 nodes

				if (Map[x][y] == 'R') {	
					// used to set 2 nodes in front of target as crossnode
					// ghosts can math path in this 2 nodes

					SpawnedNode->isCrossNode = true;
				}
				else if (Map[x][y] == 'E') {
					// used to set 2 nodes in front of target as crossnode
					// ghosts can math path in this 2 nodes

					SpawnedNode->isCrossNode = true;
				}
				if (isWalkableChar(Map[x+1][y]) && isWalkableChar(Map[x][y + 1])) {
					SpawnedNode->isCrossNode = true;
				}
				else if (isWalkableChar(Map[x + 1][y]) && isWalkableChar(Map[x][y - 1])) {
					SpawnedNode->isCrossNode = true;
				}
				else if (isWalkableChar(Map[x - 1][y]) && isWalkableChar(Map[x][y + 1])) {
					SpawnedNode->isCrossNode = true;
				}
				else if (isWalkableChar(Map[x - 1][y]) && isWalkableChar(Map[x][y - 1])) {
					SpawnedNode->isCrossNode = true;
				}// 3 nodes
				else if (isWalkableChar(Map[x - 1][y]) && isWalkableChar(Map[x +1][y])
					&& isWalkableChar(Map[x][y + 1])) {
					SpawnedNode->isCrossNode = true;
				}
				else if (isWalkableChar(Map[x - 1][y]) && isWalkableChar(Map[x + 1][y])
					&& isWalkableChar(Map[x][y - 1])) {
					SpawnedNode->isCrossNode = true;
				}
				else if (isWalkableChar(Map[x][y - 1]) && isWalkableChar(Map[x][y + 1])
					&& isWalkableChar(Map[x - 1][y])) {
					SpawnedNode->isCrossNode = true;
				}
				else if (isWalkableChar(Map[x][y - 1]) && isWalkableChar(Map[x][y + 1])
					&& isWalkableChar(Map[x + 1][y])) {
					SpawnedNode->isCrossNode = true;
				}// 4 nodes
				else if (isWalkableChar(Map[x + 1][y]) && isWalkableChar(Map[x][y + 1]) 
					&& isWalkableChar(Map[x - 1][y]) && isWalkableChar(Map[x][y - 1])) {
					SpawnedNode->isCrossNode = true;
				}
			}
		}
	}
}


AGridBaseNode* AGridGenerator::SpawnNodeActorById(char CharId, FVector Position) const
{
	AGridBaseNode* Node;
	TSubclassOf<AGridBaseNode> ClassToSpawn = AGridBaseNode::StaticClass();

	if (CharId == 'O')
	{
		ClassToSpawn = OutsideLabyrinthNode;
	}
	else if (CharId == '#')
	{
		ClassToSpawn = WallNode;
	}
	else if (CharId == ' ')
	{
		ClassToSpawn = LabyrinthNode;
	}
	else if (CharId == 'T')
	{
		ClassToSpawn = TunnelNode;
	}
	else if (CharId == 'H')
	{
		// we need nodes inside the house to be walkable
		ClassToSpawn = LabyrinthNode;
	}
	else if (CharId == 'G')
	{
		ClassToSpawn = GateNode;
	}
	else if (CharId == '.')
	{
		ClassToSpawn = PointNode;
	}
	else if (CharId == 'B')
	{
		ClassToSpawn = PowerNode;
	}
	else if (CharId == 'E')
	{
		ClassToSpawn = LabyrinthNode;
	}
	else if (CharId == 'P')
	{
		ClassToSpawn = LabyrinthNode;
	}
	else if (CharId == 'R')
	{
		ClassToSpawn = LabyrinthNode;
	}
	else if (CharId == '1')
	{
		ClassToSpawn = LabyrinthNode;
	}
	else if (CharId == '2')
	{
		ClassToSpawn = LabyrinthNode;
	}
	else if (CharId == '3')
	{
		ClassToSpawn = LabyrinthNode;
	}
	else
	{
		// 4 - ClydeTile	(ORANGE)
		// 
		// I created a new node for path visualization
		// to be replaced if necessary with a "dot" representing the food"
		ClassToSpawn = LabyrinthNode;
	}
	Node = GetWorld()->SpawnActor<AGridBaseNode>(ClassToSpawn, Position, FRotator::ZeroRotator);
	// if node spawned is a powerNode then start the flashing
	if (ClassToSpawn == PowerNode)
	{
		Cast<APowerNode>(Node)->PowerNodeVisible();
	}
	return Node;
}


bool AGridGenerator::IsNodeValidForWalk(AGridBaseNode* Node)
{
	if (Node == nullptr) return false;
	if (Node->EWalkableId == NotWalkable) return false;

	return true;
}

AGridBaseNode* AGridGenerator::GetNextNode(const FVector2D StartCoords, FVector InputDir)
{

	const float RequestedX = StartCoords.X + InputDir.X;
	const float RequestedY = StartCoords.Y + InputDir.Y;
	// the clamp function returns a number between min and max if in range
	// if smaller than min returns the min
	// if larger than max returns the max
	const float ClampedX = FMath::Clamp(RequestedX, 0.f, MapSizeX - 1);	// 0 <= X <= 35
	const float ClampedY = FMath::Clamp(RequestedY, 0.f, MapSizeY - 2);	// 0 <= Y <= 27
	AGridBaseNode* PossibleNode = GetTileMAp()[FVector2D(ClampedX, ClampedY)];

	if (RequestedX > ClampedX || RequestedX < 0)
	{
		//X overflow
		PossibleNode = nullptr;
	}
	if (RequestedY > ClampedY || RequestedY < 0)
	{
		//Y overflow
		PossibleNode = nullptr;
	}
	return PossibleNode;
}

FVector2D AGridGenerator::GetPosition(const FHitResult& Hit)
{
	return Cast<AGridBaseNode>(Hit.GetActor())->GetGridPosition();
}

TArray<AGridBaseNode*>& AGridGenerator::GetTileArray()
{
	return Grid;
}

FVector AGridGenerator::GetRelativeLocationByXYPosition(const int32 InX, const int32 InY)
{
	return  TileSize * FVector(InX, InY, 0);
}

FVector2D AGridGenerator::GetXYPositionByRelativeLocation(const FVector& Location)
{
	double x = floor(Location[0] / (TileSize));
	double y = floor(Location[1] / (TileSize));
	return FVector2D(x, y);
}

FVector2D AGridGenerator::GetTwoDOfVector(FVector DDDVector)
{
	return FVector2D(DDDVector.X, DDDVector.Y);
}

FVector AGridGenerator::GetThreeDOfTwoDVector(FVector2D DDDVector)
{
	return FVector(DDDVector.X, DDDVector.Y, 0.0f);
}

AGridBaseNode* AGridGenerator::GetNodeByCoords(const FVector2D Coords)
{
	if (Coords.X > MapSizeX - 1 || Coords.Y > MapSizeY - 2) return nullptr;
	if (Coords.X < 0 || Coords.Y < 0) return nullptr;
	return GetTileMAp()[FVector2D(Coords.X, Coords.Y)];
}

bool AGridGenerator::IsNodeReachableAndNextToCurrentPosition(const FVector2D CurrentCoordinates, const FVector2D TargetCoords)
{
	const float DistX = FMath::Abs(CurrentCoordinates.X - TargetCoords.X);
	const float DistY = FMath::Abs(CurrentCoordinates.Y - TargetCoords.Y);
	if (DistX > 1 || DistY > 1) return false;
	AGridBaseNode* const N = GetNodeByCoords(TargetCoords);
	if (N && N->EWalkableId == NotWalkable) return false;
	return true;
}

AGridBaseNode* AGridGenerator::GetClosestNodeFromMyCoordsToTargetCoords(const FVector2D StartCoords, const FVector2D TargetCoords, FVector IgnoredDir)
{
	//Get all neighbours
	const TArray<FDirNode> Neighbours = GetNodeNeighbours(GetNodeByCoords(StartCoords));
	float Dist = FLT_MAX;
	AGridBaseNode* ReturnNode = nullptr;

	// scan CLOCKWISE: top, right, bottom, left -> respect specific MINIMUM gap IN CASE
	// BY 2 NODES AT THE SAME DISTANCE
	for (FDirNode FDirNode : Neighbours)
	{
		if (FDirNode.Node == nullptr || FDirNode.Dir == IgnoredDir || (FDirNode.Node->EWalkableId == NotWalkable))
			continue;

		const float TempDist = FVector2D::Distance(FDirNode.Node->GetGridPosition(), TargetCoords);
		if (TempDist < Dist)
		{
			Dist = TempDist;
			ReturnNode = FDirNode.Node;
		}
	}
	return ReturnNode;
}

TArray<FDirNode> AGridGenerator::GetNodeNeighbours(const AGridBaseNode* Node)
{
	TArray<FDirNode> Vec;
	if (Node)
	{
		//check order: clock-wise { Up, Right, Down, Left}
		//up
		Vec.Add(FDirNode(GetNextNode(Node->GetGridPosition(), FVector::ForwardVector), FVector::ForwardVector));
		//right
		Vec.Add(FDirNode(GetNextNode(Node->GetGridPosition(), FVector::RightVector), FVector::RightVector));
		//down
		Vec.Add(FDirNode(GetNextNode(Node->GetGridPosition(), -FVector::ForwardVector), -FVector::ForwardVector));
		//left
		Vec.Add(FDirNode(GetNextNode(Node->GetGridPosition(), FVector::LeftVector), FVector::LeftVector));
	}
	return Vec;
}

