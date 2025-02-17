/**********************************************************************
ffgaff.cpp - Test energy and gradients for GAFF force field

This file is part of the Open Babel project.
For more information, see <http://openbabel.org/>

Some portions Copyright (C) 2008 Geoffrey R. Hutchison

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

#include <fstream>
#include <cstdlib>

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>
#include <openbabel/forcefield.h>
#include <openbabel/obutil.h>

using namespace std;
using namespace OpenBabel;

#ifdef TESTDATADIR
  string ctestdatadir = TESTDATADIR;
  string cresults_file = ctestdatadir + "gaffresults.txt";
  string cmolecules_file = ctestdatadir + "gaff.sdf";
#else
  string cresults_file = "files/gaffresults.txt";
  string cmolecules_file = "files/gaff.sdf";
#endif

void NGenerateEnergies();

BOOST_AUTO_TEST_CASE(ffgaff)
{
  int defaultchoice = 1;
  
  int choice = defaultchoice;

  if (argc > 1) {
    if(sscanf(argv[1], "%d", &choice) != 1) {
      printf("Couldn't parse that input as a number\n");
      return -1;
    }
  }

  // Define location of file formats for testing
  #ifdef FORMATDIR
    char env[BUFF_SIZE];
    snprintf(env, BUFF_SIZE, "BABEL_LIBDIR=%s", FORMATDIR);
    putenv(env);
  #endif

  if (choice == 99)
    {
      NGenerateEnergies();
      return 0;
    }


  cout << "# Testing GAFF Force Field..." << endl;

  std::ifstream mifs;
  if (!SafeOpen(mifs, cmolecules_file.c_str()))
    {
      cout << "Bail out! Cannot read file " << cmolecules_file << endl;
      return -1; // test failed
    }

  std::ifstream rifs;
  if (!SafeOpen(rifs, cresults_file.c_str()))
    {
      cout << "Bail out! Cannot read file " << cresults_file << endl;
      return -1; // test failed
    }

  char buffer[BUFF_SIZE];
  vector<string> vs;
  OBMol mol;
  OBConversion conv(&mifs, &cout);
  unsigned int currentTest = 0;

  if(! conv.SetInAndOutFormats("SDF","SDF"))
    {
      cout << "Bail out! SDF format is not loaded" << endl;
      return -1; // test failed
    }

  OBForceField* pFF = OBForceField::FindForceField("GAFF");

  if (pFF == nullptr) {
    cerr << "Bail out! Cannot load force field!" << endl;
    return -1; // test failed
  }

  pFF->SetLogFile(&cout);
  pFF->SetLogLevel(OBFF_LOGLVL_NONE);

  double energy;
  while(mifs)
    {
      mol.Clear();
      conv.Read(&mol);
      if (mol.Empty())
        continue;
      if (!rifs.getline(buffer,BUFF_SIZE))
        {
          cout << "Bail out! error reading reference data" << endl;
          return -1; // test failed
        }

      if (!pFF->Setup(mol)) {
        cout << "Bail out! could not setup force field on " << mol.GetTitle() << endl;
        return -1; // test failed
      }

      // compare the calculated energy to our reference data
      energy = pFF->Energy(false);
      if (fabs(atof(buffer) - energy) > 1.0e-3)
        {
          cout << "not ok " << ++currentTest << " # calculated energy incorrect "
               << " for molecule " << mol.GetTitle() << "\n";
          cout << "# Expected " << buffer << " found " <<
            energy << "\n";
        }
      else
        cout << "ok " << ++currentTest << " # energy \n";

      // check that gradients validate too
      if (!pFF->ValidateGradients())
        {
          cout << "not ok " << ++currentTest << " # gradients do not validate "
               << " for molecule " << mol.GetTitle() << "\n";
        }
      else
        cout << "ok " << ++currentTest << " # gradients \n";
    }

  // return number of tests run
  cout << "1.." << currentTest << endl;

  // Passed tests
  return 0;
}

void NGenerateEnergies()
{
  std::ifstream ifs;
  if (!SafeOpen(ifs, cmolecules_file.c_str()))
    return;

  std::ofstream ofs;
  if (!SafeOpen(ofs, cresults_file.c_str()))
    return;

  OBMol mol;
  OBConversion conv(&ifs, &cout);
  char buffer[BUFF_SIZE];

  if(! conv.SetInAndOutFormats("SDF","SDF"))
    {
      cerr << "SDF format is not loaded" << endl;
      return;
    }

  OBForceField* pFF = OBForceField::FindForceField("GAFF");

  if (pFF == nullptr) {
    cerr << "Cannot load force field!" << endl;
    return;
  }

  pFF->SetLogFile(&cout);
  pFF->SetLogLevel(OBFF_LOGLVL_NONE);

  for (;ifs;)
    {
      mol.Clear();
      conv.Read(&mol);
      if (mol.Empty())
        continue;

      if (!pFF->Setup(mol)) {
        cerr << "Could not setup force field on molecule: " << mol.GetTitle() << endl;
        return;
      }

      // Don't compute gradients
      sprintf(buffer, "%15.5f\n", pFF->Energy(false));
      ofs << buffer;
    }

	cerr << " GAFF force field energies written successfully" << endl;
  return;
}
