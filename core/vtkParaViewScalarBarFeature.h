/*=========================================================================

  Program:   Visualization Toolkit

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

   This software is distributed WITHOUT ANY WARRANTY; without even
   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
   PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkParaViewScalarBarFeature -
// .SECTION Description
//

#ifndef __vtkParaViewScalarBarFeature_h
#define __vtkParaViewScalarBarFeature_h

#include <vtkActor.h>
#include <vtkNew.h>
#include <vtkObject.h>

#include "vtkFeature.h"
#include "vtkmapcore_export.h"

class vtkDiscretizableColorTransferFunction;

class VTKMAPCORE_EXPORT vtkParaViewScalarBarFeature : public vtkFeature
{
public:
  static vtkParaViewScalarBarFeature* New();
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(vtkParaViewScalarBarFeature, vtkFeature)

  void Init() override;

  void CleanUp() override;

  void Update() override;

  vtkProp* PickProp() override;

  // NB: ParaView scalar bar needs to be fed with a vtkDiscretizableCTF
  void SetLookupTable(vtkDiscretizableColorTransferFunction* lut);
  void SetTitle(const std::string& title);

protected:
  vtkParaViewScalarBarFeature();
  ~vtkParaViewScalarBarFeature() override;

private:
  vtkParaViewScalarBarFeature(const vtkParaViewScalarBarFeature&);            // Not implemented
  vtkParaViewScalarBarFeature& operator=(const vtkParaViewScalarBarFeature&); // Not implemented

  struct ParaViewScalarBarFeatureInternals;
  ParaViewScalarBarFeatureInternals* const Internals;
};

#endif // __vtkParaViewScalarBarFeature_h
