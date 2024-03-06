// Fill out your copyright notice in the Description page of Project Settings.


#include "Clyde.h"
#include "TestGridGameMode.h"

AGridBaseNode* AClyde::GetPlayerRelativeTarget()
{
	return Super::GetPlayerRelativeTarget();
}

AClyde::AClyde()
{
	CurrentGridCoords = FVector2D(18, 15);
	Chase_Target = nullptr;
}

void AClyde::BeginPlay()
{
	AGhostPawn::BeginPlay();

	//MOVED TO TEstGridBaseMode for CENTRALIZED STATE TRANSITION HANDLING  -> TOO MUCH FUNCTION CALL
	Clyde_Scatter_TargetNode_position = FVector2D(1, 0);

	Clyde_Scatter_TargetNode_ptr = *GridGenTMap.Find(Clyde_Scatter_TargetNode_position);
}

void AClyde::SetGhostTarget()
{
	// Target is used in Scatter(= scatter target node)/ Chase (= PacMan)
	// IS DIFFERENT FROM TargetNode (node where decided to move to)
	AGridBaseNode* Target = nullptr;

	// node where it is saved node that ghost has decided to move to (near node)
	AGridBaseNode* PossibleNode = nullptr;

	// CODE FOR GHOST HOUSE
	if (isInsideHouse && !isExitingHouse) {
		// set last valid input direction to go only up or down ==> should be moved to game mode 
		// when ghost is spawned
		set_to_GhostHouseMovementSpeed();
		if (LastValidInputDirection == FVector(1, 0, 0))
		{
			SetTargetNode(*GridGenTMap.Find(FVector2D(19, 15)));
		}
		else if(LastValidInputDirection == FVector(-1, 0, 0))
		{
			SetTargetNode(*GridGenTMap.Find(FVector2D(17, 15)));
		}
		
		if (CurrentGridCoords == FVector2D(19, 15))
		{
			// need to revert direction and set new target, which will be lower-right node
			LastValidInputDirection = FVector(-1, 0, 0);
		}
		
		if (CurrentGridCoords == FVector2D(17, 15))
		{
			// need to revert direction and set new target, which will be lower-right node
			LastValidInputDirection = FVector(1, 0, 0);
		}
		return;
	}

	// check if ghost NEEDS TO EXIT THE HOUSE
	if (isExitingHouse)
	{	
		if (GetCurrentGridCoords() == FVector2D(19, 13))
		{
			// NOW THE GHOST HAS TO PASS THROUGH THE GATE, just set node outside as target node
			SetTargetNode(*GridGenTMap.Find(FVector2D(21, 13)));
		}
		// check last valid input direction, if ghost was going down then we need to revert it
		else if (LastValidInputDirection == FVector(-1, 0, 0))
		{
			SetTargetNode(*GridGenTMap.Find(FVector2D(19, 15)));
			revert_Ghost_Direction();
		}
		// check if a certain position has been reached
		else if (CurrentGridCoords == FVector2D(19, 15))
		{
			// then make ghost go towards gate
			SetTargetNode(*GridGenTMap.Find(FVector2D(19, 13)));
			LastValidInputDirection == FVector(0, -1, 0);
		}
		else if (LastValidInputDirection == FVector(1, 0, 0))
		{
			SetTargetNode(*GridGenTMap.Find(FVector2D(19, 15)));
		}
		return;
	}

	// GHOST HAS TO RETURN TO GHOST HOUSE WHEN EATEN in order to respawn
	if (eaten)
	{
		if (!isEnteringHouse)
		{
			// GHOST WAS EATEN BY PACMAN and hasn't reached the house gate yet ==> NEEDS TO MOVE TOWARDS RESPAWN POSITION
			Target = *GridGenTMap.Find(FVector2D(21, 13));
			PossibleNode = TheGridGen->GetClosestNodeFromMyCoordsToTargetCoords(this->GetLastNodeCoords(), Target->GetGridPosition(), -(this->GetLastValidDirection()));
			if (CurrentGridCoords == Target->GetGridPosition())
			{
				// RESPAWN HAS BEEN REACHED, now the ghost will resume previous state thanks to GameMode
				// ghost is in front of the gate, now it has to move through it and enter the ghost house
				isEnteringHouse = true;
			}
			TargetNode = PossibleNode;
			// save PossibleNode also in this variable to avoid crash in following instrucions
			NextNode = PossibleNode;
			// CHECK ADDED TO AVOID CRASHES SetFrightTarget() and OnNodeReached()
			// AVOID UPDATING LastValidInputDirection to (0,0,0) as it could cause errors 
			// e.g. IN SET FRIGHT TARGET WITH (0,0,0) transitions on teleport would make the game crash
			if (PossibleNode != LastNode)
			{
				LastValidInputDirection = TheGridGen->GetThreeDOfTwoDVector(PossibleNode->GetGridPosition() - this->GetLastNodeCoords());
			}
		}

		else
		{
			if (CurrentGridCoords == FVector2D(21, 13))
			{
				SetTargetNode(*GridGenTMap.Find(FVector2D(19, 13)));
			}

			else if (CurrentGridCoords == FVector2D(19, 13))
			{
				// ghost needs to turn right (with respect to our point of view)
				SetTargetNode(*GridGenTMap.Find(FVector2D(19, 15)));
				SetLastValidDirection(FVector(0, 1, 0)); 
			}
			else if (CurrentGridCoords == FVector2D(19, 15))
			{
				// RESPAWN HASE BEEN REACHED, now the ghost will resume previous state by synching with GameMode
				isEnteringHouse = false;
				isInsideHouse = true;
				RespawnReached = true;
				eaten = false;
				GameMode->end_RESPAWN_STATE();
				SetTargetNode(*GridGenTMap.Find(FVector2D(17, 15)));
				SetLastValidDirection(FVector(-1, 0, 0));
			}
		}
		return;
	}


	else if (Ghost_State != ECurrentState::Freightened && !isInsideHouse)
	{
		// SCATTER / CHASE STATE -> 
		// SAME LOGIC IN MATHING AIRLINE DISTANCE FROM TARGET BUT DIFFERENT TARGET

		if ((Ghost_State == ECurrentState::Chase)) {
			////DONE in CHASE STATE PUT A SWITCH TO SELECT TARGET NODE SWITICHING TO STATE

			// get current chase target through the appointed function
			set_CHASE_target();
			// update Target, which will be used to compute the node where Pinky will move
			Target = Chase_Target;

			if (!Target)
			{
				Target = GetPlayer()->GetLastNode();
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("GetLastNode")));
			}
		}
		else
		{	//SCATTER, added condition for blinky (when it becomes elroy it won't enter is scatter mode)

			Target = Clyde_Scatter_TargetNode_ptr;
		}

		//check CONCURRENT state change to scatter when ghost HAS JUST ARRIVED in (18,27) with Dir = (0, 1, 0) (RIGHT)
		if ((CurrentGridCoords == TeleportStaticArray[1]->GetGridPosition()) && LastValidInputDirection == FVector(0, 1, 0))
		{
			PossibleNode = *(GridGenTMap.Find(FVector2D(18, 27)));
		} //check CONCURRENT state change to scatter when ghost HAS JUST ARRIVED in (18,0) with Dir = (0, -1, 0) (LEFT)
		else if ((CurrentGridCoords == TeleportStaticArray[0]->GetGridPosition()) && LastValidInputDirection == FVector(0, -1, 0))
		{
			PossibleNode = *(GridGenTMap.Find(FVector2D(18, 0)));
		}
		else {
			PossibleNode = TheGridGen->GetClosestNodeFromMyCoordsToTargetCoords(this->GetLastNodeCoords(), Target->GetGridPosition(), -(this->GetLastValidDirection()));
		}
		//----------------------------------------------------------------------------------------------------
	}
	else if(!isInsideHouse)
	{
		//Freightened STATE -> USE RANDOM GENERATOR

		// ATTENTION!!!!!
		// a) Target (NOT TargetNode, but a variable decalred in this function) 
		// IS USELESS in this conditiona branch -> will be NULL like initialization WITH NO HARM
		// 
		// b) TARGET_NODE has been set with set_FRIGHTENED_target() called BEFORE
		//  this function in FRIGHT TRANSITION in to_FRIGHTENED_STATE() triggered by GameMode
		//	FOR FIRST TargetNode set AFTER STATE CHANGE IT' S ENOUGH BUT AFTER THAT 
		//	I NEED TO REASSIGN IT in next target EVERYTIME THE CHECK IS MADE
		// 
		// c) !!!!IN THIS CONDITION BRANCH set POSSIBLE_NODE = TARGET_NODE

		set_FRIGHTENED_target();

		PossibleNode = TargetNode;
	}


	//COMMON VARIABLE UPDATE NOT DEPENDING ON WHICH STATE THE GHOST IS IN
	if (PossibleNode)
	{
		TargetNode = PossibleNode;
		// save PossibleNode also in this variable to avoid crash in following instrucions
		NextNode = PossibleNode;
		
		// CHECK ADDED TO AVOID CRASHES SetFrightTarget() and OnNodeReached()
		// AVOID UPDATING LastValidInputDirection to (0,0,0) as it cuold cause errors 
		// e.g. IN SET FRIGHT TARGET WITH (0,0,0) transitions on teleport would make crash the game
		if (PossibleNode != LastNode)
		{
			LastValidInputDirection = TheGridGen->GetThreeDOfTwoDVector(PossibleNode->GetGridPosition() - this->GetLastNodeCoords());
		}
	}
}

