// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridBaseNode.h"
#include "FruitNode.generated.h"

/**
 * 
 */
UCLASS()
class PACMANGRID_API AFruitNode : public AGridBaseNode
{
	GENERATED_BODY()
	
public:
	//ATTRIBUTES
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* Fruit_Mesh;

	UPROPERTY(EditAnywhere)
		UBoxComponent* Fruit_Collider;

	//METHODS
	AFruitNode();

};
