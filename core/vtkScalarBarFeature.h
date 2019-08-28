/*=========================================================================

  Program:   Visualization Toolkit

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

   This software is distributed WITHOUT ANY WARRANTY; without even
   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
   PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkScalarBarFeature -
// .SECTION Description
//

#ifndef __vtkScalarBarFeature_h
#define __vtkScalarBarFeature_h

#include <vtkActor.h>
#include <vtkNew.h>
#include <vtkObject.h>

#include "vtkFeature.h"
#include "vtkmapcore_export.h"

class vtkContext2DScalarBarActor;
class vtkScalarsToColors;
// NB: ParaView scalar bar needs to be fed with a
// vtkDiscretizableColorTransferFunction

class VTKMAPCORE_EXPORT vtkScalarBarFeature : public vtkFeature
{
public:
  static vtkScalarBarFeature* New();
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(vtkScalarBarFeature, vtkFeature)

  // Description
  // Get actor for the polydata
  //vtkGetObjectMacro(Actor, vtkActor);

  // Description
  // Get mapper for the polydata
  //vtkGetObjectMacro(Mapper, vtkPolyDataMapper);

  void Init() override;

  void CleanUp() override;

  void Update() override;

  vtkProp* PickProp() override;

  void SetLookupTable(vtkScalarsToColors* lut);
  void SetTitle(const std::string& title);

protected:
  vtkScalarBarFeature();
  ~vtkScalarBarFeature();

private:
  vtkScalarBarFeature(const vtkScalarBarFeature&);            // Not implemented
  vtkScalarBarFeature& operator=(const vtkScalarBarFeature&); // Not implemented

  struct ScalarBarFeatureInternals;
  ScalarBarFeatureInternals* const Internals;
};

#endif // __vtkScalarBarFeature_h
