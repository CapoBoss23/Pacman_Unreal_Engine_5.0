// Fill out your copyright notice in the Description page of Project Settings.


#include "GhostPawn.h"
#include "PacManPawn.h"
#include "TestGridGameMode.h"
#include "PowerNode.h"
#include "Kismet/GameplayStatics.h"

AGhostPawn::AGhostPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//STATES GHOSTS SPEED

	//values are PROPORTIONED: 
	ChaseAndScatterGhostsMovementSpeed = 844.0f;

	FrightGhostsMovementSpeed = 563.0f;

	RespawnGhostsMovementSpeed = 2000.0f;

	// cruise elroy 1 moves at the same speed as pacman
	CruiseElroyMovementSpeed = 900.0f;

	// speed inside the house ==> will be changed only once the ghost exits
	GhostHouseMovementSpeed = 800.0f;

	//starting state: chase
	CurrentMovementSpeed = ChaseAndScatterGhostsMovementSpeed;

	// attach additional meshes to the root component (collider)
	StaticMeshBlue = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Blue Mesh"));
	StaticMeshWhite = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("White Mesh"));

	StaticMeshLeftEye = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Left Eye Mesh"));
	StaticMeshRightEye = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Right Eye Mesh"));
	StaticMeshLeftPupil = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Left Pupil Mesh"));
	StaticMeshRightPupil = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Right Pupil Mesh"));

	StaticMeshBlue->SetupAttachment(Collider);
	StaticMeshWhite->SetupAttachment(Collider);
	StaticMeshLeftEye->SetupAttachment(Collider);
	StaticMeshRightEye->SetupAttachment(Collider);
	StaticMeshLeftPupil->SetupAttachment(Collider);
	StaticMeshRightPupil->SetupAttachment(Collider);
	// set visibility to false, will be changed ONLY in frightened state
	StaticMeshBlue->SetVisibility(false);
	StaticMeshWhite->SetVisibility(false);

	// for first 3 seconds (out of 6) the ghost will be just blue 
	Change_Mesh_Time = 3.0f;
	Flash_Time = 0.23f;
	Flash_Counter = 0;


	//Ghost state variable initialization
	//to use it I needed to ADD #include "TestGridGameMode.h" HERE IN CPP and break a circular dependency
	Ghost_State = ECurrentState::Scatter;
	eaten = false;
	RespawnReached = false;
	isElroy = false;
	isElroy2 = false;

	//random generator initialization
	Frightened_State_RandomGenerator = FRandomStream(RandomGenerator_Seed);

	//-------------------------------------------------------------------
	// INITIALIZE Direction static array USED in RANDOM DECISION IN FRIGHT STATE 

	//UP
	Dir_Vector[0] = FVector(1, 0, 0);

	//RIGHT
	Dir_Vector[1] = FVector(0, 1, 0);

	//DOWN
	Dir_Vector[2] = FVector(-1, 0, 0);

	//LEFT
	Dir_Vector[3] = FVector(0, -1, 0);
	//-------------------------------------------------------------------
	// GHOST HOUSE
	isInsideHouse = false;
	isExitingHouse = false;
	isEnteringHouse = false;
}

void AGhostPawn::BeginPlay()
{
	Super::BeginPlay();
	FVector2D StartNode = TheGridGen->GetXYPositionByRelativeLocation(GetActorLocation());
	
	LastNode = TheGridGen->TileMap[StartNode];
	Player = Cast<APacManPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), APacManPawn::StaticClass()));
}

