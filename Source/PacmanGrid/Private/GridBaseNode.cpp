// Fill out your copyright notice in the Description page of Project Settings.


#include "GridBaseNode.h"

// Sets default values
AGridBaseNode::AGridBaseNode()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//used by GHOSTS to respect specifications
	isTunnel = false;
		
	//used by ghosts to respect specifications
	isCrossNode = false;

	isGateNode = false;
}

// Called when the game starts or when spawned
void AGridBaseNode::BeginPlay()
{
	AActor::BeginPlay();
}

// Called every frame
void AGridBaseNode::Tick(float DeltaTime)
{
	AActor::Tick(DeltaTime);
}

TEnumAsByte<EFood_Node_Type> AGridBaseNode::get_EEatableId()
{
	return EEatableId;
}

void AGridBaseNode::SetGridPosition(const double InX, const double InY)
{
	TileGridPosition.Set(InX, InY);
}

FVector2D AGridBaseNode::GetGridPosition() const
{
	return TileGridPosition;
}

FVector AGridBaseNode::GetTileCoordinates()
{
	return TileCoordinatesPosition;
}


