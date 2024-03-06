// Fill out your copyright notice in the Description page of Project Settings.


#include "PacManWidget.h"

void UPacManWidget::NativeConstruct()
{
	Super::NativeConstruct();

	DisplayScore(0);
	DisplayLives(4);

	//get GameInstance Reference for every GridPawn
	GameInstance = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	DisplayHighScore(GameInstance->highscore);

	DisplayGameOver("");
}

void UPacManWidget::DisplayScore(int score)
{
	ScoreLabel->SetText(FText::AsNumber(score));
}

void UPacManWidget::DisplayLives(int lives)
{
	LivesLeftLabel->SetText(FText::AsNumber(lives));
}

void UPacManWidget::DisplayHighScore(int highscore)
{
	HighscoreLabel->SetText(FText::AsNumber(highscore));
}

void UPacManWidget::DisplayGameOver(FString gameOver)
{
	GameOverLabel->SetText(FText::FromString(gameOver));
}