void AGhostPawn::OnNodeReached()
{
	//same for all nodes
	CurrentGridCoords = TargetNode->GetGridPosition();
	LastNode = TargetNode;

	if (LastNode->isCrossNode) { 
		// IF CROSS NODE or Ghost is eaten (need to check position at every tile)
		// usual business as before -> with these instructions 
		// NEW TARGET NODE WILL BE DECIDED in next tick
		SetTargetNode(nullptr);
	}
	else
	{	
		//NOT A CROSSNODE -> KEEP ON GOING WITH THIS DIRECTION
		

		//CHECK IF IT IS A TELEPORT NODE!!! (TYPE TUNNEL NODE) 
		// -> USE 2 TELEPORT CHECK TO MOVE THE GHOST
		//  IF NOT KEEP ON GOING TO SAME DIRECTION
		//

		//TELEPORT IF (18,27) -> (18,0)
		if (TeleportStaticArray[1] == TargetNode && LastValidInputDirection == FVector(0, 1, 0))
		{
			const FVector Location(1850.0f, 50.0f, GetActorLocation().Z);

			//Set current grid position
			CurrentGridCoords = FVector2D(18, 0);
			//set lastnode
			LastNode = *(GridGenTMap.Find(FVector2D(18, 0)));
			//set nextnode as the one to the right on the teleport node
			SetNextNode(*(GridGenTMap.Find(FVector2D(18, 1))));
			//set targetnode
			SetTargetNode(NextNode);

			//teleport pacman to (18,0)
			SetActorLocation(Location);

		} //TELEPORT IF (18,0) -> (18,27)
		else if (TeleportStaticArray[0] == TargetNode && LastValidInputDirection == FVector(0, -1, 0))
		{
			const FVector Location(1850.0f, 2750.0f, GetActorLocation().Z);

			//Set current grid position
			CurrentGridCoords = FVector2D(18, 27);
			//set lastnode
			LastNode = *(GridGenTMap.Find(FVector2D(18, 27)));
			//set nextnode QUELLO A SINISTRA DI DOVE TELEPORT
			SetNextNode(*(GridGenTMap.Find(FVector2D(18, 26))));
			//set targetnode
			SetTargetNode(NextNode);

			//teleport pacman to (18,0)
			SetActorLocation(Location);
		}
		else {
			//IT' S NOT A TELEPORT 
			FVector2D NodeCoordsToMoveTo = CurrentGridCoords + TheGridGen->GetTwoDOfVector(LastValidInputDirection);
			AGridBaseNode* NodeToMoveTo = *(GridGenTMap.Find(NodeCoordsToMoveTo));
			
			// verify is NodeToMoveTo is a PowerNode ==> we may have a situation if pacman enters the same node at the same time
			if (Cast<APowerNode>(NodeToMoveTo))
			{
				// verify if pacman is moving towards the same PowerNode
				if (GetPlayerTargetNode() == NodeToMoveTo)
				{
					// pacman has precedence over the ghost ==> gameMode will enter in Frightened state and ghost will be eaten
					this->SetEaten(true);
					while (GameMode->ECurrentState != Freightened)
					{
						// just wait till Frightened state has started
					}
					// now that Frightened state has started initiate respawn procedure
					// for this ghost, as it was eaten by pacman
					GameMode->start_RESPAWN_STATE();
				}
			}

			SetTargetNode(NodeToMoveTo);
		}

	}
}

//------------------------------------------------------------------------
// FUNCTION USED BY GAMEMODE TO SET TARGETS during STATE CHANGES

void AGhostPawn::HandleMovement()
{
	// OVERRIDEN FOR GHOSTS AS THEY NEED TO SLOW DOWN IN TUNNEL

	if (TargetNode == nullptr) return;

	//spawned at z=6 beacuse LOWER THAN THAT GHOST DOES NOT SPAWN
	FVector Z_Spawn_offset = FVector(0, 0, 6);

	//in this way GHOSTS AND PACMAN stay at z=6 -> WE HAD SPAWNING PROBLEM AT a lower z level
	//for GHOSTS -> this way ALL PAWNS are at the same level
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

	if (TargetNode->isTunnel  && !eaten)
	{
		//IN TUNNEL SLOW DOWN
		const auto Pos = FMath::Vector2DInterpConstantTo(StartVector, EndVector, GetWorld()->GetDeltaSeconds(), 0.6*CurrentMovementSpeed);
		const FVector Location(Pos.X, Pos.Y, GetActorLocation().Z);
		SetActorLocation(Location);
	}
	else {
		const auto Pos = FMath::Vector2DInterpConstantTo(StartVector, EndVector, GetWorld()->GetDeltaSeconds(), CurrentMovementSpeed);
		const FVector Location(Pos.X, Pos.Y, GetActorLocation().Z);
		SetActorLocation(Location);
	}
}

void AGhostPawn::set_SCATTER_target()
{
}

void AGhostPawn::set_CHASE_target()
{
}

