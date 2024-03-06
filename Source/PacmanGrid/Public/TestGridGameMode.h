// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridGenerator.h"
#include "GridPawn.h"
#include "PacManPawn.h"
#include "Blinky.h"
#include "Pinky.h"
#include "Inky.h"
#include "Clyde.h"
#include "GhostPawn.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameMode.h"
#include "TestGridGameMode.generated.h"

//USTRUCT FOR GAEMEODE REFERENCE TO 5 PAWN
// https://docs.unrealengine.com/4.26/en-US/ProgrammingAndScripting/GameplayArchitecture/Structs/UsingStructs/
USTRUCT(BlueprintType)
struct FGridPawn_ptr_Struct
{
		GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
		APacManPawn* PacMan_ptr;

	UPROPERTY(VisibleAnywhere)
		ABlinky* Blinky_ptr;

	UPROPERTY(BlueprintReadOnly)
		APinky* Pinky_ptr;

	UPROPERTY(BlueprintReadOnly)
		AInky* Inky_ptr;

	UPROPERTY(BlueprintReadOnly)
		AClyde* Clyde_ptr;
};

//------------------------------------------------------------------------------------------
// in this way I CAN USE THIS DATA DEFINITION TO KEEP TRACK OF STATES IN GHOSTS 
// TO SWITCH IN virtual void AGhostPawn::SetGhostTarget() TO USE THE RIGHT FUNCTION
// TO DECIDE THE TARGET ACCORDING TO ACTUAL STATE
UENUM()
enum ECurrentState
{
	Chase		UMETA(DisplayName = "Chase"),
	Scatter		UMETA(DisplayName = "Scatter"),
	Freightened   UMETA(DisplayName = "Freightened"),
};

//------------------------------------------------------------------------------------------

UCLASS()
class PACMANGRID_API ATestGridGameMode : public AGameMode
{
	GENERATED_BODY()

protected:

	//timer used for PACMAN POWER EFFECT and GHOSTS and FRIGHT STATE
	FTimerHandle PowerNode_Effect_TimerHandle;
	 
	//time interval to set fright time
	float PowerNode_Effect_Time;

public:
	//UStruct of Specified grid pawn
	//UStruct with 5 pointer to PACMAN and Ghosts, different types for everyone
	// 
	//		Pacman_ptr 
	//		Blinky_ptr (Red) 
	//		Pinky_ptr (Pink)
	//		Inky_ptr (Cyan) 
	//		Clyde_ptr (Orange)	
	//	UGameplayStatics::GetActorOfClass  was used ->  This is a slow operation, use with caution e.g. do not use every frame. 
	//	It is called in BEGIN PLAY function
	//	https://docs.unrealengine.com/4.27/en-US/API/Runtime/Engine/Kismet/UGameplayStatics/GetActorOfClass/
	//	!!! ATTENTION !!! must cast object type from AActor to  SPECIFICPAWN pointer (e.g. APacManPawn*)
	FGridPawn_ptr_Struct GridPawn_references;

	//-------------------------------------------
	//REFERENCE FOR BLUEPRINT TO FILL GridPawnArray
	UPROPERTY(EditDefaultsOnly, Category = "GridPawnArray")
		TSubclassOf<APacManPawn> PacManPawn;

	UPROPERTY(EditDefaultsOnly, Category = "GridPawnArray")
		TSubclassOf<ABlinky> BlinkyPawn;

	UPROPERTY(EditDefaultsOnly, Category = "GridPawnArray")
		TSubclassOf<APinky> PinkyPawn;

	UPROPERTY(EditDefaultsOnly, Category = "GridPawnArray")
		TSubclassOf<AInky> InkyPawn;

	UPROPERTY(EditDefaultsOnly, Category = "GridPawnArray")
		TSubclassOf<AClyde> ClydePawn;
	//-------------------------------------------
	// TSubclassOf is a template class that provides UClass type safety.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AGridGenerator> GridGeneratorClass;

	// reference to a GameField object
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AGridGenerator* GField;

	ATestGridGameMode();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//---------------------------------------------------------------
	// manage FRIGHT STATE/POWERNODE EFFECT for GHOSTS AND PACMAN

	void start_FRIGHT_STATE();

	void end_FRIGHT_STATE();
	//---------------------------------------------------------------
	// manage SCATTER STATE only for ghosts
	// ghosts start in SCATTER STATE
	UPROPERTY(EditAnywhere)
		TEnumAsByte<ECurrentState> ECurrentState = ECurrentState::Scatter;

	// also need information on which state I was in before entering freightened state
	// because I need to resume the relative timer once it ends
	// if the value of property is set to 0 ==> scatter state
	// if it's set to 1 ==> chase state
	UPROPERTY(EditAnywhere)
		int LastState;

	void start_SCATTER_STATE();

	// counter used to understand how many times ghosts have entered in scatter state
	// they will start the game in scatter state and will enter it a total of 4 times
	UPROPERTY(EditAnywhere)
		int Scatter_State_Counter;

	// IMPORTANT: 1st and 2nd time it lasts 7 secs, 3rd and 4th time it lasts 5 secs
	float Scatter_State12_Time;
	float Scatter_State34_Time;

	// handle for scatter state timer
	FTimerHandle Scatter_State_TimerHandle;

	//---------------------------------------------------------------
	 // manage CHASE STATE only for ghosts
	void start_CHASE_STATE();

	// chase state lasts 20 seconds fot the first 3 times then goes on indefinitely
	float Chase_State_Time;

	// handle for chase state timer
	FTimerHandle Chase_State_TimerHandle;

	//---------------------------------------------------------------
	// manage RESPAWN STATE only for single ghost
	void start_RESPAWN_STATE();
	void end_RESPAWN_STATE();
	// timer to check every second if respawn position has been reached
	float Respawn_Check_Timer;
	// handle for the respawn check timer
	FTimerHandle Respawn_Check_TimerHandle;
	// counter to check how many ghosts pacman ate during frightened state
	// will be reset to 0 at the end of frightened state
	int32 countEatenGhosts;

	//---------------------------------------------------------------
	// MANAGE CRUISE ELROY BEHAVIOUR
	void start_CRUISE_ELROY1();
	void start_CRUISE_ELROY2();
};
