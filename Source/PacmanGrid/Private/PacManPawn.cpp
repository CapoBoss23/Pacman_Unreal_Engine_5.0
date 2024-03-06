// Fill out your copyright notice in the Description page of Project Settings.


#include "PacManPawn.h"
#include "TestGridGameMode.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "TestGridGameMode.h"

APacManPawn::APacManPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// set this pawn to be controlled by the lowest-numbered player
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	// direction vector initialized with zero (the pawn does not move at game start until
	// one of the w-a-s-d keys is pressed)

	LastInputDirection = FVector(0, 0, 0);
	LastValidInputDirection = FVector(0, 0, 0);

	// initial position of pacman in grid coordinates (9,13)
	CurrentGridCoords = FVector2D(9, 13);
	//------------------------------------------------
	//SPEED SET UP
	NormMovementSpeed = 900.0f;

	FrightMovementSpeed = 1013.0f;
	
	CurrentMovementSpeed = NormMovementSpeed;
	
	are_ghosts_frightened = false;

	OneLifeToAdd = true; 
	FirstFruitSpawned = false;
	SecondFruitSpawned = false;
	Fruit_Despawn_Time = 9.0f;

	UserHUDClass = nullptr;
	PacManWidget = nullptr;

	Collision_Reactivation_Timer = 0.2f;

	Reload_Level_Timer = 4.0f;
	PointNodeCounter = 0;
	PowerNodeCounter = 0;
	isGameOver = false;

	secondCounterActive = false;
	GhostPointCounter = 0;

	Ghost_Exit_Timer = 4.0f;
}

void APacManPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APacManPawn::SetVerticalInput(float AxisValue)
{
		if (AxisValue == 0) return;
		const FVector Dir = (GetActorForwardVector() * AxisValue).GetSafeNormal();
		LastInputDirection = Dir.GetSafeNormal();
		SetNextNodeByDir(LastInputDirection);
}

void APacManPawn::SetHorizontalInput(float AxisValue)
{
	if (AxisValue == 0) return;
	const FVector Dir = (GetActorRightVector() * AxisValue).GetSafeNormal();
	LastInputDirection = Dir;
	SetNextNodeByDir(LastInputDirection);
}

void APacManPawn::OnClick()
{
	FHitResult Hit = FHitResult(ForceInit);
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursor(ECollisionChannel::ECC_Pawn, true, Hit);
	if (AGridBaseNode* CurrTile = Cast<AGridBaseNode>(Hit.GetActor()))
	{
		FVector2D CurrCoords = CurrTile->GetGridPosition();
	}
}

void APacManPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//Bind Actions here
	InputComponent->BindAxis(TEXT("MoveForward"), this, &APacManPawn::SetVerticalInput);
	InputComponent->BindAxis(TEXT("MoveRight"), this, &APacManPawn::SetHorizontalInput);
	// bind the OnClick function to InputComponent for test purpose
	InputComponent->BindAction("Click", IE_Pressed, this, &APacManPawn::OnClick);

}

//---------------------------------------------------------------------------------
//SPEED CHANGES OF INTERACTION WITH POINT NODES BASED ON STATES CHANGES
void APacManPawn::set_to_NormMovementSpeed()
{
	CurrentMovementSpeed = NormMovementSpeed;
}

void APacManPawn::set_to_FrightMovementSpeed()
{
	CurrentMovementSpeed = FrightMovementSpeed;
}

//sets AGridPawn::CurrentMovementSpeed = APacManPawn::FrightMovementSpeed  AND are_ghosts_frightened = TRUE
void APacManPawn::start_PowerNode_effect()
{
	are_ghosts_frightened = true;
	set_to_FrightMovementSpeed();

}

//sets AGridPawn::CurrentMovementSpeed = APacManPawn::NormMovementSpeed  AND are_ghosts_frightened = FALSE
void APacManPawn::end_PowerNode_effect()
{
	are_ghosts_frightened = false;
	set_to_NormMovementSpeed();
}

//---------------------------------------------------------------------------------