void AGhostPawn::set_FRIGHTENED_target()
{
	if (!eaten) {
	// RANDOM DECISION FOR THE GHOST ==> SAME FOR EVERY GHOST

	//system of cordinates used by Unreal
	//				
	// 
	// ^ Z-AXIS
	// |
	// |     ^ X-AXIS
	// |    /
	// |   /
	// |  / 
	// | /
	// O---------> Y-AXIS
	// 
	// O -> (0,0,0) in OUR GRID
	// Z-axis Outgoing from the plane XY
	//
	// FVector carthesian cordinates = (x,y,z)
	//


	// static array used to chose random direction declared in GhostPawn.h 
	// AND INITIALIZED IN CONSTUCTOR GhostPawn() IN .cpp
	// AS IT' S THE SAME FOR ALL GHOSTS
	//	
	// array structure according to clock-wise preference order specification
	// 
	// ----------------------------------------------------
	// | U=0 (up) | R=1 (right) | D=2 (down) | L=3 (left) |
	// ----------------------------------------------------
	// random direction (index) is selected. IF NOT WALKABLE run clock-wise other options 
	// WITHOUT reverting direction. use modular algebra to scan the static array

	//------------------------------------------------------
	// Variable used 

	//used to select the static array cell and AS COUNTER VARIABLE IN FOR CYCLE
		uint32 array_index;

		//lenght of static array
		const int32 lenght = 4;

		//variable used to scan ALL the array ONCE
		int32 count = 0;

		//POINTER that will be assigned to target AND USED DURING DECISION PROCESS
		AGridBaseNode* SelectedNode_ptr;

		//selected position to check
		FVector2D SelectedNodeCords;
		// -------------------------------------------------
		//get a random starting direction

		int32 random_number = Frightened_State_RandomGenerator.RandRange(min, max);

		//select random starting direction
		if (random_number >= 0 && random_number <= 249999)
		{
			//UP DIRECTION SELECTED
			array_index = 0;
		}
		else if (random_number >= 250000 && random_number <= 499999) {
			//RIGHT DIRECTION SELECTED
			array_index = 1;
		}
		else if (random_number >= 500000 && random_number <= 749999) {
			//DOWN DIRECTION SELECTED
			array_index = 2;
		}
		else
		{
			// random_number >= 750000 && random_number <= 1000000

			//LEFT DIRECTION SELECTED
			array_index = 3;
		}

		do {
			// check if ghost is eaten, if it is exit immediately
			if (eaten)
			{
				break;
			}

			//GET POSSIBLE NODE TO WALK TO
			SelectedNodeCords = CurrentGridCoords + TheGridGen->GetTwoDOfVector(Dir_Vector[array_index]);

			// check  JUST ENTERED (18,27) and revert direction because of TO FRIGHT STATE CHANGE HAPPENS
			// and want TO REACH (18,28) -> DOES NOT EXIST
			if (SelectedNodeCords == FVector2D(18, 28)) {
				SelectedNodeCords = FVector2D(18, 27); //LEFT teleport position

			}
			// check  JUST ENTERED (18,0) and revert direction because of TO FRIGHT STATE CHANGE HAPPENS
			// and want TO REACH (18,-1) -> DOES NOT EXIST
			else if (SelectedNodeCords == FVector2D(18, -1)) {
				SelectedNodeCords = FVector2D(18, 0); //RIGHT teleport position
			}
			//----------------------------------------------------------------------------------------------------------

			SelectedNode_ptr = *(GridGenTMap.Find(SelectedNodeCords));

			// IF SELECTED NODE IS IN THE FORBIDDEN DIRECTION (opposite of LastValidInputDirection) and is WALKABLE
			if ((Dir_Vector[array_index] != ((-1) * LastValidInputDirection)) && (TheGridGen->IsNodeValidForWalk(SelectedNode_ptr)))
			{
				TargetNode = SelectedNode_ptr;
				break; //and terminate the function
			}
			else
			{
				//UPDATE CYCLE VARIABLES
				//circular update to scan array JUST ONCE
				array_index = (array_index + 1) % lenght;
				count = count + 1;
			}
		} while (count < lenght);

	}
}

void AGhostPawn::set_to_ChaseAndScatterGhostsMovementSpeed()
{
	if (!isElroy)
	{
		CurrentMovementSpeed = ChaseAndScatterGhostsMovementSpeed;
	}
	else if(isElroy)
	{
		setCuiseElroy1MovementSpeed();
	}
	else if (isElroy2)
	{
		setCuiseElroy2MovementSpeed();
	}
}

void AGhostPawn::set_to_FrightGhostsMovementSpeed()
{
	CurrentMovementSpeed = FrightGhostsMovementSpeed;
}

void AGhostPawn::set_to_RespawnGhostMovementSpeed()
{
	CurrentMovementSpeed = RespawnGhostsMovementSpeed;
}

void AGhostPawn::set_to_GhostHouseMovementSpeed()
{
	CurrentMovementSpeed = GhostHouseMovementSpeed;
}

void AGhostPawn::revert_Ghost_Direction()
{
	// also executed when entering in Frightened state, might cause problems if pacman
	// and a ghost stepped on a PowerNode simultaneously ==> IN THIS CASE DO NOT REVERT DIRECTION
	if (!eaten)
	{
		LastValidInputDirection = LastValidInputDirection * (-1);
	}
}

