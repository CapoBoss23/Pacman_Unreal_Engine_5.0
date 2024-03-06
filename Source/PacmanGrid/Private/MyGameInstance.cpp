// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "Kismet/KismetStringLibrary.h"

void UMyGameInstance::Init()
{
	//------------------------------------------------
	level = 1;

	score = 0;
	lives = 3;		//NB: MAX = 5
	//points 
	PointNode_points = 10;
	PowerNode_points = 50;
	FruitNode_points = 100;

	// THIS SCORE WILL BE MULTIPLIED FOR n = number of ghosts eaten in a row
	//  THAT MEANS YOU ADD 200 FOR EACH GHOST EATEN IN A TIME INTERVAL OF A TIMER -> CAN DO IT
	//  just add this call in pacman with an if that checks if ghost state = fright
	GhostEat_points = 200;

	//----------------------------------------
	// HIGHSCORE UPDATE PATH

	// relative path of project folder
	RelativePath = FPaths::ProjectDir();

	//get complete path for HighScoreUpdate
	FullHighScorePath.Append(RelativePath);
	FullHighScorePath.Append("/HighScore.txt");

	//------------------------------------------------
	// get highscore value from text file and convert it to int
	
	highscore = UKismetStringLibrary::Conv_StringToInt(ReadFromFile(FullHighScorePath, ReadSuccessfull));
}

FString UMyGameInstance::ReadFromFile(FString FilePath, bool& bOutSuccess)
{
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
	{
		bOutSuccess = false;
		return "";
	}

	FString RetString = "";
	if (!FFileHelper::LoadFileToString(RetString, *FilePath))
	{
		bOutSuccess = false;
		return "";
	}
	bOutSuccess = true;
	
	// will be casted to int
	return RetString;
}

//------------------------------------------------
//score updating
void UMyGameInstance::add_PointNode_points()
{
	score += PointNode_points;
}

void UMyGameInstance::add_PowerNode_points()
{
	score += PowerNode_points;
}

void UMyGameInstance::add_GhostEat_points()
{
	score += GhostEat_points;
}

void UMyGameInstance::add_FruitNode_points()
{
	score += FruitNode_points;
}

//--------------------------------------------------------
//lives updating
	
void UMyGameInstance::add_PacMan_life()
{
	lives += 1;
}
void UMyGameInstance::remove_PacMan_life()
{
	lives -= 1;
}


