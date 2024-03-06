// Fill out your copyright notice in the Description page of Project Settings.


#include "PointNode.h"

APointNode::APointNode()
{
	EEatableId = Point;
	eaten= false;

	//mesh and collider to handle point spawn adn eating
	Point_Collider = CreateDefaultSubobject<UBoxComponent>(TEXT("Point_Collider"));
	RootComponent = Point_Collider;

	Point_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Point_Mesh"));
	Point_Mesh->SetupAttachment(Point_Collider);
}

bool APointNode::is_eaten_node()
{
	return eaten;
}

void  APointNode::eat_node() {
	eaten = true;
}
