// Copyright 2019-2023 Seven47 Software. All Rights Reserved.
// Unauthorized copying of this file, via any medium is strictly prohibited

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "VehicleWheelBase.generated.h"

UENUM(BlueprintType)
enum class EWheelMode : uint8
{
	Raycast, Physics
};

USTRUCT(BlueprintType)
struct FDebugForce
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle - Debug")
	FVector Location = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle - Debug")
	FVector Force = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle - Debug")
	EWheelMode WheelMode = EWheelMode::Raycast;
	
	FDebugForce(){}

	FDebugForce(FVector InLocation, FVector InForce, EWheelMode InWheelMode)
	{
		Location = InLocation;
		Force = InForce;
		WheelMode = InWheelMode;
	}
};

USTRUCT(BlueprintType)
struct FAVS_Inputs // Input data to sent to physics thread each game tick
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle - Input")
	float Steering = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle - Input")
	float Throttle = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle - Input")
	float Brake = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle - Input")
	bool Handbrake = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle - Input")
	float Torque = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle - Input")
	bool ReverseTorque = false;
	
	FAVS_Inputs(){}
};

USTRUCT()
struct FAVS1_Wheel_State // Physics thread wheel state
{
	GENERATED_BODY()

	FVector2D Slip = FVector2D(0.0f, 0.0f);
	float AngularVelocity = 0.0f;
	
	FAVS1_Wheel_State(){}
};

USTRUCT(BlueprintType)
struct FAVS1_Wheel_Output // Data output from the physics thread
{
	GENERATED_BODY()

	// Last trace
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle System Plugin|Wheel State")
	FHitResult LastTrace;
	
	// Wheel's angular velocity in Rad/s
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle System Plugin|Wheel State")
	float AngularVelocity = 0.0f;

	// Length of the spring at the current compression
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle System Plugin|Wheel State")
	float CurrentSpringLength = 0.0f;
	
	FAVS1_Wheel_Output(){}
};

USTRUCT(BlueprintType)
struct FAVS1_Wheel_Config // Configuration data to sent to physics thread each game tick //TODO: Split into WheelConfig & SimulationConfig
{
	GENERATED_BODY()

	// Cached wheel position relative to Vehicle (Not relative to parent)
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Vehicle System Plugin|Wheel State")
	FTransform WheelLocalTransform = FTransform();

	UPROPERTY(Transient)
	bool isLocked = false;

	// Wheel physics object
	UPROPERTY()
	UPrimitiveComponent* WheelPrim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Wheel - Config|Wheel")
	EWheelMode WheelMode = EWheelMode::Raycast;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Wheel - Config|Wheel|Raycast Settings", meta=(EditCondition="WheelMode==EWheelMode::Raycast"))
	TEnumAsByte<ECollisionChannel> TraceChannel = ECollisionChannel::ECC_Vehicle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Wheel - Config|Wheel|Raycast Settings", meta=(EditCondition="WheelMode==EWheelMode::Raycast"))
	TArray<AActor*> TraceIgnoreActors;

	// (Rim+Tire) Wheel Mass in Kg
	// Used in wheel simulation, not the actual mass of the wheel mesh physics object
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Wheel - Config|Wheel", meta=(Units="Kg"))
	float WheelMass = 15.0f;

	// Use the wheel mesh bounds to determine WheelRadius
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Wheel - Config|Wheel")
	bool AutoWheelRadius = true;
	
	// Wheel mesh Radius in cm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Wheel - Config|Wheel", meta=(EditCondition="AutoWheelRadius==false", Units="cm"))
	float WheelRadius = 30.0f;

	// Friction Coefficient
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Wheel - Config|Wheel")
	FVector2D TireFriction = FVector2D(1.4f, 1.4f);
	
	// Wheel receives torque
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Wheel - Config|Drive/Steer")
	bool IsDrivingWheel = false;

	// Wheel rotates around Yaw with the steering input, if this is false you can still control steer manually with "SetCurrentSteering"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Wheel - Config|Drive/Steer")
	bool IsSteerableWheel = false;

