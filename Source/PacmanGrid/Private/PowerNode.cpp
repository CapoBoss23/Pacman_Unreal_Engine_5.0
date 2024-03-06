// Fill out your copyright notice in the Description page of Project Settings.


#include "PowerNode.h"


APowerNode::APowerNode()
{
	EEatableId = Power;
	eaten = false;

	//mesh and collider to handle point spawn adn eating
	Power_Collider = CreateDefaultSubobject<UBoxComponent>(TEXT("Power_Collider"));
	RootComponent = Power_Collider;

	Power_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Power_Mesh"));
	Power_Mesh->SetupAttachment(Power_Collider);

	PowerNode_Flashing_Timer = 0.2f;
}

bool APowerNode::is_eaten_node()
{
	return 	eaten;
}

void APowerNode::eat_node()
{
	eaten = true;
}

void APowerNode::PowerNodeNotVisible()
{
	if (!eaten)
	{
		Power_Mesh->SetVisibility(false);
		GetWorld()->GetTimerManager().SetTimer(PowerNode_Flashing_TimerHandle, this, &APowerNode::PowerNodeVisible, PowerNode_Flashing_Timer, false);
	}
	else
	{
		// power node was eaten, should stay invisible until the end of the level
		Power_Mesh->SetVisibility(false);
	}
}

void APowerNode::PowerNodeVisible()
{
	if (!eaten)
	{
		Power_Mesh->SetVisibility(true);
		GetWorld()->GetTimerManager().SetTimer(PowerNode_Flashing_TimerHandle, this, &APowerNode::PowerNodeNotVisible, PowerNode_Flashing_Timer, false);
	}
	else
	{
		// power node was eaten, should stay invisible until the end of the level
		Power_Mesh->SetVisibility(false);
	}
}
