// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PACMANGRID_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()
protected:
	//Attributes
	// level number used to handle stats management with appropriate local
	 
	// structure in gamefield TO READ /SAVE (PacMan/  ghosts speeds / time ecc)
	UPROPERTY(EditAnywhere)
	int level;

	//----------------------------------------------------------------------------------------------
	//score values
	// During the game, everything that PacMan manages to eat is counted in the form of points, which
	// allow you to get an extra life once you reach 10,000 (according to the default configuration).
	// The balls scattered along the maze are worth 10 points each (there are 240 in the whole maze, for a
	// total of 2,400 points) while the power pills are worth 50 (for a total of 200 points).
	//
	// You can also get extra points by eating ghosts once they have been made vulnerable by one of the power pills:
	// in this case you get 200, 400, 800 and 1,600 points (for a total of 3,000 points) by swallowing up the ghosts in sequence
	// (200 points with the first ghost, 400 with the second and so on).There is also another possibility for increasing your
	// score: during the execution of each level, an icon appears twice in the center of the labyrinth, in the
	// most cases representing a fruit. If the player is skilled enough to retrieve it before
	// disappears, the score will increase.

	//SAME FOR EACH LEVEL
	UPROPERTY(EditAnywhere)
		int PointNode_points;

	UPROPERTY(EditAnywhere)

		int PowerNode_points;

	UPROPERTY(EditAnywhere)

		int GhostEat_points;

	UPROPERTY(EditAnywhere)

		int FruitNode_points;

	//----------------------------------------------------------------------------------------------
	virtual void Init() override;

public:
	//-----------------------------------------------
	// HIGHSCORE UPDATE 
	// used by PacManPawn to get file position references when needed

	//relative localtion for PROJECT FOLDER
	FString RelativePath;

	//location of HighScore.txt file used to save this data
	FString FullHighScorePath;
	//----------------------------------------------

	// total score gained by Pacman
	UPROPERTY(EditAnywhere)
	int score;

	//starting pacman lives number
	UPROPERTY(EditAnywhere)
	int lives;
	
	// highscore
	UPROPERTY(EditAnywhere)
	int highscore;

	// function to read highscore value from textfile
	UFUNCTION(BlueprintCallable)
		static FString ReadFromFile(FString FilePath, bool& bOutSuccess);

	bool ReadSuccessfull;

	//--------------------------------------------------------
	//score updating

	void add_PointNode_points();

	void add_PowerNode_points();

	void add_GhostEat_points();

	void add_FruitNode_points();
	
	//--------------------------------------------------------
	//lives updating
	void add_PacMan_life();

	void remove_PacMan_life();
};
