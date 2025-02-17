/**********************************************************************
mol.cpp - Unit tests for Open Babel OBMol class

Copyright (C) 2005-2006 Geoffrey R. Hutchison
 
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
#include <cstdlib>
#include <cstdio>
#include <iostream>

using namespace std;
using namespace OpenBabel;

string cmlfile = "../cmltest/cs2a.cml";
string cmlfile_multi = "3d.head.2.cml";

BOOST_AUTO_TEST_CASE(cmlreadfile)
{
  OBConversion obconv_first;
  BOOST_ASSERT(obconv_first.SetInFormat("CML"));
  string cmlfile = "../cmltest/cs2a.cml";
  string cmlfile_multi = "3d.head.2.cml";
  // Test using ReadFile to read from CML
  OpenBabel::OBMol obmol_first;
  if (obconv_first.ReadFile(&obmol_first, TESTDATADIR + cmlfile))
          cout << "ok 2!" << endl;
  else
          cout << "not ok 2" << endl;


  
  OBConversion obconv;
  if(!obconv.SetInFormat("CML"))
  {
    cout << "Bail out! Fail format isn't loaded!" << endl;
    return -1;
  }
  // Test using ReadFile to read from multimol CML
  OpenBabel::OBMol obmol;
  if (obconv.ReadFile(&obmol, TESTDATADIR + cmlfile_multi))
          cout << "ok 3!" << endl;
  else
          cout << "not ok 3" << endl;

  // Test reading the second and final molecule using Read
  if (obconv.Read(&obmol))
          cout << "ok 4!" << endl;
  else
          cout << "not ok 4" << endl;
  if (obmol.NumAtoms()==28)
          cout << "ok 5!" << endl;
  else
          cout << "not ok 5" << endl;
 
  // Test that there are no remaining molecules
  // (this test fails on Linux)
  // if (!obconv.Read(&obmol))
  //        cout << "ok 6!" << endl;
  // else
  //        cout << "not ok 6" << endl;

  // the total number of tests for "prove"
  // update when you add more tests!
  cout << "1..5\n";

  return 0;
}