void AClyde::set_SCATTER_target()
{
	// update target node with the node outside the map, which is specific of blinky
	TargetNode = Clyde_Scatter_TargetNode_ptr;
}

void AClyde::set_CHASE_target()
{
	// clyde chases pacman like BLinky if the distance between them is lees less than 8 tiles
	// otherwise it's target is Clyde_Scatter_TargetNode_ptr
	// 
	// get pacman location and convert it to FVector2D, then sum the correct amount
	FVector2D CurrentPacmanCoords = GetPlayerRelativeTarget()->GetGridPosition();
	// get Clyde location
	FVector2D CurrentClydeCoords = GameMode->GridPawn_references.Clyde_ptr->GetCurrentGridCoords();

	// compute euclidean distance between the two pawns
	double distance = FVector::DistXY(FVector3d(CurrentPacmanCoords.X, CurrentPacmanCoords.Y, 6), FVector3d(CurrentClydeCoords.X, CurrentClydeCoords.Y, 6));

	if (distance < 8)
	{
		// Clyde chase target is Clyde_Scatter_TargetNode_ptr
		Chase_Target = Clyde_Scatter_TargetNode_ptr;
	}
	else
	{
		// Clyde chases pacman like Blinky
		Chase_Target = GetPlayerRelativeTarget();
	}

	if (Chase_Target != nullptr) {
		TargetNode = Chase_Target;
	}
}

FVector2D AClyde::GetSpawnLocation()
{
	return FVector2D(18, 15);
}
