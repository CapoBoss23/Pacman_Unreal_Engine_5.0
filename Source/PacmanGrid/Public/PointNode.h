// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridBaseNode.h"
#include "Components/BoxComponent.h"
#include "PointNode.generated.h"

/**
 * 
 */
UCLASS()
class PACMANGRID_API APointNode : public AGridBaseNode
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
		UStaticMeshComponent* Point_Mesh;

	UPROPERTY(EditAnywhere)
		UBoxComponent* Point_Collider;

	//METHODS
	APointNode();

	// checks if the node has ALREADY been eaten
	bool is_eaten_node();
	
	// sets eaten attribute to TRUE when you eat it
	void eat_node();
};
