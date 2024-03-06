// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridBaseNode.h"
#include "GridGenerator.h"
#include "GameFramework/Pawn.h"
#include "Components/BoxComponent.h"
#include "MyGameInstance.h"
#include "GridPawn.generated.h"

UCLASS()
class PACMANGRID_API AGridPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGridPawn();
	virtual void SetVerticalInput(float AxisValue);
	virtual void SetHorizontalInput(float AxisValue);

	UFUNCTION(BlueprintCallable)
	void SetNextNodeByDir(FVector InputDir);

	FVector GetLastValidDirection() const;

	//------------------------------------------
	//usefull for ghost movement
	UFUNCTION(BlueprintCallable)
		AGridBaseNode* GetLastNode() const;
	UFUNCTION(BlueprintCallable)
		AGridBaseNode* GetTargetNode() const;
	UFUNCTION(BlueprintCallable)
		FVector2D GetLastNodeCoords() const;
	UFUNCTION(BlueprintCallable)
		FVector2D GetTargetNodeCoords() const;
	//------------------------------------------

	void SetLastValidDirection(FVector Dir);

	void SetLastDirection(FVector Dir);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
		virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	FVector LastInputDirection;
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	FVector LastValidInputDirection;

	/*SPEED TO CHANGE IN DIFFERENT STATES WITH
	CHILDREN PAWN SPEED ATTRIBUTE*/
	UPROPERTY(EditAnywhere, Category = "Movement")
	float CurrentMovementSpeed;
	
	UPROPERTY(EditAnywhere)
	float AcceptedDistance = 4.f;

	UPROPERTY(VisibleAnywhere)
	FVector2D CurrentGridCoords;

	//--------------------------------
	UPROPERTY(VisibleAnywhere)
	class ATestGridGameMode* GameMode;

	//---------------------------------------
	UPROPERTY(VisibleAnywhere)
		UMyGameInstance* GameInstance;
	//---------------------------------------
	UPROPERTY(VisibleAnywhere)
		AGridGenerator* TheGridGen;
	//-----------------------------------------------------
	// grid generator TMAP with pointers to grid nodes
	UPROPERTY(VisibleAnywhere)
		TMap<FVector2D, AGridBaseNode*> GridGenTMap;

	// meaning of elements in TeleportStaticArray, based on index value:
	// [0] -> left to right teleport
	// [1] -> right to left teleport
	TStaticArray<AGridBaseNode*,2> TeleportStaticArray;

public:	
	// SET TO PUBLIC BECAUSE NEED TO MODIFY THIS FROM GAMEMODE
	// static mesh component for all pawns
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* StaticMesh;

protected:
	// set up collider for pawns in order to handle interactions between pacman and point/power/ghosts
	UPROPERTY(EditAnywhere)
		UBoxComponent* Collider;
	//-----------------------------------------------------
	//teleport delay data -> FOR GHOSTS
	float ghotst_teleport_delay;
	
	//-----------------------------------------------------

	UPROPERTY(VisibleAnywhere, Category = "Nodes")
	AGridBaseNode* NextNode;
	UPROPERTY(VisibleAnywhere, Category = "Nodes")
	AGridBaseNode* TargetNode;
	UPROPERTY(VisibleAnywhere, Category = "Nodes")
	AGridBaseNode* LastNode;

	//THIS 2 functions ARE OVERRIDEN FOR GhostPawn
	virtual void HandleMovement();
	virtual void OnNodeReached();
	//same for each pawn(pacman and ghosts)
	void MoveToCurrentTargetNode();


public:
	// bool used when lives are 0 and level needs to be reloaded
	// all pawns stay still for some seconds while the text GAME OVER is shown on UI
	UPROPERTY(VisibleAnywhere)
		bool CanMove;
	
	// getter methods for protected functions
	FVector2D GetCurrentGridCoords();

	//---------------------------------------------
	void SetCurrentGridCoords(FVector2D CurrentGridCoords_to_set);
	//----------------------------------------------
	void SetLastNode(AGridBaseNode* newLastNode);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// called on left mouse click (binding)
	UFUNCTION()
	virtual void OnClick();

	// changes for every pawn because each one decide hase
	// different target
	virtual void SetTargetNode(AGridBaseNode* Node);
	void SetNextNode(AGridBaseNode* Node);
	void SetNodeGeneric(const FVector Dir);

};
