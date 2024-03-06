// Fill out your copyright notice in the Description page of Project Settings.


#include "Blinky.h"
#include "TestGridGameMode.h"

AGridBaseNode* ABlinky::GetPlayerRelativeTarget()
{
	return Super::GetPlayerRelativeTarget();
}

ABlinky::ABlinky()
{
	CurrentGridCoords = FVector2D(21, 10);
}

void ABlinky::BeginPlay() {
	AGhostPawn::BeginPlay();
	
	//MOVED TO TEstGridBaseMode for CENTRALIZED STATE TRANSITION HANDLING  to avoid TOO MUCH FUNCTION CALL
	Blinky_Scatter_TargetNode_position = FVector2D(35, 25);

	Blinky_Scatter_TargetNode_ptr = *GridGenTMap.Find(Blinky_Scatter_TargetNode_position);
}

void ABlinky::SetGhostTarget()
{
	// COMMON VARIABLES USED IN EVERY STATE
	
	// Target is used in Scatter(= scatter target node)/ Chase (= PacMan)
	// IS DIFFERENT FROM TargetNode (node where decided to move to)
	AGridBaseNode*  Target = nullptr;

	// node where it is saved node that ghost has decided to move to (near node)
	AGridBaseNode* PossibleNode = nullptr;

	// CODE FOR GHOST HOUSE
	// 
	// check if ghost NEEDS TO EXIT THE HOUSE
	if (isExitingHouse)
	{
		// ghost needs to get outside the house ASAP
		if (GetCurrentGridCoords() == FVector2D(19, 13))
		{
			// NOW THE GHOST HAS TO PASS THROUGH THE GATE, just set node outside as target node
			SetTargetNode(*GridGenTMap.Find(FVector2D(21, 13)));
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
				// RESPAWN HASE BEEN REACHED, now the ghost will resume previous state thanks to GameMode
				// ghost is in front of the gate, now it has to move through it and enter the ghost house
				isEnteringHouse = true;
			}
			TargetNode = PossibleNode;
			// save PossibleNode here because otherwise the program will crash in following instructions.  
			// Thus, the node to go to is saved in both variables
			NextNode = PossibleNode;
			// CHECK ADDED TO AVOID CRASHES in SetFrightTarget() and OnNodeReached()
			// AVOID UPDATING LastValidInputDirection to (0,0,0) as it could cause errors 
			// e.g. IN SET FRIGHT TARGET WITH (0,0,0) during transitions on teleport, the game would crash
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
				// RESPAWN HASE BEEN REACHED, now the ghost will resume previous state by synching with GameMode
				// Blinky exits the house immediately
				isEnteringHouse = false;
				isInsideHouse = true;
				isExitingHouse = true;
				RespawnReached = true;
				eaten = false;
				GameMode->end_RESPAWN_STATE();
				SetTargetNode(*GridGenTMap.Find(FVector2D(21, 13)));
			}
		}
		return;
	}


	else if (Ghost_State != ECurrentState::Freightened)
	{
		// SCATTER / CHASE STATE -> 
		// SAME LOGIC IN MATHING AIRLINE DISTANCE FROM TARGET BUT DIFFERENT TARGET

		if ((Ghost_State == ECurrentState::Chase || isElroy)) {
			// DONE IN CHASE STATE. PUT A SWITCH TO SELECT TARGET NODE WHEN SWITICHING TO STATE
			Target = GetPlayerRelativeTarget();

			if (!Target)
			{
				Target = GetPlayer()->GetLastNode();
			}
		}
		else 
		{	//SCATTER, added condition for blinky (when it becomes elroy it won't enter is scatter mode)
			Target = Blinky_Scatter_TargetNode_ptr;
		}
		
		//check CONCURRENT state change to scatter when ghost HAS JUST ARRIVED in (18,27) with Dir = (0, 1, 0) (RIGHT)
		if ((CurrentGridCoords == TeleportStaticArray[1]->GetGridPosition()) && LastValidInputDirection == FVector(0, 1, 0))
		{
			PossibleNode = *(GridGenTMap.Find(FVector2D(18,27)));
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
	else
	{
		set_FRIGHTENED_target();
		PossibleNode = TargetNode;
	}


	//COMMON VARIABLE UPDATE NOT DEPENDING ON WHICH STATE THE GHOST IS IN
	if (PossibleNode)
	{
		TargetNode = PossibleNode;
		// save PossibleNode here because otherwise the program will crash in following instructions.  
		// Thus, the node to go to is saved in both variables
		NextNode = PossibleNode;
		
		//------------------------------------------------
		// CHECK ADDED TO AVOID CRASHES in SetFrightTarget() and OnNodeReached()
		// AVOID UPDATING LastValidInputDirection to (0,0,0) as it cuold cause errors 
		// e.g. IN SET FRIGHT TARGET WITH (0,0,0) transitions on teleport would make crash the game
		if (PossibleNode != LastNode)
		{
			LastValidInputDirection = TheGridGen->GetThreeDOfTwoDVector(PossibleNode->GetGridPosition() - this->GetLastNodeCoords());
		}
	}
}

void ABlinky::set_SCATTER_target()
{
	// update target node with the node outside the map, which is specific of blinky
	TargetNode = Blinky_Scatter_TargetNode_ptr;
}

void ABlinky::set_CHASE_target()
{
	//SPECIFICATION ASKS to get currentNode occupied by pacman -> THIS MEHOD GET LastPosition WHICH is 
	// in theory the currentNode, according to code logic
	AGridBaseNode* CHASE_Target = GetPlayerRelativeTarget();

	if (CHASE_Target != nullptr) {
		TargetNode = CHASE_Target;
	}
}

FVector2D ABlinky::GetSpawnLocation()
{
	return FVector2D(21, 13);
}


