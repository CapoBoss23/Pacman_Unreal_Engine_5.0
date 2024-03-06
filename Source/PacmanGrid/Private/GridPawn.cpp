// Fill out your copyright notice in the Description page of Project Settings.


#include "GridPawn.h"
#include "GridPlayerController.h"
#include "TestGridGameMode.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AGridPawn::AGridPawn()
{
	// the pawn is automatically spawned at the player start position
	// given that the default pawn has been set in the settings as BP_GridPawn
	//
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Collider = CreateDefaultSubobject<UBoxComponent>(TEXT("Collider"));
	RootComponent = Collider;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	StaticMesh->SetupAttachment(Collider);
	
	LastNode = nullptr;
	TargetNode = nullptr;
	NextNode = nullptr;

	//teleport delay -> FOR GHOST
	ghotst_teleport_delay = 0.5f;
}

void AGridPawn::SetVerticalInput(float AxisValue)
{
}

void AGridPawn::SetHorizontalInput(float AxisValue)
{
}


// Called when the game starts or when spawned
void AGridPawn::BeginPlay()
{
	Super::BeginPlay();
	GameMode = (ATestGridGameMode*)(GetWorld()->GetAuthGameMode());
	TheGridGen = GameMode->GField;
	GridGenTMap = TheGridGen->GetTileMAp();

	//save in StaticArray 2 teleport tile* -> save LOTS of function calls
	//left teleport (18,0) -> (18,27)
	TeleportStaticArray[0] = *(GridGenTMap.Find(FVector2D(18, 0)));

	//right teleport (18,27) -> (18,0)
	TeleportStaticArray[1] = *(GridGenTMap.Find(FVector2D(18, 27)));
	//----------------------------------------------
	// SPEED DATA PUT IN DERIVED CLASSES 
	CanMove = true;
	Collider->OnComponentBeginOverlap.AddDynamic(this, &AGridPawn::OnOverlapBegin);
	//-----------------------------------------------

	//get GameInstance Reference for every GridPawn
	GameInstance = Cast<UMyGameInstance> (UGameplayStatics::GetGameInstance(GetWorld()));
}

void AGridPawn::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

// Called every frame
void AGridPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (CanMove)
	{
		HandleMovement();
	}
}

void AGridPawn::OnClick()
{
}

void AGridPawn::HandleMovement()
{
	//IF PAWN IS IN TELEPORT NODE
	MoveToCurrentTargetNode();
	if (TargetNode == nullptr)
	{
		if (NextNode != nullptr)
		{
			SetTargetNode(NextNode);
			SetNextNode(nullptr);
		}
	}
}

// Called to bind functionality to input
void AGridPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	APawn::SetupPlayerInputComponent(PlayerInputComponent);
}

void AGridPawn::MoveToCurrentTargetNode()
{
	if (TargetNode == nullptr) return;
	
		//spawned at z=6 beacuse LOWER THAN THAT GHOST DOES NOT SPAWN
		FVector Z_Spawn_offset = FVector(0, 0, 6);

		//in this way GHOSTS AND PACMAN stay at z=6 -> WE HAD SPAWNING PROBLEM AT a lower z level
		//for GHOSTS -> this way ALL APWNS are at the same level
		FVector Target_Fvector = TargetNode->GetActorLocation() + Z_Spawn_offset;
		//this DOES NOT NEED Z_Spawn_offset bacause Pawn actual location is already at z=6
		FVector PawnLocation = GetActorLocation();
		
		const float Dist = FMath::Abs(FVector::Dist2D(Target_Fvector, PawnLocation));
		if (Dist <= AcceptedDistance)
		{
			OnNodeReached();
			return;
		}
		// interpolation function that makes the pawn move to a new position given the current position
		const FVector2D StartVector = TheGridGen->GetTwoDOfVector(GetActorLocation());
		const FVector2D EndVector = TheGridGen->GetTwoDOfVector(TargetNode->GetActorLocation());
		//CurrentMovementSpeed:
		const auto Pos = FMath::Vector2DInterpConstantTo(StartVector, EndVector, GetWorld()->GetDeltaSeconds(), CurrentMovementSpeed);
		const FVector Location(Pos.X, Pos.Y, GetActorLocation().Z);
		SetActorLocation(Location);
}

