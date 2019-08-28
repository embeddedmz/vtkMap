/*=========================================================================

  Program:   Visualization Toolkit

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

   This software is distributed WITHOUT ANY WARRANTY; without even
   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
   PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkContext2DScalarBarActor.h"
#include "vtkParaViewScalarBarFeature.h"

#include <vtkDiscretizableColorTransferFunction.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkParaViewScalarBarFeature)

struct vtkParaViewScalarBarFeature::ParaViewScalarBarFeatureInternals
{
  vtkNew<vtkContext2DScalarBarActor> ScalarBarActor;
};

//----------------------------------------------------------------------------
vtkParaViewScalarBarFeature::vtkParaViewScalarBarFeature() :
    Internals(new vtkParaViewScalarBarFeature::ParaViewScalarBarFeatureInternals)
{
}

//----------------------------------------------------------------------------
vtkParaViewScalarBarFeature::~vtkParaViewScalarBarFeature()
{
  delete Internals;
}

//----------------------------------------------------------------------------
void vtkParaViewScalarBarFeature::PrintSelf(std::ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  // TODO
}

//----------------------------------------------------------------------------
void vtkParaViewScalarBarFeature::Init()
{
  //if (this->GetMTime() > this->BuildTime.GetMTime())
  //{
  //}

  this->Layer->AddActor2D(this->Internals->ScalarBarActor.GetPointer());
}

//----------------------------------------------------------------------------
void vtkParaViewScalarBarFeature::Update()
{
  this->Internals->ScalarBarActor->SetVisibility(this->IsVisible());
  this->UpdateTime.Modified();
}

//----------------------------------------------------------------------------
void vtkParaViewScalarBarFeature::CleanUp()
{
  this->Layer->RemoveActor2D(this->Internals->ScalarBarActor.GetPointer());
  this->SetLayer(nullptr);
}

//----------------------------------------------------------------------------
vtkProp* vtkParaViewScalarBarFeature::PickProp()
{
  return this->Internals->ScalarBarActor.GetPointer();
}

//----------------------------------------------------------------------------
void vtkParaViewScalarBarFeature::SetLookupTable(vtkDiscretizableColorTransferFunction* lut)
{
  this->Internals->ScalarBarActor->SetLookupTable(lut);
}

//----------------------------------------------------------------------------
void vtkParaViewScalarBarFeature::SetTitle(const std::string& title)
{
  this->Internals->ScalarBarActor->SetTitle(title.c_str());
}