void AGhostPawn::to_SCATTER_STATE()
{
	// CHECK PREVIOUS STATE AND SAVE IT

	if (Ghost_State == ECurrentState::Chase)
	{
		Ghost_Last_State = ECurrentState::Chase;
	}
	else if (Ghost_State == ECurrentState::Freightened)
	{
		Ghost_Last_State = ECurrentState::Freightened;
	}

	//SET NEW STATE
	Ghost_State = ECurrentState::Scatter;

	if (!isInsideHouse) {
		// GHOST IS NOT INSIDE THE HOUSE ==> behave normally		
		set_to_ChaseAndScatterGhostsMovementSpeed();
		
		if (Ghost_Last_State != ECurrentState::Freightened) {
			//revert DIRECTION TO SHOW STATE CHANGE
			revert_Ghost_Direction();
		}

		//this target node will the fixed target in SCATTER STATE

		//FIRST, assign TargetNode= SCATTER target to use to MATH CLOSEST NODE TO WALK TO REACH NEW SCATTER TARGET
		// SET SCATTER TARGET ONLY IF bool isElroy is false
		// this way elroy effectively stays in chase state indefinitely (but direction reversal is still performed)
		if (!isElroy)
		{
			set_SCATTER_target();

			//SECOND, this way we should get the right next node to walk to reach  NEW SCATTER TARGET
			SetGhostTarget();
		}
	}
	
}

void AGhostPawn::to_CHASE_STATE()
{
	// CHECK PREVIOUS STATE AND SAVE IT

	if (Ghost_State == ECurrentState::Scatter)
	{
		Ghost_Last_State = ECurrentState::Scatter;
	}
	else if (Ghost_State == ECurrentState::Freightened)
	{
		Ghost_Last_State = ECurrentState::Freightened;
	}

	//SET NEW STATE
	Ghost_State = ECurrentState::Chase;

	if (!isInsideHouse)
	{
		// just behave normaly, otherwise leave control to SetGhostTarget()
		set_to_ChaseAndScatterGhostsMovementSpeed();

		if (Ghost_Last_State != ECurrentState::Freightened) {
			//revert DIRECTION TO SHOW STATE CHANGE
			revert_Ghost_Direction();
		}

		//FIRST, assign CHASE target(relativeplayerposition) to use to MATH CLOSEST NODE TO WALK TO REACH PACMAN
		set_CHASE_target();
		SetGhostTarget();
	}
}


void AGhostPawn::to_FRIGHTENED_STATE()
{
	// CHECK PREVIOUS STATE AND SAVE IT

	if (Ghost_State == ECurrentState::Scatter)
	{
		Ghost_Last_State = ECurrentState::Scatter;
	}
	else if (Ghost_State == ECurrentState::Chase)
	{
		Ghost_Last_State = ECurrentState::Chase;
	}

	//SET NEW STATE
	Ghost_State = ECurrentState::Freightened;

	set_to_FrightGhostsMovementSpeed();

	//revert DIRECTION TO SHOW STATE CHANGE
	if (!isInsideHouse)
	{
		revert_Ghost_Direction();
	}


	//FIRST, assign FRIGHT STATE target(random position) FIRST TIME TO MAKE GHOST BEHAVE CORRECTLY DURING STATE TRANSITION
	//afterwards it will be called within set_FRIGHTENED_target()
	// DO THIS ONLY IF GHOST WAS NOT EATEN (in this case a problematic situation
	// happened, needs special handling)
	if (!eaten && !isInsideHouse)
	{
		set_FRIGHTENED_target();
	}

	SetGhostTarget();
}

//------------------------------------------------------------------------

void AGhostPawn::Tick(float DeltaTime)
{
	AGridPawn::Tick(DeltaTime);
	if (this->GetTargetNode() == nullptr)
	{
		if (CanMove) {
			SetGhostTarget();
		
			if (CurrentGridCoords == FVector2D(21, 13) && isExitingHouse)
			{
				isInsideHouse = false;
				isExitingHouse = false;
				if (GameMode->ECurrentState == Chase)
				{
					set_to_ChaseAndScatterGhostsMovementSpeed();
					set_CHASE_target();
				}
				else if (GameMode->ECurrentState == Scatter)
				{
					set_to_ChaseAndScatterGhostsMovementSpeed();
					set_SCATTER_target();
				}
				else
				{
					set_to_FrightGhostsMovementSpeed();
					set_FRIGHTENED_target();
				}
				SetGhostTarget();
			}
		}
	}
}

APacManPawn* AGhostPawn::GetPlayer() const
{
	return Player;
}

void AGhostPawn::SetSpeed(float Speed)
{
	CurrentMovementSpeed = Speed;
}

