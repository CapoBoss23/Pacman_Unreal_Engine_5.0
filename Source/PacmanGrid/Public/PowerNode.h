// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridBaseNode.h"
#include "Components/BoxComponent.h"
#include "PowerNode.generated.h"

/**
 * 
 */
UCLASS()
class PACMANGRID_API APowerNode : public AGridBaseNode
{
	GENERATED_BODY()

//----------------------------------------------------
protected:
	//used to check if the has ALREADY BEEN EATEN
	UPROPERTY(VisibleAnywhere)
		bool eaten;

public:
	//ATTRIBUTES
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* Power_Mesh;

	UPROPERTY(EditAnywhere)
		UBoxComponent* Power_Collider;

	//METHODS
	APowerNode();

	// checks if the node has ALREADY been eaten
	bool is_eaten_node();

	// sets eaten attribute to TRUE when you eat it
	void eat_node();

	//-----------------------------------------------------
	// POWER NODE FLASHING
	FTimerHandle PowerNode_Flashing_TimerHandle;
	float PowerNode_Flashing_Timer;
	void PowerNodeNotVisible();
	void PowerNodeVisible();
};
