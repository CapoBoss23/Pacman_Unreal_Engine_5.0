// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "MyGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "PacManWIdget.generated.h"

UCLASS()
class PACMANGRID_API UPacManWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void DisplayScore(int score);
	void DisplayLives(int lives);
	void DisplayHighScore(int highscore);
	void DisplayGameOver(FString gameOver);

protected:
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* ScoreLabel;

	UPROPERTY(meta = (BindWidget))
		class UTextBlock* LivesLeftLabel;
	
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* HighscoreLabel;
	
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* GameOverLabel;

	// get reference to game instance
	UPROPERTY(VisibleAnywhere)
		UMyGameInstance* GameInstance;

	void NativeConstruct() override;

};