	// Maximum angle (in degrees) that the wheel steers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Wheel - Config|Drive/Steer", meta=(EditCondition="IsSteerableWheel"))
	float MaxSteeringAngle = 30.0f;

	// Reverse direction of torque (usually for wheel's facing the opposite direction)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Wheel - Config|Drive/Steer")
	bool InvertTorque = false;

	// Reverse direction of steering
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Wheel - Config|Drive/Steer")
	bool InvertSteering = false;

	// Wheel stops with handbrake
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Wheel - Config|Brakes")
	bool IsBrakingWheel = true;

	// Brake pressure applied to wheel (0 - 1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Wheel - Config|Brakes")
	float BrakePressure = 0.1f;

	// Constant resistance applied to wheel (0 - 1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Wheel - Config|Brakes")
	float RollingResistance = 0.01f;

	// Wheel stops with handbrake
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Wheel - Config|Brakes")
	bool IsHandbrakeWheel = false;

	// Show a preview of wheel travel, can be helpful while configuring suspension
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Wheel - Config|Suspension")
	bool EditorPreview = false;
	
	// Length of Spring in cm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Wheel - Config|Suspension")
	float SpringLength = 25.0f;
	// Spring rate in N/mm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Wheel - Config|Suspension")
	float SpringStrength = 25.0f;
	// Damper force in kNs/m
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Wheel - Config|Suspension")
	float SpringDamping = 1.0f;

	FAVS1_Wheel_Config(){ CalculateConstants(); }

	// Constants
	UPROPERTY(Transient)
	float WheelRadiusM; // Radius of wheel in meters
	UPROPERTY(Transient)
	float Inertia; // Inertia of wheel in kg*m^2
	void CalculateConstants()
	{
		WheelRadiusM = WheelRadius * 0.01f; // Convert to meters
		Inertia = (0.5f * WheelMass * WheelRadiusM * WheelRadiusM); // Calculate inertia in kg*m^2
	}
};

UCLASS(Abstract, Blueprintable, ClassGroup="VehicleSystem", meta=(BlueprintSpawnableComponent))
class VEHICLESYSTEMPLUGIN_API UVehicleWheelBase : public USceneComponent
{
	GENERATED_BODY()

private:
	float CurAngVel = 0.0f;

protected: // Accessible by subclasses
	virtual void BeginPlay() override;

	// Updates the WheelRadius if AutoWheelRadius is true
	UFUNCTION(BlueprintCallable, Category = "Vehicle System Plugin|Wheel State")
	void UpdateWheelRadius();

	// Updates the transform cache used for suspension simulation
	UFUNCTION(BlueprintCallable, Category = "Vehicle System Plugin|Wheel State")
	void UpdateLocalTransformCache();

	UPROPERTY(BlueprintReadWrite, Category = "Vehicle System Plugin|Wheel State")
	bool SimulateSuspension = true;

	UPROPERTY(BlueprintReadWrite, Category = "Vehicle System Plugin|Wheel State")
	bool isAttached = false;

	UPROPERTY(BlueprintReadWrite, Category = "Vehicle System Plugin|Wheel State")
	bool isLocked = false;

	// Target wheel angular velocity in rad/s
	UPROPERTY(BlueprintReadWrite, Category = "Vehicle System Plugin|Wheel State")
	float TargetAngVel = 0.0f;
	
public:	
	UVehicleWheelBase();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ** Config ** //

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Vehicle System Plugin|Wheel State")
	void SetWheelMode(EWheelMode NewMode);
	
	// Mesh used to represent the wheel
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Wheel - Config")
	UStaticMesh* WheelStaticMesh;
	
	// Current configuration of this wheel, this is the data sent to the physics simulation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Wheel - Config", meta=(ShowOnlyInnerProperties))
	FAVS1_Wheel_Config WheelConfig;

