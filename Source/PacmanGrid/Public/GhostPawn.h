// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridPawn.h"
#include "PacManPawn.h"
#include "GridGenerator.h"
#include "Math/RandomStream.h"
#include "GhostPawn.generated.h"

UCLASS()
class PACMANGRID_API AGhostPawn : public AGridPawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGhostPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnNodeReached() override;
	//------------------------------------------------------------
	//STATE GHOSTS SPEED

	//I assume "normal" state = chase and scatter
	UPROPERTY(EditAnywhere, Category = "Movement")
		float ChaseAndScatterGhostsMovementSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement")
		float FrightGhostsMovementSpeed;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
		float GhostHouseMovementSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement")
		float RespawnGhostsMovementSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement")
		float CruiseElroyMovementSpeed;

	//overriden  because GHOSTS NEED TO SLOW DOWN IN TUNNEL
	virtual void HandleMovement() override; 
	//-------------------------------------------------------------------------------------------------

	//GHOST STATE VARIABLE 

	enum ECurrentState Ghost_State;

	enum ECurrentState Ghost_Last_State;

	bool eaten;
	//-------------------------------------------------------------------------------------------------


	// FUNCTION USED BY GAMEMODE TO SET TARGETS during STATE CHANGES
	// 
	// 1) speed changes ARE COMMON TO ALL GHOSTS -> DEFINED THEM IN GhostPawn CLASS
	// 
	// 2) target changes ARE COMMON TO ALL GHOSTS -> DEFINED THEM IN GhostPawn CLASS
	// WHILE SPECIFIC TARGET NODES ARE DEFINED IN EACH GHOST
	

	// target change function

	virtual void set_SCATTER_target();

	virtual void set_CHASE_target();

	void set_FRIGHTENED_target();

	//speed changes function

	void set_to_ChaseAndScatterGhostsMovementSpeed();

	void set_to_FrightGhostsMovementSpeed();

	void set_to_RespawnGhostMovementSpeed();
	
	void set_to_GhostHouseMovementSpeed();

	//used to revert direction during state changes
	void revert_Ghost_Direction();

public:
	//state functions to BE CALLED BY GAMEMODE TO DO SETTING IN STATE TRANSITIONS
	void to_SCATTER_STATE();

	void to_CHASE_STATE();

	void to_FRIGHTENED_STATE();

	//-----------------------------------------------------------------------------------
	// Random Generator with SAME SEED for alla ghosts 
	// USE 
	//
	// a) constructor with parameters to set generator with the chosen seed
	// FRandomStream ( int32 InSeed )
	// 
	// b) void Reset() const -> resets to SEED value when eaten/ eat pacman
	// 
	// c) GET random number in a range-> RandRange( int32 Min,int32 Max) const
	//
	struct FRandomStream Frightened_State_RandomGenerator;


	// SAME SEED for Frightened_State_RandomGenerator FOR ALL GHOSTS 
	const int32 RandomGenerator_Seed = 30;	// random number

	//random generator limits

	int32 min = 0;

	int32 max = 1000000; //10e6

	//----------------------------------------------------------------------------------
	//array used in set_FRIGHTENED_target() FOR RANDOM DECISIONS
	//
	// IT RESPECTS clock-wise preference
	// 
	// indexes and directions:
	// ------------------------
	// | U=0 | R=1| D=2 | L=3 |
	// ------------------------
	// U = (1,0,0)  up
	// R = (0,1,0) 	right
	// D = (-1,0,0)	down
	// L = (0,-1,0)	left
	TStaticArray<FVector, 4> Dir_Vector;
	//-----------------------------------------------------------------------------------
public:

	// MESHES 
	// static meshes to implement different appearance during Frightened state
	// ghosts flash 5 times in the last seconds before returning to the previous state
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* StaticMeshBlue;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* StaticMeshWhite;

	// define meshes for ghost eyes ==> composed by eye and pupil
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* StaticMeshLeftEye;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* StaticMeshRightEye;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* StaticMeshLeftPupil;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* StaticMeshRightPupil;
	//------------------------------------------------------------

private:
	UPROPERTY(VisibleAnywhere)
		class APacManPawn* Player;

	UPROPERTY(VisibleAnywhere)
		AGridPawn* GridPawn;

public:
	virtual void Tick(float DeltaTime) override;
	UFUNCTION()
		APacManPawn* GetPlayer() const;
	UFUNCTION()
		void SetSpeed(float Speed);
	UFUNCTION()
		virtual AGridBaseNode* GetPlayerRelativeTarget();
	UFUNCTION()
		virtual AGridBaseNode* GetPlayerTargetNode();

	virtual void SetGhostTarget();

	//------------------------------------------
	// function that handles frightened state
	// will be called by TestGridGameMode once Frightened state is entered
	void Fright_State_Manager();
	// functions that handle visible mesh, which can change between base, blue and white
	void Visible_Mesh_White();
	void Visible_Mesh_Blue();
	void Reset_Visible_Mesh();

	// handle for the timer that sets mesh visibility
	FTimerHandle Blue_Mesh_TimerHandle;

	// duration of the timers
	float Change_Mesh_Time;
	float Flash_Time;

	// counter needed to do precisely 6 flashes (transitions between blue and white)
	// then reset base static mesh as the only one visible 
	// do this by calling Reset_Visible_Mesh
	int Flash_Counter;

	//------------------------------------------
	// HANDLING OF PACMAN-GHOST COLLISION
	// getter and setter for eaten bool
	bool GetEaten();
	void SetEaten(bool isEaten);

	// bool to check if ghost has reached respawn position
	// will be accessed in SetGhostTarget() and in GameMode->start_RESPAWN_STATE()
	bool RespawnReached;

	// get spawn location of each ghost ==> implemented in actual child classes
	virtual FVector2D GetSpawnLocation();

	// method to respawn ghost, it needs to exit previous state 
	// and resume it once respawn operations are completed
	void RespawnGhost(FVector2D respawnPosition);

	// CRUISE ELROY
	//------------------------------------------
	// bool to check if blinky has become elroy
	bool isElroy;
	bool isElroy2;
	void setCuiseElroy1MovementSpeed();
	void setCuiseElroy2MovementSpeed();
	//------------------------------------------
	// GHOST HOUSE
	bool isInsideHouse;
	bool isExitingHouse;
	bool isEnteringHouse;
};