AGridBaseNode* AGhostPawn::GetPlayerRelativeTarget()
{
	return Player->GetLastNode();
}

AGridBaseNode* AGhostPawn::GetPlayerTargetNode()
{
	return Player->GetTargetNode();
}

void AGhostPawn::SetGhostTarget()
{
}

void AGhostPawn::Fright_State_Manager()
{
	// set visibility of base static mesh to false ==> all ghosts become BLUE
	// IMPORTANT: we implement ghost color change by setting the meshes at different heights
	// white mesh will be the highest, then blue mesh in the middle, then base static mesh at the bottom
	// set blue mesh visibility to true so that it emerges
	StaticMeshBlue->SetVisibility(true);
	// then set a timer to start ghost flashing after 3 seconds (half of frigthened state duration)
	GetWorld()->GetTimerManager().SetTimer(Blue_Mesh_TimerHandle, this, &AGhostPawn::Visible_Mesh_White, Change_Mesh_Time, false);

}

void AGhostPawn::Visible_Mesh_White()
{
	// now display white mesh for 0.25 seconds
	// do this only is ghost is not eaten, otherwise it flashes when it shouldn't
	if (!eaten && GameMode->ECurrentState == Freightened)
	{
		StaticMeshWhite->SetVisibility(true);
		GetWorld()->GetTimerManager().SetTimer(Blue_Mesh_TimerHandle, this, &AGhostPawn::Visible_Mesh_Blue, Flash_Time, false);
	}
	else
	{
		Reset_Visible_Mesh();
	}
}

void AGhostPawn::Visible_Mesh_Blue()
{
	// check if current state is Frightened (otherwise flashing may still occurr after this state ends due to concurrency between methods)
	if (GameMode->ECurrentState == Freightened)
	{
		// set white mesh visibility to false so that blue mesh emerges
		// do this only is ghost is not eaten, otherwise it flashes when it shouldn't
		if (!eaten)
		{
			StaticMeshWhite->SetVisibility(false);
			StaticMeshBlue->SetVisibility(true);
		}

		Flash_Counter += 1;
		// to do 6 flashes we need 6 transitions
		if (Flash_Counter < 6 && !eaten)
		{
			GetWorld()->GetTimerManager().SetTimer(Blue_Mesh_TimerHandle, this, &AGhostPawn::Visible_Mesh_White, Flash_Time, false);
		}
		else
		{
			// counter reached the limit ==> must exit flashing mode and reset the visible mesh when it expires
			GetWorld()->GetTimerManager().SetTimer(Blue_Mesh_TimerHandle, this, &AGhostPawn::Reset_Visible_Mesh, Flash_Time, false);
		}
	}
	else
	{
		Reset_Visible_Mesh();
	}
}

void AGhostPawn::Reset_Visible_Mesh()
{
	// stop the timer that may eventually be still active
	GetWorld()->GetTimerManager().PauseTimer(Blue_Mesh_TimerHandle);
	// now reset the meshes and set visibility of both white and blue to false
	// because Frightened State is gonna end
	StaticMeshWhite->SetVisibility(false);
	StaticMeshBlue->SetVisibility(false);
	// reset counter for future use
	Flash_Counter = 0;
}

bool AGhostPawn::GetEaten()
{
	return eaten;
}

void AGhostPawn::SetEaten(bool isEaten)
{
	eaten = isEaten;
}

FVector2D AGhostPawn::GetSpawnLocation()
{
	//METHOD OVERRIDEN BY SPECIFIC DERIVED CLASSES -> 4 GHOSTS
	//but try set same FVector also here
	return FVector2D(21,13);
}

void AGhostPawn::RespawnGhost(FVector2D respawnPosition)
{
	// set correct movement speed
	set_to_RespawnGhostMovementSpeed();
	// only eyes are visibile in this state
	StaticMesh->SetVisibility(false);
	StaticMeshBlue->SetVisibility(false);
	StaticMeshWhite->SetVisibility(false);
	
	TargetNode = *(GridGenTMap.Find(FVector2D(21, 13)));
	SetGhostTarget();
}

void AGhostPawn::setCuiseElroy1MovementSpeed()
{
	// at level 1 pacman speed is at 80%, cruise elroy 2 speed is at 80%
	CurrentMovementSpeed = CruiseElroyMovementSpeed;
}

void AGhostPawn::setCuiseElroy2MovementSpeed()
{
	// at level 1 pacman speed is at 80%, cruise elroy 2 speed is at 85%
	CurrentMovementSpeed = (CruiseElroyMovementSpeed * 85) / 80;
}