	// Update this wheel's mass (Rim+Tire) in Kg
	UFUNCTION(BlueprintCallable, Category = "Vehicle Wheel - Config")
	void SetRaycastWheelMass(float NewMass)
	{
		WheelConfig.WheelMass = NewMass;
		WheelConfig.CalculateConstants(); // Recalculate inertia
	}

	/** Creates a constraint between the skeletal mesh bone and this wheel's collision or mesh component */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Vehicle Wheel - Config|Wheel|Skeletal Mesh")
	bool ConnectToBone = false;
	/** Bone to snap to if we are a child of a skeletal mesh */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Vehicle Wheel - Config|Wheel|Skeletal Mesh")
	FName BoneName;


	// ** Simulation ** //

	// Current wheel data output from the simulation
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle System Plugin|Wheel State")
	FAVS1_Wheel_Output WheelData;

	UPROPERTY()
	FRotator WheelRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Vehicle System Plugin|Wheel State")
	UPrimitiveComponent* WheelMeshComponent;

	/** Wheel's Physics Constraint has a spring */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Vehicle Wheel - Config|Suspension|Physics Mode")
	bool HasSpring = true;

	/**
	 * True: Do not let wheel exceed spring bounds (WheelTravel)
	 * False: Soft-Lock (Damp) wheel past spring bounds (WheelTravel)
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Vehicle Wheel - Config|Suspension|Physics Mode")
	bool SpringHardLock = false;

	/** Force applied down -Z on the wheel at all times */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Vehicle Wheel - Config|Suspension|Physics Mode")
	double PhysicsDownforce = 50.0f;


	UFUNCTION(BlueprintCallable, Category = "Vehicle System Plugin|Wheel State")
	void SetWheelMeshComponent(UPrimitiveComponent* NewComponent)
	{
		WheelMeshComponent = NewComponent;
		WheelConfig.WheelPrim = NewComponent;
		UpdateWheelRadius();
	}

	UFUNCTION(BlueprintCallable, Category = "Vehicle System Plugin|Wheel State")
	void ResetWheelCollisions();

	UFUNCTION(BlueprintPure, Category = "Vehicle System Plugin|Wheel State")
	bool GetHasContact() { return WheelData.LastTrace.bBlockingHit; }

	UFUNCTION(BlueprintPure, Category = "Vehicle System Plugin|Wheel State")
	EWheelMode GetWheelMode(){ return WheelConfig.WheelMode; }

	UFUNCTION(BlueprintPure, Category = "Vehicle System Plugin|Wheel State")
	bool GetIsAttached() const { return isAttached; }

	UFUNCTION(BlueprintCallable, Category = "Vehicle System Plugin|Wheel State")
	void SetIsSimulatingSuspension(bool NewSimulate) { SimulateSuspension = NewSimulate; }

	UFUNCTION(BlueprintPure, Category = "Vehicle System Plugin|Wheel State")
	bool GetIsSimulatingSuspension() const { return SimulateSuspension; }

	UFUNCTION(BlueprintPure, Category = "Vehicle System Plugin|Wheel State")
	float GetWheelAngVelInRadians();

	UFUNCTION(BlueprintCallable, Category = "Vehicle System Plugin|Wheel State")
	FVector GetWheelVelocity(bool Local = false);


	// ** Inputs ** //

	UPROPERTY(BlueprintReadOnly, Category = "Vehicle System Plugin|Wheel State")
	float SteeringInput = 0.0f;

	UFUNCTION(BlueprintCallable, Category = "Vehicle System Plugin|Wheel State")
	void SetSteeringInput(float Steering = 0.0f, bool InvertSteering = false)
	{
		SteeringInput = FMath::Clamp((InvertSteering ? (Steering * -1.0f) : Steering), -1.0f, 1.0f);
	}

	UFUNCTION(BlueprintPure, Category = "Vehicle System Plugin|Wheel State")
	float GetSteeringInput()
	{
		return SteeringInput;
	}

	UFUNCTION(BlueprintPure, Category = "Vehicle System Plugin|Wheel State")
	float GetSteeringAngle()
	{
		return WheelConfig.MaxSteeringAngle * SteeringInput;
	}
};
