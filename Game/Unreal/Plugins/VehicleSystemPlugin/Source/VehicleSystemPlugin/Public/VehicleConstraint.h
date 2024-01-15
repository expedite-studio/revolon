// Copyright 2019-2022 Seven47 Software. All Rights Reserved.
// Unauthorized copying of this file, via any medium is strictly prohibited

#pragma once

#include "CoreMinimal.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "VehicleConstraint.generated.h"

/**
 * 
 */
UCLASS(ClassGroup=VehicleSystem, meta=(BlueprintSpawnableComponent), HideCategories=(Activation,"Components|Activation", Physics, Mobility), ShowCategories=("Physics|Components|PhysicsConstraint"))
class VEHICLESYSTEMPLUGIN_API UVehicleConstraint : public UPhysicsConstraintComponent
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "VehicleSystemPlugin")
	void SetLinearSoftConstraint(bool SoftConstraint, float Stiffness, float Damping);
};
