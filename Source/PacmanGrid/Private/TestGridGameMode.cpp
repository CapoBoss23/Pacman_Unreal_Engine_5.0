// Fill out your copyright notice in the Description page of Project Settings.


#include "TestGridGameMode.h"
#include "Math/Vector.h"
#include "PacManWidget.h"

ATestGridGameMode::ATestGridGameMode()
{
	// initialize scatter state counter to 0, will increment in start_SCATTER_STATE()
	Scatter_State_Counter = 0;
	PowerNode_Effect_Time = 6.0f;
	Scatter_State12_Time = 7.0f;
	Scatter_State34_Time = 5.0f;
	Chase_State_Time = 20.0f;
	LastState = 0;
	Respawn_Check_Timer = 0.2f;
	countEatenGhosts = 0;
}


void ATestGridGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	// with this position the grid coordinate tile (0,0) has
	// spatial coordinate of the lower left edge (0,0)
	//
	// e.g. spatial coordinate of tile with grid coords (0, 0)
	// 
	// (100,0) -------------- (100,100)
    	//         |            |
	//	   |            |
	//         |  (50, 50)  |
	//         |            |
	//         |            |
	//  (0,0)  -------------- (0, 100)
	//

	FVector GridPos(50.0f, 50.0f, 0.0f);

	// ATTENTION all functions should be normalized if the position
	// of the grid was no longer this one but any one in three-dimensional space
	if (GridGeneratorClass != nullptr)
	{
		// grid spawn 
		GField = GetWorld()->SpawnActor<AGridGenerator>(GridGeneratorClass, GridPos, FRotationMatrix::MakeFromX(FVector(0, 0, 0)).Rotator());
	};

	//------------------------------------------------------------------------------------------------------
	// SPAWN THE GHOSTS here
	GridPawn_references.Blinky_ptr = GetWorld()->SpawnActor<ABlinky>(BlinkyPawn, FVector((100 * 21) + 50, (100 * 10) + 50, 6.0f), FRotationMatrix::MakeFromX(FVector(0, 0, 0)).Rotator());	
	if (GridPawn_references.Blinky_ptr)
	{
	}

	GridPawn_references.Pinky_ptr = GetWorld()->SpawnActor<APinky>(PinkyPawn, FVector((100 * 18) + 50, (100 * 14) + 50, 6.0f), FRotationMatrix::MakeFromX(FVector(0, 0, 0)).Rotator());
	if (GridPawn_references.Pinky_ptr)
	{
		GridPawn_references.Pinky_ptr->isInsideHouse = true;
		// now ghost has to go up
		// PINKY EXITS THE HOUSE IMMEDIATELY
		GridPawn_references.Pinky_ptr->SetLastValidDirection(FVector(1, 0, 0));
		GridPawn_references.Pinky_ptr->isExitingHouse = true;
	}
	
	GridPawn_references.Inky_ptr = GetWorld()->SpawnActor<AInky>(InkyPawn, FVector((100 * 18) + 50, (100 * 12) + 50, 6.0f), FRotationMatrix::MakeFromX(FVector(0, 0, 0)).Rotator());
	if (GridPawn_references.Inky_ptr)
	{
		GridPawn_references.Inky_ptr->isInsideHouse = true;
		// now ghost has to go up
		GridPawn_references.Inky_ptr->SetLastValidDirection(FVector(1, 0, 0));
	}
	
	GridPawn_references.Clyde_ptr = GetWorld()->SpawnActor<AClyde>(ClydePawn, FVector((100 * 18) + 50, (100 * 15) + 50, 6.0f), FRotationMatrix::MakeFromX(FVector(0, 0, 0)).Rotator());
	if (GridPawn_references.Clyde_ptr)
	{
		GridPawn_references.Clyde_ptr->isInsideHouse = true;
		// now ghost has to go up
		GridPawn_references.Clyde_ptr->SetLastValidDirection(FVector(1, 0, 0));
	}

	//------------------------------------------------------------------------------------------------------
	//PacMan REFERENCE
	GridPawn_references.PacMan_ptr = Cast<APacManPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), PacManPawn));

	// need to set ghosts in SCATTER STATE as they start the game this way
	start_SCATTER_STATE();
}

