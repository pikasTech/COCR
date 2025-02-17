/**********************************************************************
obmolecformat.h - Subclass of OBFormat for conversion of OBMol.

Copyright (C) 2005 Chris Morley

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
#pragma once

#include <unordered_map>
#include <openbabel/babelconfig.h>
#include <openbabel/obconversion.h>
#include <typeinfo>
#include <cstdlib>

namespace OpenBabel {

  class OBMol;
  class OBDescriptor;
  class OBReaction;

/** \class OBMoleculeFormat obmolecformat.h <openbabel/obmolecformat.h>
    \brief An OBFormat convenience subclass for conversion to/from OBMol data

    This class is not intended for direct use outside of Open Babel, unless
    you're writing a new format converting to or from an OBMol molecule.
    (e.g., see http://openbabel.org/wiki/HowTo:Add_A_New_File_Format).

    An OBFormat which converts to and/or from OBMol can derive from this class
    to save duplicating the ReadChemObject() and/or WriteChemObject() methods.
    Derive directly from OBFormat if the object converted is not OBMol or
    if interaction with the framework is required during the execution
    of ReadMolecule() or WriteMolecule(), as for example in CMLFormat
**/

//////////////////////////////////////////////////////////////////////

class OB_EXPORT OBMoleculeFormat : public OBFormat
{
public:

  OBMoleculeFormat()
  {
    if(!OptionsRegistered)
    {
      OptionsRegistered=true;
      OBConversion::RegisterOptionParam("b",         this, 0, OBConversion::INOPTIONS);
      OBConversion::RegisterOptionParam("s",         this, 0, OBConversion::INOPTIONS);
      OBConversion::RegisterOptionParam("title",     this, 1, OBConversion::GENOPTIONS);
      OBConversion::RegisterOptionParam("addtotitle",this, 1, OBConversion::GENOPTIONS);
      OBConversion::RegisterOptionParam("property",  this, 2, OBConversion::GENOPTIONS);
      OBConversion::RegisterOptionParam("C",         this, 0, OBConversion::GENOPTIONS);
      OBConversion::RegisterOptionParam("j",         this, 0, OBConversion::GENOPTIONS);
      OBConversion::RegisterOptionParam("join",      this, 0, OBConversion::GENOPTIONS);
      OBConversion::RegisterOptionParam("separate",  this, 0, OBConversion::GENOPTIONS);

      //The follow are OBMol options, which should not be in OBConversion.
      //But here isn't entirely appropriate either, since one could have
      //OBMol formats loaded but which don't derived from this class.
      //However, this possibility is remote.
      OBConversion::RegisterOptionParam("s", nullptr, 1,OBConversion::GENOPTIONS);
      OBConversion::RegisterOptionParam("v", nullptr, 1,OBConversion::GENOPTIONS);
      OBConversion::RegisterOptionParam("h", nullptr, 0,OBConversion::GENOPTIONS);
      OBConversion::RegisterOptionParam("d", nullptr, 0,OBConversion::GENOPTIONS);
      OBConversion::RegisterOptionParam("b", nullptr, 0,OBConversion::GENOPTIONS);
      OBConversion::RegisterOptionParam("c", nullptr, 0,OBConversion::GENOPTIONS);
      OBConversion::RegisterOptionParam("p", nullptr, 1,OBConversion::GENOPTIONS);
      OBConversion::RegisterOptionParam("t", nullptr, 0,OBConversion::GENOPTIONS);
      OBConversion::RegisterOptionParam("k", nullptr, 0,OBConversion::GENOPTIONS);
      OBConversion::RegisterOptionParam("filter", nullptr, 1,OBConversion::GENOPTIONS);
      OBConversion::RegisterOptionParam("add", nullptr, 1,OBConversion::GENOPTIONS);
      OBConversion::RegisterOptionParam("delete", nullptr, 1,OBConversion::GENOPTIONS);
      OBConversion::RegisterOptionParam("append", nullptr, 1,OBConversion::GENOPTIONS);
    }
  }

  //! Static routine,  which can be called from elsewhere
  static bool ReadChemObjectImpl(OBConversion* pConv, OBFormat*);
  //! Static routine,  which can be called from elsewhere
  static bool WriteChemObjectImpl(OBConversion* pConv, OBFormat*);

  /// The "Convert" interface for reading a new molecule
  virtual bool ReadChemObject(OBConversion* pConv)
  { return ReadChemObjectImpl(pConv, this);}

  /// The "Convert" interface for writing a new molecule
  virtual bool WriteChemObject(OBConversion* pConv)
  { return WriteChemObjectImpl(pConv, this);}

  ///Applies output options to molecule. Returns false to terminate output.
  static bool DoOutputOptions(OBBase* pOb, OBConversion* pConv);

  /// \name Routines to handle the -C option for combining data from several OBMols
  //@{
  //! Defer output of a molecule until later, so it can be combined with others
  //! \return Success, or false if no molecule was read.
  static bool   DeferMolOutput(OBMol* pmol, OBConversion* pConv, OBFormat* pF);
  //! Write out all molecules queued with DeferMolOutput
  static bool   OutputDeferredMols(OBConversion* pConv);
  //! Delete the list of queued molecules from DeferMolOutput
  static bool   DeleteDeferredMols();
  //! \return the OBMol which combines @p pFirst and @p pSecond (i.e.)
  static OBMol* MakeCombinedMolecule(OBMol* pFirst, OBMol* pSecond);
  //@}

  //!When sent an OBReaction object, output all the constituent molecules
  static bool OutputMolsFromReaction
    (OBReaction* pReact, OBConversion* pConv, OBFormat* pFormat);

  typedef std::unordered_map<std::string, unsigned> NameIndexType;

  // documentation in obmolecformat.cpp
  static bool   ReadNameIndex(NameIndexType& index, const std::string& datafilename,
                  OBFormat* pInFormat);

  //! \return the type of data converted by this format (here, OBMol)
  const std::type_info& GetType()
  {
    return typeid(OBMol*);
  }

private:

  static bool OptionsRegistered;
  static std::map<std::string, OBMol*> IMols;
  static OBMol* _jmol; //!< Accumulates molecules with the -j option
  static std::vector<OBMol> MolArray; //!< Used in --separate option
  static bool StoredMolsReady; //!< Used in --separate option
  static OBDescriptor* _pDesc;

};

}
//! \file obmolecformat.h
//! \brief Subclass of OBFormat for conversion of OBMol.