void APacManPawn::BeginPlay(){

	Super::BeginPlay();
	// initial position of the pawn (PlayerStart)
	FVector2D StartNode = TheGridGen->GetXYPositionByRelativeLocation(GetActorLocation());
	LastNode = TheGridGen->TileMap[StartNode];

	CurrentMovementSpeed = NormMovementSpeed;

	//--------------------------------------------------------------------------------
	// get the reference to PacManWidget in order to display it on the user interface (UI)
	// wasn't able to do this in GameInstance, but no problem
	// we have a reference to gameinstance here, so we can update the shown variables
	// every time PacManPawn updates them 
	if (IsValid(UserHUDClass))
	{
		PacManWidget = Cast<UPacManWidget>(CreateWidget(GetWorld(), UserHUDClass));

		if (PacManWidget != nullptr)
		{
			PacManWidget->AddToViewport();
		}
	}

	// set number of lives in the UI
	PacManWidget->DisplayLives(GameInstance->lives);
	PacManWidget->DisplayScore(GameInstance->score);

	//--------------------------------------------------------
	// activate timer that makes ghost exit the house
	GetWorld()->GetTimerManager().SetTimer(Ghost_Exit_TimerHandle, this, &APacManPawn::GhostHouseExit, Ghost_Exit_Timer, false);

	//----------------------------------------
	// HIGHSCORE INITIALIZATION WITH DATA SAVED IN HighScore.txt
	FullHighScorePath.Append(GameInstance->FullHighScorePath);
}

void APacManPawn::HandleMovement()
{
	AGridPawn::HandleMovement();	

	// it moves autonomously until the next node is walkable
	if (!TargetNode && !NextNode)
	{
		if (TheGridGen->IsNodeValidForWalk(TheGridGen->GetNextNode(CurrentGridCoords, LastInputDirection)))
		{
			SetLastValidDirection(LastInputDirection);
		}
		SetNodeGeneric(LastValidInputDirection);
	}
}

void APacManPawn::SetTargetNode(AGridBaseNode* Node)
{

	AGridPawn::SetTargetNode(Node);
}