void ATestGridGameMode::start_FRIGHT_STATE()
{
	// NEED TO PAUSE ONGOING TIMERS AND RESUME THEM ONCE FREIGHTENED STATE ENDS

	// first of all I need to check for active timers and PAUSE them
	// understand this by checking the value of property CurrentState
	// also need to save this information to UNPAUSE the timer once Freightened state ends
	if (ECurrentState == ECurrentState::Scatter)
	{
		LastState = 0;
		GetWorld()->GetTimerManager().PauseTimer(Scatter_State_TimerHandle);
	}
	else if (ECurrentState == ECurrentState::Chase)
	{
		LastState = 1;
		GetWorld()->GetTimerManager().PauseTimer(Chase_State_TimerHandle);
	}

	// set current state to Freightened
	ECurrentState = ECurrentState::Freightened;

	
	//notify 5 pawns 
	if (GridPawn_references.PacMan_ptr != nullptr)
	{
		GridPawn_references.PacMan_ptr->start_PowerNode_effect();
	}

	// TRANSITION TO SCATTER STATE ONLY IF GHOST HAS NOT BEEN EATEN
	if (GridPawn_references.Blinky_ptr != nullptr && !GridPawn_references.Blinky_ptr->GetEaten())
	{
		GridPawn_references.Blinky_ptr->Fright_State_Manager();

		//set ghost data correctly
		GridPawn_references.Blinky_ptr->to_FRIGHTENED_STATE();
	}

	if (GridPawn_references.Pinky_ptr != nullptr && !GridPawn_references.Pinky_ptr->GetEaten())
	{
		GridPawn_references.Pinky_ptr->Fright_State_Manager();

		//set ghost data correctly
		GridPawn_references.Pinky_ptr->to_FRIGHTENED_STATE();
	}
	
	if (GridPawn_references.Inky_ptr != nullptr && !GridPawn_references.Inky_ptr->GetEaten())
	{
		GridPawn_references.Inky_ptr->Fright_State_Manager();

		//set ghost data correctly
		GridPawn_references.Inky_ptr->to_FRIGHTENED_STATE();
	}
	
	if (GridPawn_references.Clyde_ptr != nullptr && !GridPawn_references.Clyde_ptr->GetEaten())
	{
		GridPawn_references.Clyde_ptr->Fright_State_Manager();

		//set ghost data correctly
		GridPawn_references.Clyde_ptr->to_FRIGHTENED_STATE();
	}

	//set timer 
	GetWorld()->GetTimerManager().SetTimer(PowerNode_Effect_TimerHandle, this, &ATestGridGameMode::end_FRIGHT_STATE, PowerNode_Effect_Time, false);
}

