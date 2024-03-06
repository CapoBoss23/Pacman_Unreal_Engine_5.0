// Fill out your copyright notice in the Description page of Project Settings.


#include "Inky.h"
#include "TestGridGameMode.h"

AGridBaseNode* AInky::GetPlayerRelativeTarget()
{
	return Super::GetPlayerRelativeTarget();
}

AInky::AInky()
{
	CurrentGridCoords = FVector2D(18, 12);
	Chase_Target = nullptr;
}

void AInky::BeginPlay()
{
	AGhostPawn::BeginPlay();
	Inky_Scatter_TargetNode_position = FVector2D(1, 27);
	Inky_Scatter_TargetNode_ptr = *GridGenTMap.Find(Inky_Scatter_TargetNode_position);
}

void AInky::SetGhostTarget()
{
	// Target is used in Scatter(= scatter target node)/ Chase (= PacMan)
	// IS DIFFERENT FROM TargetNode (node where decided to move to)
	AGridBaseNode* Target = nullptr;

	// node where it is saved node that ghost has decided to move to (near node)
	AGridBaseNode* PossibleNode = nullptr;

	// CODE FOR GHOST HOUSE
	if (isInsideHouse && !isExitingHouse) {
		// need to set upper-right node as target node
		// set last valid input direction to go only up or down ==> should be moved to game mode 
		// when ghost is spawned
		set_to_GhostHouseMovementSpeed();
		if (LastValidInputDirection == FVector(1, 0, 0))
		{
			SetTargetNode(*GridGenTMap.Find(FVector2D(19, 12)));
		}
		else if (LastValidInputDirection == FVector(-1, 0, 0))
		{
			SetTargetNode(*GridGenTMap.Find(FVector2D(17, 12)));
		}

		if (CurrentGridCoords == FVector2D(19, 12))
		{
			// need to revert direction and set new target, which will be lower-right node
			LastValidInputDirection = FVector(-1, 0, 0);
		}

		if (CurrentGridCoords == FVector2D(17, 12))
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
			// ghost has reached the node in front of the gate ==> need to make it move through the gate
		}
		// check last valid input direction, if ghost was going down then we need to revert it
		else if (LastValidInputDirection == FVector(-1, 0, 0))
		{
			SetTargetNode(*GridGenTMap.Find(FVector2D(19, 12)));
			revert_Ghost_Direction();
		}
		// check if a certain position has been reached
		else if (CurrentGridCoords == FVector2D(19, 12))
		{
			// then make ghost go towards gate
			SetTargetNode(*GridGenTMap.Find(FVector2D(19, 13)));
			LastValidInputDirection == FVector(0, 1, 0);
		}
		else if (LastValidInputDirection == FVector(1, 0, 0))
		{
			SetTargetNode(*GridGenTMap.Find(FVector2D(19, 12)));
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
				// ghost is in front of the gate, now it has to move through it and enter the ghost house
				isEnteringHouse = true;
			}
			TargetNode = PossibleNode;
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
				SetTargetNode(*GridGenTMap.Find(FVector2D(19, 12)));
				SetLastValidDirection(FVector(0, -1, 0));
			}
			else if (CurrentGridCoords == FVector2D(19, 12))
			{
				// RESPAWN HAS BEEN REACHED, now the ghost will resume previous state by synching with GameMode
				isEnteringHouse = false;
				isInsideHouse = true;
				RespawnReached = true;
				eaten = false;
				GameMode->end_RESPAWN_STATE();
				SetTargetNode(*GridGenTMap.Find(FVector2D(17, 12)));
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
			}
		}
		else
		{	
			//SCATTER, added condition for blinky (when it becomes elroy it won't enter is scatter mode)
			Target = Inky_Scatter_TargetNode_ptr;
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
	else if (!isInsideHouse)
	{
		//Freightened STATE -> USE RANDOM GENERATOR

		// ATTENZION!!!!!
		// a) Target (NOT TargetNode, but a variable decalred in this function) 
		// IS USELESS in this condition branch -> will be NULL like initialization WITH NO HARM
		// 
		// b) TARGET_NODE has been set with set_FRIGHTENED_target() called BEFORE
		//  this function in FRIGHT TRANSITION in to_FRIGHTENED_STATE() triggered by GameMode
		//	FOR FIRST TargetNode set AFTER STATE CHANGE IT' S ENOUGH BUT AFTER THAT 
		//	I NEED TO REASSIGN IT in next target EVERYTIME THE CHECK IS MADE
		// 
		// c) IN THIS CONDITION BRANCH set POSSIBLE_NODE = TARGET_NODE

		set_FRIGHTENED_target();

		PossibleNode = TargetNode;
	}


	//COMMON VARIABLE UPDATE NOT DEPENDING ON WHICH STATE THE GHOST IS IN
	if (PossibleNode)
	{
		TargetNode = PossibleNode;
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

void AInky::set_SCATTER_target()
{
	// update target node with the node outside the map, which is specific of blinky
	TargetNode = Inky_Scatter_TargetNode_ptr;
}

void AInky::set_CHASE_target()
{
	// get pacman position and last valid input direction
	// returns a FVector ==> need to select just X and Y coordinates
	FVector inputDir = GetPlayer()->GetLastValidDirection();
	// need to multiply such vector by 4
	// FVector2D vectorToSum = FVector2D(4 * inputDir.X, 4 * inputDir.Y);
	FVector2D VectorToSum = FVector2D(2 * inputDir.X, 2 * inputDir.Y);

	// if pacman direction is up, then inky will point 2 tiles up and 2 to the left
	if (inputDir == FVector(1, 0, 0))
	{
		VectorToSum = FVector2D(2, -2);
	}

	// get pacman location and convert it to FVector2D, then sum the correct amount
	FVector2D CurrentPacmanCoords= GetPlayerRelativeTarget()->GetGridPosition();
	FVector2D PacmanProjection = FVector2D(CurrentPacmanCoords.X + VectorToSum.X, CurrentPacmanCoords.Y + VectorToSum.Y);

	// now get blinky position in terms of FVector2D
	FVector2D CurrentBlinkyCoords = GameMode->GridPawn_references.Blinky_ptr->GetCurrentGridCoords();
	
	// calculate vector A - B, where A is pacman position and B is Blinky position
	// also multiply by 2 its components
	FVector2D VectorPacmanBlinky = FVector2D((PacmanProjection.X - CurrentBlinkyCoords.X) * 2, (PacmanProjection.Y - CurrentBlinkyCoords.Y) * 2);
	
	// now sum this vector to blinky position vector and get the target vector
	FVector2D PossibleCoords = FVector2D(VectorPacmanBlinky.X + CurrentBlinkyCoords.X, VectorPacmanBlinky.Y + CurrentBlinkyCoords.Y);

	// need to check if selected values are within map bounds
	// for this use CLAMP function for X and Y coords
	const float ClampedX = FMath::Clamp(PossibleCoords.X, 0, 35);
	const float ClampedY = FMath::Clamp(PossibleCoords.Y, 0, 27);

	// update chase target variable, which will be used by set_CHASE_target()
	Chase_Target = *(GridGenTMap.Find(FVector2D(ClampedX, ClampedY)));
	if (Chase_Target != nullptr) {
		TargetNode = Chase_Target;
	}
}

FVector2D AInky::GetSpawnLocation()
{
	return FVector2D(18, 12);
}
