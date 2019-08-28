/*=========================================================================

  Program:   Visualization Toolkit

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

   This software is distributed WITHOUT ANY WARRANTY; without even
   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
   PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkScalarBarFeature.h"

//#include <vtkDiscretizableColorTransferFunction.h>
#include <vtkObjectFactory.h>
#include <vtkScalarsToColors.h>
#include <vtkScalarBarActor.h>

vtkStandardNewMacro(vtkScalarBarFeature)

struct vtkScalarBarFeature::ScalarBarFeatureInternals
{
  //vtkNew<vtkContext2DScalarBarActor> ScalarBarActor;
  vtkNew<vtkScalarBarActor> ScalarBarActor;
};

//----------------------------------------------------------------------------
vtkScalarBarFeature::vtkScalarBarFeature() :
    Internals(new vtkScalarBarFeature::ScalarBarFeatureInternals)
{
}

//----------------------------------------------------------------------------
vtkScalarBarFeature::~vtkScalarBarFeature()
{
  delete Internals;
}

//----------------------------------------------------------------------------
void vtkScalarBarFeature::PrintSelf(std::ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  // TODO
}

//----------------------------------------------------------------------------
void vtkScalarBarFeature::Init()
{
  //if (this->GetMTime() > this->BuildTime.GetMTime())
  //{
  //}

  this->Layer->AddActor2D(this->Internals->ScalarBarActor.GetPointer());
}

//----------------------------------------------------------------------------
void vtkScalarBarFeature::Update()
{
  this->Internals->ScalarBarActor->SetVisibility(this->IsVisible());
  this->UpdateTime.Modified();
}

//----------------------------------------------------------------------------
void vtkScalarBarFeature::CleanUp()
{
  this->Layer->RemoveActor2D(this->Internals->ScalarBarActor.GetPointer());
  this->SetLayer(nullptr);
}

//----------------------------------------------------------------------------
vtkProp* vtkScalarBarFeature::PickProp()
{
  return this->Internals->ScalarBarActor.GetPointer();
}

//----------------------------------------------------------------------------
void vtkScalarBarFeature::SetLookupTable(vtkScalarsToColors* lut)
{
  this->Internals->ScalarBarActor->SetLookupTable(lut);
}

//----------------------------------------------------------------------------
void vtkScalarBarFeature::SetTitle(const std::string& title)
{
  this->Internals->ScalarBarActor->SetTitle(title.c_str());
}