void ATestGridGameMode::end_FRIGHT_STATE()
{
	// reset counter
	countEatenGhosts = 0;
	//notify 5 pawns 
	if (GridPawn_references.PacMan_ptr != nullptr)
	{
		GridPawn_references.PacMan_ptr->end_PowerNode_effect();
	}

	// stop ghost flashing, need to call Reset_Visible_Mesh() for every ghost 
	GridPawn_references.Blinky_ptr->Reset_Visible_Mesh();
	GridPawn_references.Pinky_ptr->Reset_Visible_Mesh();
	GridPawn_references.Inky_ptr->Reset_Visible_Mesh();
	GridPawn_references.Clyde_ptr->Reset_Visible_Mesh();
	
	// unpause active timer in order to resume normal game flow
	// undestand which one was paused by checking LastState variable
	// don't need to switch state here as it will be done automatically once the timer expires
	if (LastState == 0)
	{
		// last state was Scatter
		GetWorld()->GetTimerManager().UnPauseTimer(Scatter_State_TimerHandle);
		ECurrentState = ECurrentState::Scatter;
		//------------------------------------------------------------------------------------
		//Notifications to GHOSTS of state change to Scatter PACMAN DOES NOT CARE

		if (GridPawn_references.Blinky_ptr != nullptr) {

			GridPawn_references.Blinky_ptr->to_SCATTER_STATE();
		}

		if (GridPawn_references.Pinky_ptr != nullptr) {

			GridPawn_references.Pinky_ptr->to_SCATTER_STATE();
		}
		
		if (GridPawn_references.Inky_ptr != nullptr) {

			GridPawn_references.Inky_ptr->to_SCATTER_STATE();
		}
		
		if (GridPawn_references.Clyde_ptr != nullptr) {

			GridPawn_references.Clyde_ptr->to_SCATTER_STATE();
		}
		
		//------------------------------------------------------------------------------------
	}
	else if (LastState == 1)
	{
		// last state was Chase
		GetWorld()->GetTimerManager().UnPauseTimer(Chase_State_TimerHandle);
		ECurrentState = ECurrentState::Chase;
		//------------------------------------------------------------------------------------
		//Notifications to GHOSTS of state change to Chase PACMAN DOES NOT CARE

		if (GridPawn_references.Blinky_ptr != nullptr) {

			GridPawn_references.Blinky_ptr->to_CHASE_STATE();
		}

		if (GridPawn_references.Pinky_ptr != nullptr) {

			GridPawn_references.Pinky_ptr->to_CHASE_STATE();
		}
		
		if (GridPawn_references.Inky_ptr != nullptr) {

			GridPawn_references.Inky_ptr->to_CHASE_STATE();
		}
		
		if (GridPawn_references.Clyde_ptr != nullptr) {

			GridPawn_references.Clyde_ptr->to_CHASE_STATE();
		}
		//------------------------------------------------------------------------------------
	}
}

void ATestGridGameMode::start_SCATTER_STATE()
{
	ECurrentState = ECurrentState::Scatter;
	Scatter_State_Counter += 1;

	// TRANSITION TO SCATTER STATE ONLY IF GHOST HAS NOT BEEN EATEN
	if (GridPawn_references.Blinky_ptr != nullptr && !GridPawn_references.Blinky_ptr->GetEaten()) {
		
		GridPawn_references.Blinky_ptr->to_SCATTER_STATE();
	}

	if (GridPawn_references.Pinky_ptr != nullptr && !GridPawn_references.Pinky_ptr->GetEaten())
	{
		GridPawn_references.Pinky_ptr->to_SCATTER_STATE();
	}
	
	if (GridPawn_references.Inky_ptr != nullptr && !GridPawn_references.Inky_ptr->GetEaten())
	{
		GridPawn_references.Inky_ptr->to_SCATTER_STATE();
	}
	
	if (GridPawn_references.Clyde_ptr != nullptr && !GridPawn_references.Clyde_ptr->GetEaten())
	{
		GridPawn_references.Clyde_ptr->to_SCATTER_STATE();
	}
	
	// set timer for scatter state expiration and transition to chase state
	// though amount of time set in the timer depends on how many times ghosts have already 
	// entered scatter state during the game
	if (Scatter_State_Counter == 1 || Scatter_State_Counter == 2)
	{
		GetWorld()->GetTimerManager().SetTimer(Scatter_State_TimerHandle, this, &ATestGridGameMode::start_CHASE_STATE, Scatter_State12_Time, false);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(Scatter_State_TimerHandle, this, &ATestGridGameMode::start_CHASE_STATE, Scatter_State34_Time, false);
	}
}

void ATestGridGameMode::start_CHASE_STATE()
{
	ECurrentState = ECurrentState::Chase;
	// TRANSITION TO CHASE STATE ONLY IF GHOST HAS NOT BEEN EATEN
	if (GridPawn_references.Blinky_ptr != nullptr && !GridPawn_references.Blinky_ptr->GetEaten()) {
		GridPawn_references.Blinky_ptr->to_CHASE_STATE();
	}
	
	if (GridPawn_references.Pinky_ptr != nullptr && !GridPawn_references.Pinky_ptr->GetEaten()) {
		GridPawn_references.Pinky_ptr->to_CHASE_STATE();
	}
	
	if (GridPawn_references.Inky_ptr != nullptr && !GridPawn_references.Inky_ptr->GetEaten()) {
		GridPawn_references.Inky_ptr->to_CHASE_STATE();
	}
	
	if (GridPawn_references.Clyde_ptr != nullptr && !GridPawn_references.Clyde_ptr->GetEaten()) {
		GridPawn_references.Clyde_ptr->to_CHASE_STATE();
	}

	// set the timer for the chase state
	// do this only if ghosts entered scatter state less than 4 times
	// else stay in chase mode until the game ends
	if (Scatter_State_Counter < 4)
	{
		GetWorld()->GetTimerManager().SetTimer(Chase_State_TimerHandle, this, &ATestGridGameMode::start_SCATTER_STATE, Chase_State_Time, false);
	}
}


