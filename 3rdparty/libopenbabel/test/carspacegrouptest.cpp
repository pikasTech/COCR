/**********************************************************************
carspacegrouptest.cpp - Unit tests for to check if space group is being handled
properly in .car format.

Copyright (C) 2013 by Schrodinger Inc.

This file is part of the Open Babel project.
For more information, see <http://openbabel.org/>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
***********************************************************************/
#include <boost/test/unit_test.hpp>
#include <openbabel/babelconfig.h>
#include <openbabel/mol.h>
#include <openbabel/generic.h>
#include <openbabel/obconversion.h>

#include <string>

using namespace std;
using namespace OpenBabel;

std::string static GetFilename(const std::string &filename)
{
  return  TEST_SAMPLES_PATH + "files/" + filename;
}

void testSpaceGroupWithSpace()
{
  OBConversion conv;
  OBMol mol;
  conv.SetInFormat("car");
  conv.ReadFile(&mol, GetFilename("test3.car"));
  OBUnitCell* pUC = (OBUnitCell*)mol.GetData(OBGenericDataType::UnitCell);
  BOOST_REQUIRE( pUC->GetSpaceGroupName() == "P 4" );
}


void testSpaceGroupWithoutParentheses()
{
  OBConversion conv;
  OBMol mol;
  conv.SetInFormat("car");
  conv.ReadFile(&mol, GetFilename("test2.car"));
  OBUnitCell* pUC = (OBUnitCell*)mol.GetData(OBGenericDataType::UnitCell);
  BOOST_REQUIRE( pUC->GetSpaceGroupName() == "P4" );
}

void testSpaceGroupWithParentheses()
{
  OBConversion conv;
  OBMol mol;
  conv.SetInFormat("car");
  conv.ReadFile(&mol, GetFilename("test1.car"));
  OBUnitCell* pUC = (OBUnitCell*)mol.GetData(OBGenericDataType::UnitCell);
  BOOST_REQUIRE( pUC->GetSpaceGroupName() == "P4" );

}

void testDefaultSpaceGroup()
{
  OBConversion conv;
  OBMol mol;
  conv.SetInFormat("car");
  conv.ReadFile(&mol, GetFilename("monoclinic.car"));
  OBUnitCell* pUC = (OBUnitCell*)mol.GetData(OBGenericDataType::UnitCell);
  BOOST_REQUIRE( pUC->GetSpaceGroupName() == "");
}

BOOST_AUTO_TEST_CASE(carspacegrouptest)
{
    testDefaultSpaceGroup();
    testSpaceGroupWithParentheses();
    testSpaceGroupWithoutParentheses();
    testSpaceGroupWithSpace();
}