void AGridPawn::OnNodeReached()
{
	CurrentGridCoords = TargetNode->GetGridPosition();
	LastNode = TargetNode;
	SetTargetNode(nullptr);

	//TELEPORT IF (18,27) -> (18,0)
	if (TeleportStaticArray[1] == LastNode && LastValidInputDirection == FVector(0, 1, 0))
	{
		const FVector Location(1850.0f, 50.0f, GetActorLocation().Z);
		//Set current grid position
		CurrentGridCoords = FVector2D(18, 0);
		//set lastnode
		LastNode = *(GridGenTMap.Find(FVector2D(18, 0)));
		//set nextnode  as the following node on the right of teleport node
		SetNextNode(*(GridGenTMap.Find(FVector2D(18, 1))));
		//set targetnode
		SetTargetNode(NextNode);

		//teleport pacman to (18,0)
		SetActorLocation(Location);
	}

	//TELEPORT IF (18,0) -> (18,27)
	if (TeleportStaticArray[0] == LastNode && LastValidInputDirection == FVector(0, -1, 0))
	{
		const FVector Location(1850.0f, 2750.0f, GetActorLocation().Z);
		//Set current grid position
		CurrentGridCoords = FVector2D(18, 27);
		//set lastnode
		LastNode = *(GridGenTMap.Find(FVector2D(18, 27)));
		//set nextnode  as the following node on the left of teleport node
		SetNextNode(*(GridGenTMap.Find(FVector2D(18, 26))));
		//set targetnode
		SetTargetNode(NextNode);

		//teleport pacman to (18,0)
		SetActorLocation(Location);
	}
}

void AGridPawn::SetTargetNode(AGridBaseNode* Node)
{
	TargetNode = Node;
}

void AGridPawn::SetNextNode(AGridBaseNode* Node)
{
	NextNode = Node;
}

void AGridPawn::SetNodeGeneric(const FVector Dir)
{
	const auto Node = TheGridGen->GetNextNode(CurrentGridCoords, Dir);
	if (TheGridGen->IsNodeValidForWalk(Node))
	{
		SetTargetNode(Node);
	}
}




void AGridPawn::SetNextNodeByDir(FVector InputDir)
{
	//conditional operator syntax:  (condition)? expr_if_true : expr_if_false 
	const FVector2D Coords = TargetNode ? TargetNode->GetGridPosition() : LastNode->GetGridPosition();
	const auto Node = GameMode->GField->GetNextNode(Coords, InputDir);
	if (GameMode->GField->IsNodeValidForWalk(Node))
	{
		SetNextNode(Node);
		SetLastValidDirection(InputDir);
	}
}

AGridBaseNode* AGridPawn::GetLastNode() const
{
	return LastNode;
}

AGridBaseNode* AGridPawn::GetTargetNode() const
{
	return TargetNode;
}

FVector2D AGridPawn::GetLastNodeCoords() const
{
	if (LastNode)
	{
		return LastNode->GetGridPosition();
	}
	return FVector2D(0, 0);
}

FVector2D AGridPawn::GetTargetNodeCoords() const
{
	if (TargetNode)
	{
		return TargetNode->GetGridPosition();
	}
	return FVector2D::ZeroVector;
}

//--------------------------------------------
FVector AGridPawn::GetLastValidDirection() const
{
	return LastValidInputDirection;
}

void AGridPawn::SetLastValidDirection(FVector Dir)
{
	LastValidInputDirection = Dir;
}

void AGridPawn::SetLastDirection(FVector Dir)
{
	LastInputDirection = Dir;
}

FVector2D AGridPawn::GetCurrentGridCoords()
{
	return CurrentGridCoords;
}

void AGridPawn::SetCurrentGridCoords(FVector2D CurrentGridCoords_to_set)
{
	CurrentGridCoords = CurrentGridCoords_to_set;
}

void AGridPawn::SetLastNode(AGridBaseNode* newLastNode)
{
	LastNode = newLastNode;
}
