/**********************************************************************
cifspacegrouptest.cpp - Unit tests for to check if space group is being handled
properly in .cif format.

Copyright (C) 2016 by Schrodinger Inc.

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
#include <openbabel/obconversion.h>
#include <openbabel/generic.h>

#include <string>
#include <algorithm>

using namespace std;
using namespace OpenBabel;

std::string static GetFilename(const std::string &filename)
{
  string path = TEST_SAMPLES_PATH + std::string("/files/") + filename;
  return path;
}

void testSpaceGroupUniqueTransformations()
{
  // See  https://github.com/openbabel/openbabel/pull/260
  // also https://github.com/openbabel/openbabel/pull/255
  OBConversion conv;
  OBMol mol;
  conv.SetInFormat("cif");
  conv.ReadFile(&mol, GetFilename("test01.cif"));
  OBUnitCell* pUC = (OBUnitCell*)mol.GetData(OBGenericDataType::UnitCell);

  const SpaceGroup* pSG = pUC->GetSpaceGroup();

  BOOST_REQUIRE(pSG != nullptr && pSG->IsValid());
  BOOST_REQUIRE( pSG->GetId() == 64 );
}

void testSpaceGroupClean()
{
  // See https://github.com/openbabel/openbabel/pull/254
  OBConversion conv;
  OBMol mol;
  conv.SetInFormat("cif");
  conv.SetOutFormat("pdb");
  conv.ReadFile(&mol, GetFilename("test02.cif"));
  OBUnitCell* pUC = (OBUnitCell*)mol.GetData(OBGenericDataType::UnitCell);
  const SpaceGroup* pSG = pUC->GetSpaceGroup();
  BOOST_REQUIRE(pSG != nullptr && pSG->IsValid());

  BOOST_REQUIRE( pSG->GetId() == 166 );

  string pdb = conv.WriteString(&mol);

  BOOST_REQUIRE(pdb.find("H -3 m") != string::npos);
}

void testSpaceGroupTransformations()
{
  // See https://github.com/openbabel/openbabel/pull/254
  SpaceGroup group;
  vector<string> trans;
  vector<string> trans_exp;
  vector<string> trans_got;

  // Same transformation
  trans_exp.push_back("x,y,z");
  trans.push_back("x,y,z");
  trans.push_back(" x , y , z ");
  trans.push_back("x,y+1,z");
  trans.push_back("x,y-1,z-1");
  trans.push_back("x,y-1,z");
  // Same transformation
  trans_exp.push_back("x,-y,z");
  trans.push_back("x,-y,z");
  trans.push_back("x,1-y,z");
  trans.push_back("x,-y+1,z");
  // Same transformation
  trans_exp.push_back("x,-y,-z");
  trans.push_back("x,-y,-z");
  trans.push_back("x,1-y,1-z");
  trans.push_back("x,-y+1,-z+1");
  // Same transformation
  trans_exp.push_back("x,-y,-y-z");
  trans.push_back("x,-y,-z-y");
  trans.push_back("x,1-y,-y+1-z");
  trans.push_back("x,-y+1,-y-z+1");
  // Same transformation
  trans_exp.push_back("x,-y,1/6-y-z");
  trans.push_back("x,-y,-z-y+1/6");
  trans.push_back("x,-y,-z-y+7/6");
  trans.push_back("x,1-y,7/6-y+1-z");
  trans.push_back("x,-y+1,-y-z+1+1/6");
  trans.push_back("x,-y+1,-y+1/6-z+1");
  // Same transformation
  trans_exp.push_back("x,3/4-y+z,5/6-y-z");
  trans.push_back("x,3/4-y+z,-1/6-z-y");
  trans.push_back("x,-y+3/4+z,-z-y-1/6");
  trans.push_back("x,z-y+3/4,-z-y-7/6");
  trans.push_back("x,1+z+3/4-y,-7/6-y-z");
  trans.push_back("X , 3 / 4 - Y + 1 + z , - Y - Z - 1 / 6 ");
  trans.push_back("x,z-y+1+3/4,-y-1/6-z+1");

  vector<string>::const_iterator i, iend;
  iend = trans.end();
  for (i = trans.begin(); i != iend; ++i)
    group.AddTransform(i->c_str());

  // Loop over symmetry operators
  transform3dIterator ti;
  const transform3d *t = group.BeginTransform(ti);
  while(t){
    trans_got.push_back(t->DescribeAsString());
    //cout << t->DescribeAsString() << "\n";
    t = group.NextTransform(ti);
  }

  BOOST_REQUIRE( trans_exp.size() == trans_got.size() );
  BOOST_REQUIRE( equal(trans_exp.begin(), trans_exp.end(), trans_got.begin()) );
}

void testDecayToP1()
{
  // See https://github.com/openbabel/openbabel/pull/261
  OBConversion conv;
  OBMol mol;
  conv.SetInFormat("cif");
  conv.ReadFile(&mol, GetFilename("test03.cif"));
  OBUnitCell* pUC = (OBUnitCell*)mol.GetData(OBGenericDataType::UnitCell);
  const SpaceGroup* pSG = pUC->GetSpaceGroup();
  BOOST_REQUIRE(pSG != nullptr && pSG->IsValid());

  // Check also for errors and warnings
  string summary = obErrorLog.GetMessageSummary();
  BOOST_REQUIRE( summary.find("2 warnings") != string::npos);

  BOOST_REQUIRE( pSG->GetId() == 1 );
}

void testAlternativeOrigin()
{
  // See https://github.com/openbabel/openbabel/pull/1558
  OBConversion conv;
  OBMol mol;
  conv.SetInFormat("cif");
  conv.ReadFile(&mol, GetFilename("test04.cif"));
  OBUnitCell* pUC = (OBUnitCell*)mol.GetData(OBGenericDataType::UnitCell);
  const SpaceGroup* pSG = pUC->GetSpaceGroup();

  string summary = obErrorLog.GetMessageSummary();
  BOOST_REQUIRE( summary.find("warning") == string::npos);
  BOOST_REQUIRE(pSG != nullptr && pSG->IsValid());
  BOOST_REQUIRE( pSG->GetOriginAlternative() == 1);
}

void testPdbOutAlternativeOrigin()
{
  // See https://github.com/openbabel/openbabel/pull/1558
  OBConversion conv;
  OBMol mol;
  conv.SetInFormat("cif");
  conv.SetOutFormat("pdb");
  conv.ReadFile(&mol, GetFilename("test04.cif"));

  string pdb = conv.WriteString(&mol);
  // ending space is needed to check that there is no origin set
  BOOST_REQUIRE(pdb.find("P 4/n b m ") != string::npos);

  conv.AddOption("o", OBConversion::OUTOPTIONS);
  pdb = conv.WriteString(&mol);

  BOOST_REQUIRE(pdb.find("P 4/n b m:1") != string::npos);
}

void testPdbOutHexagonalAlternativeOrigin()
{
  // See https://github.com/openbabel/openbabel/pull/1558
  OBConversion conv;
  OBMol mol;
  conv.SetInFormat("cif");
  conv.SetOutFormat("pdb");
  conv.ReadFile(&mol, GetFilename("test02.cif"));

  string pdb = conv.WriteString(&mol);
  conv.AddOption("o", OBConversion::OUTOPTIONS);
  pdb = conv.WriteString(&mol);

  BOOST_REQUIRE(pdb.find("H -3 m") != string::npos);

  // Test with missing Hall name in the CIF
  // https://github.com/openbabel/openbabel/pull/1578
  OBMol mol_nohall;
  conv.ReadFile(&mol_nohall, GetFilename("test02.nohall.cif"));

  pdb = conv.WriteString(&mol_nohall);

  BOOST_REQUIRE(pdb.find("H -3 m") != string::npos);
}

void testPdbOutAlternativeOriginSilicon()
{
  // See https://github.com/openbabel/openbabel/pull/1558
  OBConversion conv;
  OBMol mol;
  conv.SetInFormat("cif");
  conv.SetOutFormat("pdb");
  conv.ReadFile(&mol, GetFilename("test05.cif"));

  string pdb = conv.WriteString(&mol);
  conv.AddOption("o", OBConversion::OUTOPTIONS);
  pdb = conv.WriteString(&mol);

  BOOST_REQUIRE(pdb.find("F d 3 m:1") != string::npos);
}

void testPdbOutHexagonalAlternativeOrigin2()
{
  // See https://github.com/openbabel/openbabel/pull/1558
  OBConversion conv;
  OBMol mol;
  conv.SetInFormat("cif");
  conv.SetOutFormat("pdb");
  conv.ReadFile(&mol, GetFilename("test06.cif"));

  string pdb = conv.WriteString(&mol);
  conv.AddOption("o", OBConversion::OUTOPTIONS);
  pdb = conv.WriteString(&mol);

  BOOST_REQUIRE(pdb.find("H -3 m") != string::npos);
}

void testPdbRemSpacesHMName()
{
  // See https://github.com/openbabel/openbabel/pull/1558
  OBConversion conv;
  OBMol mol;
  conv.SetInFormat("cif");
  conv.SetOutFormat("pdb");
  conv.ReadFile(&mol, GetFilename("test07.cif"));

  string pdb = conv.WriteString(&mol);
  conv.AddOption("o", OBConversion::OUTOPTIONS);
  pdb = conv.WriteString(&mol);

  BOOST_REQUIRE(pdb.find("I41/amd:2") != string::npos);
}

void testPdbOccupancies()
{
  // See https://github.com/openbabel/openbabel/pull/1558
  OBConversion conv;
  OBMol mol;
  conv.SetInFormat("cif");
  conv.SetOutFormat("pdb");
  conv.ReadFile(&mol, GetFilename("test08.cif"));

  string pdb = conv.WriteString(&mol);
  conv.AddOption("o", OBConversion::OUTOPTIONS);
  pdb = conv.WriteString(&mol);

  BOOST_REQUIRE(pdb.find("HETATM    1 NA   UNL     1       0.325   0.000   4.425  0.36") != string::npos);
  BOOST_REQUIRE(pdb.find("HETATM   17  O   UNL     8       1.954   8.956   3.035  1.00") != string::npos);

  OBMol mol_pdb;
  conv.SetInFormat("pdb");
  conv.ReadFile(&mol_pdb, GetFilename("test09.pdb"));

  pdb = conv.WriteString(&mol_pdb);
  BOOST_REQUIRE(pdb.find("HETATM    1 NA   UNL     1       0.325   0.000   4.425  0.36") != string::npos);
  BOOST_REQUIRE(pdb.find("HETATM    2 NA   UNL     1       0.002   8.956   1.393  0.10") != string::npos);
  BOOST_REQUIRE(pdb.find("HETATM   17  O   UNL     8       1.954   8.956   3.035  1.00") != string::npos);
}

void testCIFMolecules()
{
  // See https://github.com/openbabel/openbabel/pull/1558
  OBConversion conv;
  OBMol mol;
  conv.SetInFormat("cif");
  conv.SetOutFormat("smi"); // check for disconnected fragments
  conv.ReadFile(&mol, GetFilename("1519159.cif"));

  string smi = conv.WriteString(&mol);
  // never, never disconnected fragments from a molecule
  BOOST_REQUIRE(smi.find(".") == string::npos);
}

void testCIFOutputFormat()
{
  // See https://github.com/openbabel/openbabel/pull/2170
  OBConversion conv;
  OBMol mol;
  conv.SetInFormat("sdf");
  conv.SetOutFormat("cif"); // check correct format
  conv.ReadFile(&mol, GetFilename("kevlar.sdf"));

  string cif = conv.WriteString(&mol);

  string ref = "    H0       H      -71.99400 -128.76240   56.30360    1.000";
  BOOST_REQUIRE(cif.find(ref) != string::npos);
}

BOOST_AUTO_TEST_CASE(cifspacegrouptest)
{
    testSpaceGroupUniqueTransformations();
    testSpaceGroupClean();
    testSpaceGroupTransformations();
    testDecayToP1();
    testAlternativeOrigin();
    testPdbOutAlternativeOrigin();
    testPdbOutHexagonalAlternativeOrigin();
    testPdbOutAlternativeOriginSilicon();
    testPdbOutHexagonalAlternativeOrigin2();
    testPdbRemSpacesHMName();
    testPdbOccupancies();
    testCIFMolecules();
    testCIFOutputFormat();
}