void APacManPawn::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const auto PointNode = Cast<APointNode>(OtherActor);
	if (PointNode && (PointNode->get_EEatableId() == Point) && !(PointNode->is_eaten_node()))
	{
		PointNode->Point_Collider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PointNode->Point_Mesh->SetVisibility(false);
		PointNode->eat_node();

		// this call will trigger the following sequence of actions:
		// ADD SCORE -> GAMEINSTANCE NOTIFICATION -> HUD NOTIFICATION
		GameInstance->add_PointNode_points();

		// communicate to HUD that it needs to synch score with GameInstance
		PacManWidget->DisplayScore(GameInstance->score);
		
		// COUNTERS for pacman (fruitNodes) and ghosts (ghost house exit)
		PointNodeCounter += 1;
		GhostPointCounter += 1;

		// check if counter has reached a certain value 
		// eventually make ghost exit the house
		// need to behave in different ways depending on whether pacman
		// was already eaten at least once (counter will trigger at lower values)
		if (!secondCounterActive)
		{
			if (GhostPointCounter == 30 && GameMode->GridPawn_references.Inky_ptr->isInsideHouse)
			{
				// VALUE SHOULD BE 30
				// make inky exit ghost house
				GameMode->GridPawn_references.Inky_ptr->isExitingHouse = true;
			}
			else if (GhostPointCounter == 60 && GameMode->GridPawn_references.Clyde_ptr->isInsideHouse)
			{
				// VALUE SHOULD BE 60
				// make Clyde exit ghost house
				GameMode->GridPawn_references.Clyde_ptr->isExitingHouse = true;
			}
		}
		else
		{
			// pacman has already been eaten at least once ==> counter triggers at lower values
			if (GhostPointCounter == 7 && GameMode->GridPawn_references.Pinky_ptr->isInsideHouse)
			{
				// make Pinky exit ghost house
				GameMode->GridPawn_references.Pinky_ptr->isExitingHouse = true;
			}
			else if (GhostPointCounter == 17 && GameMode->GridPawn_references.Inky_ptr->isInsideHouse)
			{
				// make Inky exit ghost house
				GameMode->GridPawn_references.Inky_ptr->isExitingHouse = true;
			}
			else if (GhostPointCounter == 32 && GameMode->GridPawn_references.Clyde_ptr->isInsideHouse)
			{
				// should be 32
				// make Clyde exit ghost house
				GameMode->GridPawn_references.Clyde_ptr->isExitingHouse = true;
			}	
		}


		// eventually spawn FruitNode if number of eaten pointNodes has reached a certain amount
		if (PointNodeCounter >= 70 && !FirstFruitSpawned)
		{
			// spawn first fruit through gamemode method
			FirstFruitSpawned = true;
			Spawn_Fruit();
		}
		if (PointNodeCounter >= 170 && !SecondFruitSpawned)
		{
			// spawn first fruit through gamemode method
			SecondFruitSpawned = true;
			Spawn_Fruit();
		}
		// trigger cruise elroy 1 when 20 point nodes are missing (220 collected)
		if ((PointNodeCounter >= 220) && !GameMode->GridPawn_references.Blinky_ptr->isElroy)
		{
			GameMode->start_CRUISE_ELROY1();
		}

		// trigger cuise elroy 2 when 10 point nodes are missing (230 collected)
		if ((PointNodeCounter >= 230) && !GameMode->GridPawn_references.Blinky_ptr->isElroy2)
		{	
			GameMode->start_CRUISE_ELROY2();
		}

		CheckScoreUpdates();

		if (PointNodeCounter == 240 && PowerNodeCounter == 4)
		{
			// PLAYER HAS EATEN ALL POINT NODES ==> open new level
			CanMove = false;
			GameMode->GridPawn_references.Blinky_ptr->CanMove = false;
			GameMode->GridPawn_references.Clyde_ptr->CanMove = false;
			GameMode->GridPawn_references.Pinky_ptr->CanMove = false;
			GameMode->GridPawn_references.Inky_ptr->CanMove = false;

			GetWorld()->GetTimerManager().SetTimer(Reload_Level_TimerHandle, this, &APacManPawn::ReloadLevel, Reload_Level_Timer, false);
		}
		
		// reset timer 
		GetWorld()->GetTimerManager().ClearTimer(Ghost_Exit_TimerHandle);
		GetWorld()->GetTimerManager().SetTimer(Ghost_Exit_TimerHandle, this, &APacManPawn::GhostHouseExit, Ghost_Exit_Timer, false);

		return;	
		
	}

	const auto PowerNode = Cast<APowerNode>(OtherActor);
	if (PowerNode && (PowerNode->get_EEatableId() == Power) && !(PowerNode->is_eaten_node()))
	{
		// set pacman collider off for a fraction of a second
		// this hopefully avoids simultanous collisions (i.e. with a powerNode and with a ghost)
		Collider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetWorld()->GetTimerManager().SetTimer(Collision_Reactivation_TimerHandle, this, &APacManPawn::Activate_Collider, Collision_Reactivation_Timer, false);

		PowerNode->Power_Collider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PowerNode->Power_Mesh->SetVisibility(false);
		PowerNode->eat_node();
		
		// this call will trigger the following sequence of actions:
		// ADD SCORE -> GAMEINSTANCE NOTIFICATION -> HUD NOTIFICATION
		GameInstance->add_PowerNode_points();

		// communicate to HUD that it needs to synch score with GameInstance
		PacManWidget->DisplayScore(GameInstance->score);
		
		CheckScoreUpdates();
		
		PowerNodeCounter += 1;
		if (PointNodeCounter == 240 && PowerNodeCounter == 4)
		{
			// PLAYER HAS EATEN ALL POINT NODES ==> open new level
			CanMove = false;
			GameMode->GridPawn_references.Blinky_ptr->CanMove = false;
			GameMode->GridPawn_references.Clyde_ptr->CanMove = false;
			GameMode->GridPawn_references.Pinky_ptr->CanMove = false;
			GameMode->GridPawn_references.Inky_ptr->CanMove = false;

			GetWorld()->GetTimerManager().SetTimer(Reload_Level_TimerHandle, this, &APacManPawn::ReloadLevel, Reload_Level_Timer, false);
		}

		// GAMEMODE NOTIFICATION AND IT SET TIMERS FOR PACMAN AND GHOST IN FRIGHT STATE
		// transition to Frightened state only if pawns can move
		// otherwise it means NEW LEVEL WILL BE OPENED SOON
		if (CanMove)
		{
			GameMode->start_FRIGHT_STATE();
		}
		
		return;	
	}

	const auto GhostPawn = Cast<AGhostPawn>(OtherActor);
	if (GhostPawn && (GameMode->ECurrentState == Chase || GameMode->ECurrentState == Scatter) 
		&& !(GhostPawn->GetEaten()))
	{
		// set pacman collider off for a fraction of a second
		// this hopefully avoids simultanous collisions (i.e. with a powerNode and with a ghost)
		Collider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetWorld()->GetTimerManager().SetTimer(Collision_Reactivation_TimerHandle, this, &APacManPawn::Activate_Collider, Collision_Reactivation_Timer, false);

		// pacman has been eaten at least once ==> this acts on ghost house mechanics by activating
		// another counter, which triggers at lowers values (same will be done when a ghost will be eaten)
		secondCounterActive = true;
		// reset ghost point counter
		GhostPointCounter = 0;
		
		// need reference to GameInstance on order to decrease number of lives
		// also need to teleport every pawn back to its initial position
		GameInstance->remove_PacMan_life();
		// update value shown on UI
		PacManWidget->DisplayLives(GameInstance->lives);

		// CHECK IF NUMBER OF LIVES IS ZERO ==> GAME OVER
		if (GameInstance->lives < 1)
		{
			// display GAME OVER text on UI
			FString GameOverString = "GAME OVER";
			PacManWidget->DisplayGameOver(GameOverString);
			isGameOver = true;
			// pawns stay still for some seconds, until the level is reloaded
			CanMove = false;
			GameMode->GridPawn_references.Blinky_ptr->CanMove = false;
			GameMode->GridPawn_references.Clyde_ptr->CanMove = false;
			GameMode->GridPawn_references.Pinky_ptr->CanMove = false;
			GameMode->GridPawn_references.Inky_ptr->CanMove = false;

			GetWorld()->GetTimerManager().SetTimer(Reload_Level_TimerHandle, this, &APacManPawn::ReloadLevel, Reload_Level_Timer, false);
		}

		// sleep for 1 or 2 seconds before teleporting pawns
		FPlatformProcess::Sleep(1);
		// now teleport pawns back to their initial position
		// do this for every ghost and pacman
		
		// reset timer
		GetWorld()->GetTimerManager().ClearTimer(Ghost_Exit_TimerHandle);
		GetWorld()->GetTimerManager().SetTimer(Ghost_Exit_TimerHandle, this, &APacManPawn::GhostHouseExit, Ghost_Exit_Timer, false);

		// PACMAN TELEPORT
		const FVector PacManLocation(950.0f, 1350.0f, GetActorLocation().Z);
		// set last valid input direction to vector of zeros as pacman needs to respawn and stay still
		// until player gives new input
		SetLastValidDirection(FVector(0, 0, 0));
		SetLastDirection(FVector(0, 0, 0));

		//-----------------------------------------------------------------------------
		// Set current grid position
		// used for simmetry with method used in ghost update that showed to effective
		SetCurrentGridCoords(FVector2D(9, 13));
		//-----------------------------------------------------------------------------

		// set lastnode
		SetLastNode(*(GridGenTMap.Find(FVector2D(9, 13))));
		// set nextnode to the same as last node as pacman starts still
		SetNextNode(*(GridGenTMap.Find(FVector2D(9, 13))));
		// set targetnode to the same as prevoius nodes as pacman starts still
		SetTargetNode(*(GridGenTMap.Find(FVector2D(9, 13))));
		// teleport pacman to (9,13)
		SetActorLocation(PacManLocation);

		// problems arise if a ghost was in respawn state and another ghost ate pacman
		// then make sure that every ghost exits respawn state
		GameMode->GridPawn_references.Blinky_ptr->RespawnReached = true;
		GameMode->GridPawn_references.Pinky_ptr->RespawnReached = true;
		GameMode->GridPawn_references.Inky_ptr->RespawnReached = true;
		GameMode->GridPawn_references.Clyde_ptr->RespawnReached = true;
		GameMode->end_RESPAWN_STATE();
		//---------------------------------------------------------------------------
			// BLINKY TELEPORT
			// use gamemode reference to get ref to every ghost thanks to its properties
			ABlinky* Blinky_ref = GameMode->GridPawn_references.Blinky_ptr;
			const FVector BlinkyLocation((Blinky_ref->GetSpawnLocation().X * 100) + 50, (Blinky_ref->GetSpawnLocation().Y * 100) + 50, GetActorLocation().Z);

			//-------------------------------------------------------------------------------
			Blinky_ref->SetCurrentGridCoords(Blinky_ref->GetSpawnLocation());
			Blinky_ref->isExitingHouse = false;
			Blinky_ref->isEnteringHouse = false;
			//-------------------------------------------------------------------------------
		
			// update target node
			Blinky_ref->SetLastNode(*(GridGenTMap.Find(Blinky_ref->GetSpawnLocation())));
			Blinky_ref->SetNextNode(*(GridGenTMap.Find(Blinky_ref->GetSpawnLocation())));
			// call setGhostTarget method with updated values 
			Blinky_ref->SetGhostTarget();
			// teleport to spawn position
			Blinky_ref->SetActorLocation(BlinkyLocation);


		//---------------------------------------------------------------------------------
			// PINKY TELEPORT
			APinky* Pinky_ref = GameMode->GridPawn_references.Pinky_ptr;
			Pinky_ref->isInsideHouse = true;
			Pinky_ref->isExitingHouse = false;
			Pinky_ref->isEnteringHouse = false;
			const FVector PinkyLocation((Pinky_ref->GetSpawnLocation().X * 100) + 50, (Pinky_ref->GetSpawnLocation().Y * 100) + 50, GetActorLocation().Z);
		
			//-------------------------------------------------------------------------------
			Pinky_ref->SetCurrentGridCoords(Pinky_ref->GetSpawnLocation());
			//-------------------------------------------------------------------------------

			// update target node
			Pinky_ref->SetLastNode(*(GridGenTMap.Find(Pinky_ref->GetSpawnLocation())));
			Pinky_ref->SetNextNode(*(GridGenTMap.Find(Pinky_ref->GetSpawnLocation())));
			// call setGhostTarget method with updated values 
			Pinky_ref->SetLastValidDirection(FVector(1, 0, 0));
			Pinky_ref->SetTargetNode(*GridGenTMap.Find(FVector2D(18, 14)));
			Pinky_ref->SetGhostTarget();
			// teleport to spawn position
			Pinky_ref->SetActorLocation(PinkyLocation);


		//---------------------------------------------------------------------------------
			// INKY TELEPORT
			AInky* Inky_ref = GameMode->GridPawn_references.Inky_ptr;
			Inky_ref->isInsideHouse = true;
			Inky_ref->isExitingHouse = false;
			Inky_ref->isEnteringHouse = false;
			const FVector InkyLocation((Inky_ref->GetSpawnLocation().X * 100) + 50, (Inky_ref->GetSpawnLocation().Y * 100) + 50, GetActorLocation().Z);

			//-------------------------------------------------------------------------------
			Inky_ref->SetCurrentGridCoords(Inky_ref->GetSpawnLocation());
			//-------------------------------------------------------------------------------

			// update target node
			Inky_ref->SetLastNode(*(GridGenTMap.Find(Inky_ref->GetSpawnLocation())));
			Inky_ref->SetNextNode(*(GridGenTMap.Find(Inky_ref->GetSpawnLocation())));
			// call setGhostTarget method with updated values 
			Inky_ref->SetLastValidDirection(FVector(1, 0, 0));
			Inky_ref->SetTargetNode(*GridGenTMap.Find(FVector2D(18, 12)));
			Inky_ref->SetGhostTarget();
			// teleport to spawn position
			Inky_ref->SetActorLocation(InkyLocation);

		//---------------------------------------------------------------------------------
			// CLYDE TELEPORT
			AClyde* Clyde_ref = GameMode->GridPawn_references.Clyde_ptr;
			Clyde_ref->isInsideHouse = true;
			Clyde_ref->isExitingHouse = false;
			Clyde_ref->isEnteringHouse = false;
			const FVector ClydeLocation((Clyde_ref->GetSpawnLocation().X * 100) + 50, (Clyde_ref->GetSpawnLocation().Y * 100) + 50, GetActorLocation().Z);

			//-------------------------------------------------------------------------------
			Clyde_ref->SetCurrentGridCoords(Clyde_ref->GetSpawnLocation());
			//-------------------------------------------------------------------------------

			// update target node
			Clyde_ref->SetLastNode(*(GridGenTMap.Find(Clyde_ref->GetSpawnLocation())));
			Clyde_ref->SetNextNode(*(GridGenTMap.Find(Clyde_ref->GetSpawnLocation())));
			// call setGhostTarget method with updated values 
			Clyde_ref->SetLastValidDirection(FVector(1, 0, 0));
			Clyde_ref->SetTargetNode(*GridGenTMap.Find(FVector2D(18, 15)));
			Clyde_ref->SetGhostTarget();
			// teleport to spawn position
			Clyde_ref->SetActorLocation(ClydeLocation);

		//---------------------------------------------------------------------------------
			return;
	}
	// pacman eats ghost only if ghost is in frightened state and hasn't been eaten yet 
	// if bool eaten is true then said ghost has been eaten and is moving towards respawn position
	else if (GhostPawn && GameMode->ECurrentState == Freightened && !(GhostPawn->GetEaten()))
	{
		// CURRENT STATE ==> FRIGHTENED, GHOST IS EATEN
		
		GhostPawn->SetEaten(true);
		// activate second counter for ghost house exit, triggers al lower values
		secondCounterActive = true;
		// reset the counter
		GhostPointCounter = 0;

		// reset timer
		GetWorld()->GetTimerManager().ClearTimer(Ghost_Exit_TimerHandle);
		GetWorld()->GetTimerManager().SetTimer(Ghost_Exit_TimerHandle, this, &APacManPawn::GhostHouseExit, Ghost_Exit_Timer, false);

		GameMode->countEatenGhosts += 1;
		// add points to pacman through game instance
		if (GameMode->countEatenGhosts == 1)
		{
			// add 200 points
			GameInstance->add_GhostEat_points();
		}
		else if (GameMode->countEatenGhosts == 2)
		{
			// add 400 points
			GameInstance->add_GhostEat_points();
			GameInstance->add_GhostEat_points();
		}
		else if (GameMode->countEatenGhosts == 3)
		{
			// add 800 points
			GameInstance->add_GhostEat_points();
			GameInstance->add_GhostEat_points();
			GameInstance->add_GhostEat_points();
			GameInstance->add_GhostEat_points();
		}
		else if (GameMode->countEatenGhosts == 4)
		{
			// add 1600 points
			GameInstance->add_GhostEat_points();
			GameInstance->add_GhostEat_points();
			GameInstance->add_GhostEat_points();
			GameInstance->add_GhostEat_points();
			GameInstance->add_GhostEat_points();
			GameInstance->add_GhostEat_points();
			GameInstance->add_GhostEat_points();
			GameInstance->add_GhostEat_points();
		}

		// update score on HUD
		PacManWidget->DisplayScore(GameInstance->score);

		CheckScoreUpdates();

		// call gamemode method to handle ghost respawn
		GameMode->start_RESPAWN_STATE();
		return;
	}

	// FRUIT NODE EATEN
	const auto EatenFruit = Cast<AFruitNode>(OtherActor);
	// eat only if mesh is visible, otherwise it has already been eaten
	if (EatenFruit && EatenFruit->Fruit_Mesh->IsVisible())
	{
		EatenFruit->Fruit_Mesh->SetVisibility(false);
		GameInstance->add_FruitNode_points();
		PacManWidget->DisplayScore(GameInstance->score);
		CheckScoreUpdates();
	}

}

