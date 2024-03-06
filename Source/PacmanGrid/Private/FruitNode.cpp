// Fill out your copyright notice in the Description page of Project Settings.


#include "FruitNode.h"

AFruitNode::AFruitNode()
{
	//mesh and collider to handle point spawn adn eating
	Fruit_Collider = CreateDefaultSubobject<UBoxComponent>(TEXT("Fruit_Collider"));
	RootComponent = Fruit_Collider;

	Fruit_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Fruit_Mesh"));
	Fruit_Mesh->SetupAttachment(Fruit_Collider);
}


