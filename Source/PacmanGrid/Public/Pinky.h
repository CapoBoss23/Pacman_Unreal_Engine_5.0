// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GhostPawn.h"
#include "Pinky.generated.h"

/**
 * 
 */
UCLASS()
class PACMANGRID_API APinky : public AGhostPawn
{
	GENERATED_BODY()
		virtual AGridBaseNode* GetPlayerRelativeTarget() override;
protected:
	FVector2D Pinky_Scatter_TargetNode_position;

	AGridBaseNode* Pinky_Scatter_TargetNode_ptr;

public:
	APinky();

	void BeginPlay();


	virtual void SetGhostTarget() override;

	//------------------------------------------------------------------------------------
	// FUNCTION USED BY GAMEMODE TO SET TARGETS during STATE CHANGES
	// 
	// 1) speed changes ARE COMMON TO ALL GHOSTS -> DEFINED THEM IN GhostPawn CLASS
	// 
	// 2) target changes ARE COMMON TO ALL GHOSTS -> DEFINED THEM IN GhostPawn CLASS
	// WHILE SPECIFIC TARGET NODES ARE DEFINED IN EACH GHOST
	//
	////------------------------------------------------------------------------------------
	// target change function, overrides those declared in GhostPawn

	virtual void set_SCATTER_target() override;

	virtual void set_CHASE_target() override;

	// get spawn location 
	virtual FVector2D GetSpawnLocation() override;

	// reference to target node 
	AGridBaseNode* Chase_Target;
};
