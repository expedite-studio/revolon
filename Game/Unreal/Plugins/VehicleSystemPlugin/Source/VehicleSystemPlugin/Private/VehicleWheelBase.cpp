// Copyright 2019-2023 Seven47 Software. All Rights Reserved.
// Unauthorized copying of this file, via any medium is strictly prohibited

#include "VehicleWheelBase.h"

#include "VehicleSystemFunctions.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"

UVehicleWheelBase::UVehicleWheelBase()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVehicleWheelBase::BeginPlay()
{
	Super::BeginPlay();

	// Initialize wheel config
	UpdateLocalTransformCache();
	WheelConfig.CalculateConstants();
	UpdateWheelRadius();
}

void UVehicleWheelBase::UpdateWheelRadius()
{
	if( !IsValid(WheelMeshComponent) ) return;
	
	if( WheelConfig.AutoWheelRadius )
	{
		USphereComponent* WheelSphere = Cast<USphereComponent>(WheelMeshComponent);
		if( IsValid(WheelSphere) ) return;
		
		WheelConfig.WheelRadius = UVehicleSystemFunctions::GetMeshRadius(WheelMeshComponent);
	}
}

void UVehicleWheelBase::UpdateLocalTransformCache()
{
	UPrimitiveComponent* VehicleMesh = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
	if( !IsValid(VehicleMesh) )
		return;
	
	WheelConfig.WheelLocalTransform = GetComponentTransform().GetRelativeTransform(VehicleMesh->GetBodyInstance()->GetUnrealWorldTransform());
}

void UVehicleWheelBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if( WheelConfig.WheelMode != EWheelMode::Raycast )
		return;

	if( !IsValid(WheelMeshComponent) || !GetIsAttached() || !GetIsSimulatingSuspension() )
		return;

	WheelConfig.isLocked = isLocked; // Copy isLocked into wheel config every frame, not ideal should be changed later

	if( GetHasContact() )
	{
		CurAngVel = WheelData.AngularVelocity;
	}
	else
	{
		if( isLocked )
		{
			CurAngVel = 0.0f;
		}
		else
		{
			bool isAccelerating = (FMath::Abs(TargetAngVel) > FMath::Abs(CurAngVel));
			float DriveInterpSpeed = isAccelerating ? 2.0f : 1.0f;
			float FinalTargetAV = WheelConfig.IsDrivingWheel ? TargetAngVel : 0.0f;
			float ChangeSpeed = WheelConfig.IsDrivingWheel ? DriveInterpSpeed : 0.2f;
			CurAngVel = FMath::FInterpTo(CurAngVel, FinalTargetAV, DeltaTime, ChangeSpeed);
		}
	}
	
	constexpr float RadsToDegreesPerSecond = (180.0f / PI);
	const float SpringStart = WheelConfig.SpringLength*0.5f;
	
	// Add delta rotation
	WheelRotation = UKismetMathLibrary::ComposeRotators(WheelRotation, FRotator(CurAngVel * RadsToDegreesPerSecond * -1.0f * DeltaTime, 0.0f, 0.0f));
	
	FVector NewLoc = FVector(0,0, FMath::Min(SpringStart + (WheelData.CurrentSpringLength * -1.0f), SpringStart) );
	FRotator NewRot = UKismetMathLibrary::ComposeRotators(WheelRotation, FRotator(0.0f, GetSteeringAngle(), 0.0f));
	WheelMeshComponent->SetRelativeLocationAndRotation(NewLoc, NewRot);
}

void UVehicleWheelBase::SetWheelMode_Implementation(EWheelMode NewMode)
{
	if( !IsValid(WheelMeshComponent) )
		return;
	
	WheelConfig.WheelMode = NewMode;
	ResetWheelCollisions();
}

void UVehicleWheelBase::ResetWheelCollisions()
{
	if( WheelConfig.WheelMode == EWheelMode::Physics )
	{
		WheelMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	else // Raycast
	{
		WheelMeshComponent->SetSimulatePhysics(false);
		WheelMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WheelMeshComponent->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}

float UVehicleWheelBase::GetWheelAngVelInRadians()
{
	if(GetWheelMode() == EWheelMode::Physics)
	{
		if( !IsValid(WheelMeshComponent) )
			return 0.0f;

		return WheelMeshComponent->GetPhysicsAngularVelocityInRadians().Length();
	}
	return WheelData.AngularVelocity; // Return from raycast data
}

FVector UVehicleWheelBase::GetWheelVelocity(bool Local)
{
	EWheelMode CurWheelMode = GetWheelMode();
	FTransform WheelWorldTransform = WheelMeshComponent->GetComponentTransform();;
	FVector WheelVelocityWorld;
	
	if( CurWheelMode == EWheelMode::Physics )
	{
		WheelVelocityWorld = WheelMeshComponent->GetPhysicsLinearVelocity();
	}
	else
	{
		UPrimitiveComponent* VehicleMesh = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
		if( !IsValid(VehicleMesh) )
			return FVector::ZeroVector;
		
		WheelVelocityWorld = VehicleMesh->GetPhysicsLinearVelocityAtPoint( WheelWorldTransform.GetLocation() );
	}

	if( Local )
	{
		return WheelWorldTransform.Inverse().TransformVectorNoScale(WheelVelocityWorld);
	}

	return WheelVelocityWorld;
}

