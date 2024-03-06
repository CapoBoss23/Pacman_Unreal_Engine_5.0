// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "GridBaseNode.generated.h"

UENUM()
enum EWalkableId { NotWalkable, Walkable };

UENUM()
enum EFood_Node_Type {Point, Power, Not_Eatable};

UCLASS()
class PACMANGRID_API AGridBaseNode : public AActor
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//-----------------------------------
	//used to manage point/ power node eating
	UPROPERTY(EditAnywhere)
		TEnumAsByte<EFood_Node_Type> EEatableId = Not_Eatable;
	//---------------------------------------

public:
	// Sets default values for this actor's properties
	AGridBaseNode();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
		TEnumAsByte<EWalkableId> EWalkableId = Walkable;
	//--------------------------------------------------------------
	// variable used to DECREASE GHOSTS SPEED IN TUNNELS 
	// default value = false -> ONLY  TRUE for TunnelNode
	bool isTunnel;

	// variable used to respect ghosts specification -> DIRECTION CHECK NEED TO BE MADE ONLY IN CROSS NODE
	// MOREOVER, IT' S AN EFFICIENCY BOOST 
	// default value = false -> ONLY TRUE FOR CROSSNODES
	// CROSSNODES are detected by GridGenerator during spawning process of the gamefield
	// following this principle:
	// CROSS NODE = node where we have at least 2 walkable nodes nearby (up, right, down, left)
	bool isCrossNode;

	// bool to check if node is gateNode ==> useful for ghost house
 	bool isGateNode;

	//---------------------------------------------------------------
	// tells if this tile if Point / Power /Not_Eatable
	TEnumAsByte<EFood_Node_Type> get_EEatableId();

	// set the (x, y) position
	void SetGridPosition(const double InX, const double InY);

	// get the (x, y) position
	FVector2D GetGridPosition() const;

	// (x, y) position of the tile
	UPROPERTY(EditAnywhere)
		FVector2D TileGridPosition;

	// FVector position of the tile
	UPROPERTY(EditAnywhere)
		FVector TileCoordinatesPosition;

	FVector GetTileCoordinates();

};