void ATestGridGameMode::start_RESPAWN_STATE()
{
	// understand which ghost was eaten by checking bool eaten
	// then initialize the following pointer with the correct ghost
	// THIS CODE NEEDS TO BE ABLE TO HANDLE MULTIPLE EATEN GHOSTS
	// ==> call RespawnGhost method for every one of them
	AGhostPawn* eatenGhost = nullptr;
	if (GridPawn_references.Blinky_ptr->GetEaten())
	{
		// set value in EatenGhosts array to signal ghost was eaten
		FVector2D respawnLocation = GridPawn_references.Blinky_ptr->GetSpawnLocation();
		// set new ghost target 
		GridPawn_references.Blinky_ptr->RespawnGhost(respawnLocation);
	}

	if (GridPawn_references.Pinky_ptr->GetEaten())
	{
		// set value in EatenGhosts array to signal ghost was eaten
		FVector2D respawnLocation = GridPawn_references.Pinky_ptr->GetSpawnLocation();
		// set new ghost target 
		GridPawn_references.Pinky_ptr->RespawnGhost(respawnLocation);
	}

	if (GridPawn_references.Inky_ptr->GetEaten())
	{
		// set value in EatenGhosts array to signal ghost was eaten
		FVector2D respawnLocation = GridPawn_references.Inky_ptr->GetSpawnLocation();
		// set new ghost target 
		GridPawn_references.Inky_ptr->RespawnGhost(respawnLocation);
	}

	if (GridPawn_references.Clyde_ptr->GetEaten())
	{
		// set value in EatenGhosts array to signal ghost was eaten
		FVector2D respawnLocation = GridPawn_references.Clyde_ptr->GetSpawnLocation();
		// set new ghost target 
		GridPawn_references.Clyde_ptr->RespawnGhost(respawnLocation);
	}

	// if at least one ghost was eaten then call this function recursively
	if (GridPawn_references.Blinky_ptr->GetEaten() || GridPawn_references.Inky_ptr->GetEaten() || GridPawn_references.Pinky_ptr->GetEaten() || GridPawn_references.Clyde_ptr->GetEaten())
	{
		GetWorld()->GetTimerManager().SetTimer(Respawn_Check_TimerHandle, this, &ATestGridGameMode::start_RESPAWN_STATE, Respawn_Check_Timer, false);
	}
}

