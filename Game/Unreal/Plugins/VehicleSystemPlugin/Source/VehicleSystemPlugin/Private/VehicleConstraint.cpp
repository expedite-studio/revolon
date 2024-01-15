// Copyright 2019-2022 Seven47 Software. All Rights Reserved.
// Unauthorized copying of this file, via any medium is strictly prohibited


#include "VehicleConstraint.h"

void UVehicleConstraint::SetLinearSoftConstraint(bool SoftConstraint, float Stiffness, float Damping)
{
	ConstraintInstance.ProfileInstance.LinearLimit.bSoftConstraint = SoftConstraint;
	ConstraintInstance.ProfileInstance.LinearLimit.Stiffness = Stiffness;
	ConstraintInstance.ProfileInstance.LinearLimit.Damping = Damping;
	ConstraintInstance.UpdateLinearLimit();
}