// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridPawn.h"
#include "GridGenerator.h"
#include "GridPlayerController.h"
#include "GridBaseNode.h"
#include "PointNode.h"
#include "PowerNode.h"
#include "FruitNode.h"
#include "Blueprint/UserWidget.h"
#include "PacManWidget.h"
#include "Math/RandomStream.h"
#include "PacManPawn.generated.h"


UCLASS()
class PACMANGRID_API APacManPawn : public AGridPawn
{
	GENERATED_BODY()

public:
	APacManPawn();

	virtual void Tick(float DeltaTime) override;
	virtual void SetVerticalInput(float AxisValue) override;
	virtual void SetHorizontalInput(float AxisValue) override;
	// called on left mouse click (binding)
	virtual	void OnClick() override;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	//-----------------------------------------------

	//SPEED CHANGES OF INTERACTION WITH POINT NODES BASED ON STATES CHANGES / POINT OR POWER NODE EATEN

	//sets AGridPawn::CurrentMovementSpeed = APacManPawn::NormMovementSpeed
	UFUNCTION()
	void set_to_NormMovementSpeed();

	//sets AGridPawn::CurrentMovementSpeed = APacManPawn::FrightMovementSpeed
	UFUNCTION()
	void set_to_FrightMovementSpeed();

	//-----------------------------------------------
	//ghost state change functions

	// when pacman eats POWERNODE, GameMode will be notified. Then, GameMode will set 
	// timer and trigger pacman and ghosts to change state accordingly
	UFUNCTION()
	void start_PowerNode_effect();


	// called when PowerNode effects ends
	// gamemode sets normal speed of pacman and ghosts. 
	// GAMEMODE HANDLE TIMERS AND MODIFY SPEED OF 5 PAWNS (pacman + ghosts)

	UFUNCTION()
	void end_PowerNode_effect();
	//-----------------------------------------------

	// widget class to spawn for the HUD
	UPROPERTY(EditAnywhere)
		TSubclassOf<UUserWidget> UserHUDClass;

	// widget class to spawn for the HUD
	UPROPERTY(VisibleInstanceOnly)
		UPacManWidget* PacManWidget;

protected:
	//-----------------------------------------------
	// location of HighScore.txt file used HighScore update
	FString FullHighScorePath;
	//-----------------------------------------------
	
	// PACMAN DOES NOT CARE STATE = {Frightened, Scatter, Chase} like ghosts.
	// IT ONLY NEEDS TO KNOW WHETER GHOSTS ARE FRIGHTENED OR NOT

	bool are_ghosts_frightened;
	//-----------------------------------------------
	// boolean variable to check if a life has already been added once the score
	// exceeds 10000
	bool OneLifeToAdd;

	//-----------------------------------------------
	// (OPTIONAL) COULD GET THEM FROM A DATA STRUCTURE IN THE GAME INSTANCE
	// 
	//state speeds  TO USE TO ASSING TO CurrentMovementSpeed
	UPROPERTY(EditAnywhere, Category = "Movement")
		float NormMovementSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement")
		float FrightMovementSpeed;

	//-----------------------------------------------
	virtual void BeginPlay() override;
	virtual void HandleMovement() override;
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

public:
	// PUBLIC because ghostPawn needs this in OnBeginOverlap
	virtual void SetTargetNode(AGridBaseNode* Node) override;

	//-----------------------------------------------
	// FRUIT SPAWN
	// bool to check if first and seconds fruits have been spawned
	bool FirstFruitSpawned;
	bool SecondFruitSpawned;

	void Spawn_Fruit();
	// make fruit disappear after pacman eats it ==> set mesh visibility to false
	void Despawn_Fruit();

	// subclass to spawn fruit
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<AFruitNode> FruitNode;

	// reference to a Fruit object
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		AFruitNode* Fruit_ptr;

	// GET random number in a range-> FMath::RandRange( int32 Min,int32 Max) 
	// random generator limits
	int32 minX = 12;
	int32 maxX = 24;

	int32 minY = 6;
	int32 maxY = 21;

	// we need double the timers otherwise the second one
	// does not start if the first one has already started
	// ==> this happens when two fruits are spawned less than
	// 10 secods apart
	// ACTUALLY THIS SHOULD  BE IMPOSSIBLE SINCE PACMAN
	// CANNOT EAT 100 POINTS IN LESS THAN 10 SECONDS
	FTimerHandle Fruit_TimerHandle;
	float Fruit_Despawn_Time;

	// FUNCTION THAT HANDLES SCORE UPDATE (added because code in OnBeginOverlap
	// was too verbose)
	void CheckScoreUpdates();

	// if score has surpassed highscore than the latter needs to
	// be updated in the txt file dedicated to it
	UFUNCTION(BlueprintCallable)
		static void WriteStringToFile(FString FilePath, FString String);

	FTimerHandle Reload_Level_TimerHandle;
	float Reload_Level_Timer;
	void ReloadLevel();
	bool isGameOver;

	// counter for PointNodes and PowerNodes ==> once they reach
	// a certain value ANOTHER LEVEL IS OPENED 
	int PointNodeCounter;
	int PowerNodeCounter;
	//---------------------------------------------------------------
	// SIMULTANEOUS EVENT HANDLING (acts on OnBeginOverlap)
	// we need to deactivate pacman collider for a fraction of a second
	// and reactivate it afterwards
	FTimerHandle Collision_Reactivation_TimerHandle;
	float Collision_Reactivation_Timer;
	void Activate_Collider();

	//---------------------------------------------------------------
	// GHOST HOUSE MECHANICS
	// 
	// COUNTER used for ghost house exit. Will be increased every time
	// pacman eats a point node
	int GhostPointCounter;
	// bool to activate SECOND COUNTER for ghost house exit
	// de facto counter used will be the same, but will trigger al lower values
	bool secondCounterActive;
	// timer that acts as second trigger for ghost house exit
	// becomes crucial once few point nodes remain on the map
	// this timer will be reset every time pacman eates a pointNode and is eaten by a ghost, lasts 4 seconds
	// if it expires, next ghost in order of precedence will exit the house
	FTimerHandle Ghost_Exit_TimerHandle;
	float Ghost_Exit_Timer;
	// function that tells next ghost in line to exit the house
	// upon timer expiration
	void GhostHouseExit();
};
