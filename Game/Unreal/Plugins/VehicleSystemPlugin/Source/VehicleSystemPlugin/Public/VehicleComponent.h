#pragma once

#include "Components/SceneComponent.h"
#include "VehicleComponent.generated.h"

UCLASS(Abstract, Blueprintable, ClassGroup="VehicleSystem", meta=(BlueprintSpawnableComponent))
class VEHICLESYSTEMPLUGIN_API UVehicleComponent : public USceneComponent
{
	GENERATED_BODY()
};
