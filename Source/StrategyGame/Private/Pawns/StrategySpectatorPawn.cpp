// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.


#include "StrategyGame.h"
#include "StrategySpectatorPawn.h"
#include "StrategyCameraComponent.h"
#include "StrategySpectatorPawnMovement.h"

AStrategySpectatorPawn::AStrategySpectatorPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UStrategySpectatorPawnMovement>(Super::MovementComponentName))
{
	GetCollisionComponent()->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	bAddDefaultMovementBindings = false;
	StrategyCameraComponent = CreateDefaultSubobject<UStrategyCameraComponent>(TEXT("StrategyCameraComponent"));
}

void AStrategySpectatorPawn::OnMouseScrollUp()
{
	StrategyCameraComponent->OnZoomIn();
}

void AStrategySpectatorPawn::OnMouseScrollDown()
{
	StrategyCameraComponent->OnZoomOut();
}


void AStrategySpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	UE_LOG(LogGame, Error, TEXT("%s SpectatorPawn SetupInputComponent"), *PlayerInputComponent->GetOwner()->GetName());
	check(PlayerInputComponent);
	
	PlayerInputComponent->BindAction("ZoomOut", IE_Pressed, this, &AStrategySpectatorPawn::OnMouseScrollUp);
	PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &AStrategySpectatorPawn::OnMouseScrollDown);

	PlayerInputComponent->BindAxis("MoveForward", this, &AStrategySpectatorPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AStrategySpectatorPawn::MoveRight);
	UE_LOG(LogGame, Error, TEXT("SpectatorPawn SetupInputComponent!"));
}


void AStrategySpectatorPawn::MoveForward(float Val)
{
	StrategyCameraComponent->MoveForward(Val);
	//if (Val != 0.f)
	//{
	//	FVector TankPos = this->GetActorLocation();
	//	UE_LOG(LogGame, Error, TEXT("%s SpectatorPawn MoveForward Location %s!"), *GetName(), *TankPos.ToString());
	//}
	
}


void AStrategySpectatorPawn::MoveRight(float Val)
{
	StrategyCameraComponent->MoveRight(Val);
	//if (Val != 0.f)
	//{
	//	FVector TankPos = this->GetActorLocation();
	//	UE_LOG(LogGame, Error, TEXT("%s SpectatorPawn MoveRight Location %s!"), *GetName(), *TankPos.ToString());
	//}
}

UStrategyCameraComponent* AStrategySpectatorPawn::GetStrategyCameraComponent()
{
	check( StrategyCameraComponent != NULL );
	return StrategyCameraComponent;
}