void APacManPawn::Spawn_Fruit()
{
	// select random position to spawn fruit 
	// do this by exctracting two random values from FRandomStream, one for X coordinate and one for Y coordinate
	AGridBaseNode* PossibleNode = nullptr;
	FVector2D SpawnLocation(15, 13); 

	// spawn fruit under the ghost 
	Fruit_ptr = GetWorld()->SpawnActor<AFruitNode>(FruitNode, FVector((SpawnLocation.X * 100) + 50, (SpawnLocation.Y *100)+50, 6.0f), FRotationMatrix::MakeFromX(FVector(0, 0, 0)).Rotator());
	if (Fruit_ptr)
	{
		// set parameters for newly spawned fruit node
		Fruit_ptr->SetGridPosition(SpawnLocation.X, SpawnLocation.Y);
		Fruit_ptr->TileCoordinatesPosition = FVector(SpawnLocation.X, SpawnLocation.Y, Fruit_ptr->GetActorLocation().Z);
		// set timer to destroy fruit after 10 seconds (mesh visibility will be set to false)
		GetWorld()->GetTimerManager().SetTimer(Fruit_TimerHandle, this , &APacManPawn::Despawn_Fruit, Fruit_Despawn_Time, false);

	}
}

void APacManPawn::Despawn_Fruit()
{
	Fruit_ptr->Fruit_Mesh->SetVisibility(false);
}