void ATestGridGameMode::end_RESPAWN_STATE()
{
	// BLINKY
	if (GridPawn_references.Blinky_ptr->RespawnReached)
	{
		GridPawn_references.Blinky_ptr->RespawnReached = false;
		GridPawn_references.Blinky_ptr->SetEaten(false);
		// set base mesh visibility to true and others to false
		GridPawn_references.Blinky_ptr->StaticMeshWhite->SetVisibility(false);
		GridPawn_references.Blinky_ptr->StaticMeshBlue->SetVisibility(false);
		GridPawn_references.Blinky_ptr->StaticMesh->SetVisibility(true);

		// check current state and enter it
		if (ECurrentState == ECurrentState::Chase)
		{
			GridPawn_references.Blinky_ptr->to_CHASE_STATE();
		}
		else if (ECurrentState == ECurrentState::Scatter)
		{
			GridPawn_references.Blinky_ptr->to_SCATTER_STATE();
		}
		else
		{
			if (GetWorld()->GetTimerManager().GetTimerElapsed(PowerNode_Effect_TimerHandle) > 3.0)
			{
				GridPawn_references.Pinky_ptr->Visible_Mesh_Blue();
			}
			GridPawn_references.Blinky_ptr->to_FRIGHTENED_STATE();
		}
	}

	// PINKY
	if (GridPawn_references.Pinky_ptr->RespawnReached)
	{
		GridPawn_references.Pinky_ptr->RespawnReached = false;
		GridPawn_references.Pinky_ptr->SetEaten(false);
		// set base mesh visibility to true and others to false
		GridPawn_references.Pinky_ptr->StaticMeshWhite->SetVisibility(false);
		GridPawn_references.Pinky_ptr->StaticMeshBlue->SetVisibility(false);
		GridPawn_references.Pinky_ptr->StaticMesh->SetVisibility(true);

		// check current state and enter it
		if (ECurrentState == ECurrentState::Chase)
		{
			GridPawn_references.Pinky_ptr->to_CHASE_STATE();
		}
		else if (ECurrentState == ECurrentState::Scatter)
		{
			GridPawn_references.Pinky_ptr->to_SCATTER_STATE();
		}
		else
		{
			if (GetWorld()->GetTimerManager().GetTimerElapsed(PowerNode_Effect_TimerHandle) > 3.0)
			{
				GridPawn_references.Pinky_ptr->Visible_Mesh_Blue();
			}
			GridPawn_references.Pinky_ptr->to_FRIGHTENED_STATE();
		}
	}

	// INKY
	if (GridPawn_references.Inky_ptr->RespawnReached)
	{
		GridPawn_references.Inky_ptr->RespawnReached = false;
		GridPawn_references.Inky_ptr->SetEaten(false);
		// set base mesh visibility to true and others to false
		GridPawn_references.Inky_ptr->StaticMeshWhite->SetVisibility(false);
		GridPawn_references.Inky_ptr->StaticMeshBlue->SetVisibility(false);
		GridPawn_references.Inky_ptr->StaticMesh->SetVisibility(true);

		// check current state and enter it
		if (ECurrentState == ECurrentState::Chase)
		{
			GridPawn_references.Inky_ptr->to_CHASE_STATE();
		}
		else if (ECurrentState == ECurrentState::Scatter)
		{
			GridPawn_references.Inky_ptr->to_SCATTER_STATE();
		}
		else
		{
			if (GetWorld()->GetTimerManager().GetTimerElapsed(PowerNode_Effect_TimerHandle) > 3.0)
			{
				GridPawn_references.Pinky_ptr->Visible_Mesh_Blue();
			}
			GridPawn_references.Inky_ptr->to_FRIGHTENED_STATE();
		}
	}

	// CLYDE
	if (GridPawn_references.Clyde_ptr->RespawnReached)
	{
		GridPawn_references.Clyde_ptr->RespawnReached = false;
		GridPawn_references.Clyde_ptr->SetEaten(false);
		// set base mesh visibility to true and others to false
		GridPawn_references.Clyde_ptr->StaticMeshWhite->SetVisibility(false);
		GridPawn_references.Clyde_ptr->StaticMeshBlue->SetVisibility(false);
		GridPawn_references.Clyde_ptr->StaticMesh->SetVisibility(true);

		// check current state and enter it
		if (ECurrentState == ECurrentState::Chase)
		{
			GridPawn_references.Clyde_ptr->to_CHASE_STATE();
		}
		else if (ECurrentState == ECurrentState::Scatter)
		{
			GridPawn_references.Clyde_ptr->to_SCATTER_STATE();
		}
		else
		{
			if (GetWorld()->GetTimerManager().GetTimerElapsed(PowerNode_Effect_TimerHandle) > 3.0)
			{
				GridPawn_references.Pinky_ptr->Visible_Mesh_Blue();
			}
			GridPawn_references.Clyde_ptr->to_FRIGHTENED_STATE();
		}
	}
}

void ATestGridGameMode::start_CRUISE_ELROY1()
{
	GridPawn_references.Blinky_ptr->isElroy = true;
	GridPawn_references.Blinky_ptr->setCuiseElroy1MovementSpeed();
}

void ATestGridGameMode::start_CRUISE_ELROY2()
{
	GridPawn_references.Blinky_ptr->isElroy2 = true;
	GridPawn_references.Blinky_ptr->setCuiseElroy2MovementSpeed();
}


