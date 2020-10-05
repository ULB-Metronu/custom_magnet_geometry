/* 
Beam Delivery Simulation (BDSIM) Copyright (C) Royal Holloway, 
University of London 2001 - 2020.

This file is part of BDSIM.

BDSIM is free software: you can redistribute it and/or modify 
it under the terms of the GNU General Public License as published 
by the Free Software Foundation version 3 of the License.

BDSIM is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with BDSIM.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "UDipole.hh"

#include "BDSAcceleratorComponent.hh"
#include "BDSBeamPipe.hh"
#include "BDSBeamPipeFactory.hh"
#include "BDSBeamPipeInfo.hh"
#include "BDSBeamPipeType.hh"
#include "BDSColours.hh"
#include "BDSExtent.hh"
#include "BDSFieldBuilder.hh"
#include "BDSFieldInfo.hh"
#include "BDSFieldType.hh"
#include "BDSIntegratorType.hh"
#include "BDSMagnetGeometryType.hh"
#include "BDSMagnetOuter.hh"
#include "BDSMagnetOuterFactory.hh"
#include "BDSMagnetOuterInfo.hh"
#include "BDSMagnetStrength.hh"
#include "BDSMagnetType.hh"
#include "BDSMaterials.hh"
#include "BDSSamplerPlane.hh"
#include "BDSSamplerRegistry.hh"
#include "BDSSDSampler.hh" // so we can convert BDSSamplerSD* to G4VSensitiveDetector*
#include "BDSSDManager.hh"
#include "BDSSimpleComponent.hh"
#include "BDSUtilities.hh"

#include "globals.hh" // geant4 globals / types
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"

UDipole::UDipole(G4String name,
		 G4double bFieldIn,
		 G4String params):
  BDSAcceleratorComponent(name, 1.57*CLHEP::m, /*angle*/0, "udipole"),
  bField(bFieldIn),
  horizontalWidth(1*CLHEP::m)
{
    // use function from BDSUtilities to process user params string into
    // map of strings and values.
    std::map<G4String, G4String> map = BDS::GetUserParametersMap(params);

    // the map is supplied at run time so we must check it actually has the key
    // to avoid throwing an exception.
    auto geometryGdmlPathSearch = map.find("gdml");
    if (geometryGdmlPathSearch != map.end())
    {geometryGdmlPath = geometryGdmlPathSearch->second;}
    else
    {geometryGdmlPath = "pipe.gdml";}

    auto yokeFieldMapSearch = map.find("yokeFieldMap");
    if (yokeFieldMapSearch != map.end())
    {yokeFieldMap = yokeFieldMapSearch->second;}
    else
    {yokeFieldMap = "fieldmaps/FieldMap_B3G_Complete.dat.gz";}

    gdml = new BDSGeometryFactoryGDML();
    gdml->Build("magnet", geometryGdmlPath);

    Setvolumesforfields();

}

UDipole::~UDipole()
{;}


void UDipole::Setvolumesforfields(){

    std::set<G4LogicalVolume*> logicalvolumes =  gdml->Getlogicalvolumes();

    std::set<G4LogicalVolume*>::iterator it;
    for (it = logicalvolumes.begin(); it != logicalvolumes.end(); ++it) {
        G4LogicalVolume* f = *it;
        if (f->GetName() == "inner_pipe_l"){
            magnet_pipe_volumes.push_back(f);
        }
        else if (f->GetName() == "wl"){
            containerLogicalVolume = f;
        }
        else if (f->GetMaterial()->GetName() == "G4_Fe"){
            magnet_yoke_volumes.push_back(f);
        }
        else {
            magnet_exteriors_volumes.push_back(f);
        }
    }
}

void UDipole::Build()
{
    BuildMagnet();
    BuildField(yokeFieldMap);
    SetExtents();
}

void UDipole::BuildMagnet()
{
    RegisterPhysicalVolume(gdml->Getphysicalvolumes());
}

void UDipole::BuildField(G4String yokeFieldMap)
{
  // We build a strength object. We specify the field magnitude and unit direction components.
  BDSMagnetStrength* st = new BDSMagnetStrength();
  (*st)["field"] = bField;
  (*st)["bx"] = 1;
  (*st)["by"] = 0;
  (*st)["bz"] = 0;

  // we build a recipe for the field - pure dipole using a G4ClassicalRK4 integrator
  BDSFieldInfo* PipeField = new BDSFieldInfo(BDSFieldType::dipole,
					       0, // brho - not needed for a pure dipole field
					       BDSIntegratorType::g4classicalrk4,
					       st,
					       true);

  // We 'register' the field to be constructed on a specific logical volume. All fields
  // are constructed and attached at once as per the Geant4 way of doing things. true
  // means propagate to all daughter volumes.

  for (auto it = begin (magnet_pipe_volumes); it != end (magnet_pipe_volumes); ++it) {
      BDSFieldBuilder::Instance()->RegisterFieldForConstruction(PipeField,
                                                                *it,
                                                                true);

  }

  BDSFieldInfo* mapfield = new BDSFieldInfo( BDSFieldType::bmap3d, 0, BDSIntegratorType::g4classicalrk4, nullptr,true,G4Transform3D(),yokeFieldMap,BDSFieldFormat::bdsim3d);

  for (auto it = begin (magnet_yoke_volumes); it != end (magnet_yoke_volumes); ++it) {
      BDSFieldBuilder::Instance()->RegisterFieldForConstruction(mapfield,
                                                                *it,
                                                                true);
  }
}

void UDipole::SetExtents()
{
  // record extents so bdsim can check when placing stuff beside this magnet if it overlaps
  BDSExtent ext = BDSExtent(horizontalWidth * 0.5,
			    horizontalWidth * 0.5,
			    chordLength * 0.5);
  SetExtent(ext); // BDSGeometryComponent base class method
}
