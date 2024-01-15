// Copyright 2019-2022 Seven47 Software. All Rights Reserved.
// Unauthorized copying of this file, via any medium is strictly prohibited

#include "VehicleSystemFunctions.h"
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Interfaces/IPluginManager.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"

UVehicleSystemFunctions::UVehicleSystemFunctions(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

FString UVehicleSystemFunctions::GetPluginVersion()
{
	FString PluginName = "VehicleSystemPlugin";
 
	IPluginManager& PluginManager = IPluginManager::Get();
	TArray<TSharedRef<IPlugin>> Plugins = PluginManager.GetDiscoveredPlugins();
	for(const TSharedRef<IPlugin>& Plugin: Plugins)
	{
		if (Plugin->GetName() == PluginName)
		{
			const FPluginDescriptor& Descriptor = Plugin->GetDescriptor();
			FString Version = Descriptor.VersionName;
			return Version;
		}
	}
	return "Error";
}

// This is necessary because the function provided by the engine doesn't include the bone name
void UVehicleSystemFunctions::SetLinearDamping(UPrimitiveComponent* target, float InDamping, FName BoneName)
{
	if(target)
	{
		FBodyInstance* BI = target->GetBodyInstance(BoneName);
		if (BI)
		{
			BI->LinearDamping = InDamping;
			BI->UpdateDampingProperties();
		}
	}
}

// This is necessary because the function provided by the engine doesn't include the bone name
void UVehicleSystemFunctions::SetAngularDamping(UPrimitiveComponent* target, float InDamping, FName BoneName)
{
	if(target)
	{
		FBodyInstance* BI = target->GetBodyInstance(BoneName);
		if (BI)
		{
			BI->AngularDamping = InDamping;
			BI->UpdateDampingProperties();
		}
	}
}

float UVehicleSystemFunctions::GetMeshDiameter(UPrimitiveComponent* target, FName BoneName)
{
	return GetMeshRadius(target, BoneName) * 2;
}

float UVehicleSystemFunctions::GetMeshRadius(UPrimitiveComponent* target, FName BoneName)
{
	if (target)
	{
		UStaticMeshComponent* targetStatic = Cast<UStaticMeshComponent>(target);
		if (targetStatic)
		{
			FBoxSphereBounds MyBounds = targetStatic->GetStaticMesh()->GetBounds();
			return MyBounds.BoxExtent.Z * targetStatic->GetComponentScale().Z;
		}

		FBodyInstance* BI = target->GetBodyInstance(BoneName);
		if (BI)
		{
			FBoxSphereBounds MyBounds = BI->GetBodyBounds();
			return MyBounds.BoxExtent.Z;
		}

	}
	return 0.0f;
}

FVector UVehicleSystemFunctions::GetBoneBounds(UPrimitiveComponent* target, FName BoneName, FVector &Origin)
{
	if (target)
	{
		FBodyInstance* BI = target->GetBodyInstance(BoneName);
		if (BI)
		{
			FBoxSphereBounds MyBounds = BI->GetBodyBounds();
			Origin = MyBounds.Origin;
			return MyBounds.BoxExtent;
		}

	}
	return FVector();
}

FVector UVehicleSystemFunctions::GetMeshCenterOfMass(UPrimitiveComponent* target, FName BoneName)
{
	FBodyInstance* targetBI = target->GetBodyInstance(BoneName);
	if (targetBI)
	{
		return targetBI->GetMassSpaceLocal().GetLocation();
	}

	return FVector::ZeroVector;
}

void UVehicleSystemFunctions::PrintToScreenWithTag(const FString& InString, FLinearColor TextColor, float Duration, int Tag)
{
	GEngine->AddOnScreenDebugMessage(Tag, Duration, TextColor.ToFColor(true), InString);
}

//TODO: Change these functions into a single output ENUM, GetCurrentWorldType
bool UVehicleSystemFunctions::RunningInEditor_World(UObject* WorldContextObject)
{
	if(!WorldContextObject) return false;
	
	//using a context object to get the world
	UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if(!World) return false;
	
	return (World->WorldType == EWorldType::Editor );
}

bool UVehicleSystemFunctions::RunningInPIE_World(UObject* WorldContextObject)
{
	if(!WorldContextObject) return false;
	
	//using a context object to get the world
	UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if(!World) return false;
	
	return (World->WorldType == EWorldType::PIE );
}

bool UVehicleSystemFunctions::RunningInGame_World(UObject* WorldContextObject)
{
	if(!WorldContextObject) return false;
	
	//using a context object to get the world
	UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if(!World) return false;
	
	return (World->WorldType == EWorldType::Game );
}

//Chaos physics thread force functions

FTransform UVehicleSystemFunctions::AVS_GetChaosTransform(UPrimitiveComponent* target)
{
	if(IsValid(target))
	{
		if(const FBodyInstance* BodyInstance = target->GetBodyInstance())
		{
			if(auto Handle = BodyInstance->ActorHandle)
			{
				if(Chaos::FRigidBodyHandle_Internal* RigidHandle = Handle->GetPhysicsThreadAPI())
				{
					const Chaos::FRigidTransform3 WorldCOM = Chaos::FParticleUtilitiesGT::GetActorWorldTransform(RigidHandle);

					return WorldCOM;
				}
			}
		}
	}
	return FTransform();
}


void UVehicleSystemFunctions::AVS_ChaosAddForce(UPrimitiveComponent* target, FVector Force, bool bAccelChange)
{
	if(!target)
		return;
	
	if(const FBodyInstance* BodyInstance = target->GetBodyInstance())
	{
		if(auto Handle = BodyInstance->ActorHandle)
		{
			if(Chaos::FRigidBodyHandle_Internal* RigidHandle = Handle->GetPhysicsThreadAPI())
			{
				if(bAccelChange)
				{
					const float RigidMass = RigidHandle->M();
					const Chaos::FVec3 Acceleration = Force * RigidMass;
					RigidHandle->AddForce(Acceleration, false);
				}
				else
				{
					RigidHandle->AddForce(Force, false);
				}
			}
		}
	}
}

void UVehicleSystemFunctions::AVS_ChaosAddForceAtLocation(UPrimitiveComponent* target, FVector Location, FVector Force)
{
	if(!target)
		return;
	
	if(const FBodyInstance* BodyInstance = target->GetBodyInstance())
	{
		if(auto Handle = BodyInstance->ActorHandle)
		{
			if(Chaos::FRigidBodyHandle_Internal* RigidHandle = Handle->GetPhysicsThreadAPI())
			{
				const Chaos::FVec3 WorldCOM = Chaos::FParticleUtilitiesGT::GetCoMWorldPosition(RigidHandle);
				const Chaos::FVec3 WorldTorque = Chaos::FVec3::CrossProduct(Location - WorldCOM, Force);
				RigidHandle->AddForce(Force, false);
				RigidHandle->AddTorque(WorldTorque, false);
			}
		}
	}
}

void UVehicleSystemFunctions::AVS_ChaosAddTorque(UPrimitiveComponent* target, FVector Torque, bool bAccelChange)
{
	if(!target)
		return;
	
	if(const FBodyInstance* BodyInstance = target->GetBodyInstance())
	{
		if(auto Handle = BodyInstance->ActorHandle)
		{
			if(Chaos::FRigidBodyHandle_Internal* RigidHandle = Handle->GetPhysicsThreadAPI())
			{
				if(bAccelChange)
				{
					RigidHandle->AddTorque(Chaos::FParticleUtilitiesXR::GetWorldInertia(RigidHandle) * Torque, false);
				}
				else
				{
					RigidHandle->AddTorque(Torque, false);
				}
			}
		}
	}
}

void UVehicleSystemFunctions::AVS_ChaosBrakes(UPrimitiveComponent* target, float BrakePower, float ChaosDelta)
{
	if(!target)
		return;
	
	if(const FBodyInstance* BodyInstance = target->GetBodyInstance())
	{
		if(auto Handle = BodyInstance->ActorHandle)
		{
			if(Chaos::FRigidBodyHandle_Internal* RigidHandle = Handle->GetPhysicsThreadAPI())
			{
				FVector AngVel = RigidHandle->W();
				FVector Torque = (AngVel / ChaosDelta)*(-1.0f) * BrakePower;
				RigidHandle->AddTorque(Chaos::FParticleUtilitiesXR::GetWorldInertia(RigidHandle) * Torque, false);
			}
		}
	}
}

FVector UVehicleSystemFunctions::AVS_ChaosGetPhysicsVelocityAtPoint(UPrimitiveComponent* target, FVector Point)
{
	if(const FBodyInstance* BodyInstance = target->GetBodyInstance())
	{
		if(!BodyInstance)
			return FVector::ZeroVector;

		if(auto Handle = BodyInstance->ActorHandle)
		{
			if(Chaos::FRigidBodyHandle_Internal* RigidHandle = Handle->GetPhysicsThreadAPI())
			{
				const Chaos::FVec3 COM = Chaos::FParticleUtilitiesGT::GetCoMWorldPosition(RigidHandle);
				const Chaos::FVec3 Diff = Point - COM;
				return RigidHandle->V() - Chaos::FVec3::CrossProduct(Diff, RigidHandle->W());
			}
		}
	}
	return FVector::ZeroVector;
}
