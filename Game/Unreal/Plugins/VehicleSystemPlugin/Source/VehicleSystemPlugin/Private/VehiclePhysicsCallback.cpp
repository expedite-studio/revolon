#include "VehiclePhysicsCallback.h"

#include "PBDRigidsSolver.h"
#include "VehicleSystemBase.h"

void FVehiclePhysicsCallback::OnPreSimulate_Internal()
{
	using namespace Chaos;

	float ChaosDeltaTime = GetDeltaTime_Internal();

	FVehiclePhysicsPhysicsOutput& NewOutput = GetProducerOutputData_Internal();
	NewOutput.ChaosDeltaTime = ChaosDeltaTime;
	
	const FVehiclePhysicsPhysicsInput* Input = GetConsumerInput_Internal();
	if (Input == nullptr || Input->VehicleMeshBodyInstance == nullptr )
		return;
	
	Chaos::FPhysicsSolver* PhysicsSolver = static_cast<Chaos::FPhysicsSolver*>(GetSolver());
	if (PhysicsSolver == nullptr)
		return;

	FPhysicsActorHandle ActorHandle = Input->VehicleMeshBodyInstance->GetPhysicsActorHandle();
	if(ActorHandle == nullptr)
		return;
	
	Chaos::FRigidBodyHandle_Internal* PhysicsHandle = ActorHandle->GetPhysicsThreadAPI();
	if(PhysicsHandle != nullptr)
	{
		AVehicleSystemBase* MyVehicle = Cast<AVehicleSystemBase>(Input->VehicleActor);
		if(MyVehicle != nullptr)
		{
			MyVehicle->AVS_PhysicsTick(ChaosDeltaTime); // This allows us to use the Physics Thread in Blueprint
		}
	}
}

//RigidHandle Examples, ripped from ChaosVehicles
/*
Chaos::FRigidBodyHandle_Internal* Handle = Input->PhysicsBody->GetPhysicsThreadAPI();

const FTransform WorldTM(Handle->R(), Handle->X());
VehicleWorldTransform = WorldTM;
VehicleWorldVelocity = Handle->V();
VehicleWorldAngularVelocity = Handle->W();
VehicleWorldCOM = Handle->CenterOfMass();
WorldVelocityNormal = VehicleWorldVelocity.GetSafeNormal();

VehicleUpAxis = VehicleWorldTransform.GetUnitAxis(EAxis::Z);
VehicleForwardAxis = VehicleWorldTransform.GetUnitAxis(EAxis::X);
VehicleRightAxis = VehicleWorldTransform.GetUnitAxis(EAxis::Y);

VehicleLocalVelocity = VehicleWorldTransform.InverseTransformVector(VehicleWorldVelocity);
LocalAcceleration = (VehicleLocalVelocity - LastFrameVehicleLocalVelocity) / DeltaTime;
LocalGForce = LocalAcceleration / FMath::Abs(GravityZ);
LastFrameVehicleLocalVelocity = VehicleLocalVelocity;

ForwardSpeed = FVector::DotProduct(VehicleWorldVelocity, VehicleForwardAxis);
ForwardsAcceleration = LocalAcceleration.X;
*/

void FVehiclePhysicsCallback::OnContactModification_Internal(Chaos::FCollisionContactModifier& Modifier)
{
	using namespace Chaos;
	
	if(VehicleMesh == nullptr)
		return;
	
	for (Chaos::FContactPairModifier& PairModifier : Modifier)
	{
		FSingleParticlePhysicsProxy* ContactObject1 = static_cast<FSingleParticlePhysicsProxy*>(PairModifier.GetParticlePair()[0]->PhysicsProxy());
		FSingleParticlePhysicsProxy* ContactObject2 = static_cast<FSingleParticlePhysicsProxy*>(PairModifier.GetParticlePair()[1]->PhysicsProxy());
		if(ContactObject1 == VehicleMesh)
		{
			if(WheelMeshes.Contains(ContactObject2))
			{
				PairModifier.Disable(); // Disable Collision
			}
		}
		else if (ContactObject2 == VehicleMesh)
		{
			if(WheelMeshes.Contains(ContactObject1))
			{
				PairModifier.Disable(); // Disable Collision
			}
		}
	}
}