void APacManPawn::CheckScoreUpdates()
{
	// actions that may be done upon reaching a certain score
	// CHECK IF SCORE HAS SURPASSED HIGHSCORE and eventually update it
	if (GameInstance->score > GameInstance->highscore)
	{
		GameInstance->highscore = GameInstance->score;
		PacManWidget->DisplayHighScore(GameInstance->highscore);
	}

	// extra life added at 10k points 
	if ((GameInstance->score >= 10000) && OneLifeToAdd)
	{
		if (GameInstance->lives < 5)
		{
			OneLifeToAdd = false;
			GameInstance->add_PacMan_life();
			// update lives shown on User Interface
			PacManWidget->DisplayLives(GameInstance->lives);
		}
	}
}

void APacManPawn::WriteStringToFile(FString FilePath, FString String)
{
	if (!FFileHelper::SaveStringToFile(String, *FilePath))
	{
		return;
	}
	// else everything went smoooth
}

void APacManPawn::ReloadLevel()
{
	// eventually update highscore in its txt file if current score has surpassed it
	if (GameInstance->score == GameInstance->highscore)
	{
		FString StringToWrite = FString::FromInt(GameInstance->highscore);
		WriteStringToFile(FullHighScorePath, StringToWrite);
	}
	
	if (isGameOver)
	{
		// GAME OVER
		// reset score and lives values ==> new level 
		GameInstance->lives = 3;
		GameInstance->score = 0;
		PacManWidget->DisplayScore(GameInstance->score);
		PacManWidget->DisplayLives(GameInstance->lives);
	}
	// ELSE leave current score and lives values ==> it's like another level is opened
	// and the game goes on until the player has no lives left

	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}

void APacManPawn::Activate_Collider()
{
	Collider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void APacManPawn::GhostHouseExit()
{
	// check if next ghost in line is still inside the house ==> if it is, it's time it exits the house to hunt pacman
	if (GameMode->GridPawn_references.Pinky_ptr->isInsideHouse)
	{
		GameMode->GridPawn_references.Pinky_ptr->isExitingHouse = true;
	}
	
	else if (GameMode->GridPawn_references.Inky_ptr->isInsideHouse)
	{
		GameMode->GridPawn_references.Inky_ptr->isExitingHouse = true;
	}
	
	else if (GameMode->GridPawn_references.Clyde_ptr->isInsideHouse)
	{
		GameMode->GridPawn_references.Clyde_ptr->isExitingHouse = true;
	}
	// reset timer 
	GetWorld()->GetTimerManager().ClearTimer(Ghost_Exit_TimerHandle);
	GetWorld()->GetTimerManager().SetTimer(Ghost_Exit_TimerHandle, this, &APacManPawn::GhostHouseExit, Ghost_Exit_Timer, false);
}

