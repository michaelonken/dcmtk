/*
 *
 *  Copyright (C) 2024, OFFIS e.V.
 *  All rights reserved.  See COPYRIGHT file for details.
 *
 *  This software and supporting documentation were developed by
 *
 *    OFFIS e.V.
 *    R&D Division Health
 *    Escherweg 2
 *    D-26121 Oldenburg, Germany
 *
 *
 *  Module:  dcmiod
 *
 *  Author:  Michael Onken
 *
 *  Purpose: Tests for dcmiod's checks
 *
 */

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */
#include "dcmtk/dcmdata/dcuid.h"
#include "dcmtk/oflog/oflog.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmiod/modpalettecolorlut.h"
#include "dcmtk/ofstd/oftest.h"
#include "dcmtk/ofstd/oflimits.h"

template<typename T> static T* makePixelData(unsigned long& num_entries);
template<typename T> static OFBool verifyPixelData(T* dataFound, T* data, const unsigned long& num_entries);

static void checkNonSegmentedPaletteModule(IODPaletteColorLUTModule& mod, OFBool makeSigned, Uint8 bits, const Uint16 numEntries, const Uint16*& data);
static void createNonSegmentedPaletteModule(IODPaletteColorLUTModule& mod, OFBool makeSigned, Uint8 bits, const Uint16 numEntries, const Uint16*& data);

static OFLogger tLog = OFLog::getLogger("dcmtk.test.tpalette");

/** Setup that is run for all test cases.
 *  @param  module The module to set up
 *  @return OFTrue if setup was successful, OFFalse otherwise
 *
 */
static OFBool checkDictionary()
{
     // Make sure data dictionary is loaded
    if (!dcmDataDict.isDictionaryLoaded())
    {
        OFCHECK_FAIL("no data dictionary loaded, check environment variable: " DCM_DICT_ENVIRONMENT_VARIABLE);
        return OFFalse;
    }
    return OFTrue;
}



OFTEST(dcmiod_palette_color_lut_module)
{
    OFCHECK(checkDictionary());
    IODPaletteColorLUTModule mod;
    unsigned long num_entries = 65535;
    const Uint16* data = makePixelData<Uint16>(num_entries);
    createNonSegmentedPaletteModule(mod, OFFalse, 16, num_entries, data);
    checkNonSegmentedPaletteModule(mod, OFFalse, 16, num_entries, data);

}


template<typename T>
static T* makePixelData(unsigned long& num_entries)
{
    T* data = new T[num_entries];
    for (unsigned int i = 0; i < num_entries; i++)
    {
        if (OFnumeric_limits<T>::is_signed)
            data[i] = i - num_entries / 2;
        else
            data[i] = i;
    }
    return data;
}

template<typename T>
static OFBool verifyPixelData(T* dataFound, T* data, const unsigned long& num_entries)
{
    for (unsigned int i = 0; i < num_entries; i++)
    {
        if (dataFound[i] != data[i])
        {
            OFLOG_DEBUG(tLog, "Pixel data at index " << i << " is " << dataFound[i] << " but should be " << data[i]);
            return OFFalse;
        }
    }
    return OFTrue;
}


static void createNonSegmentedPaletteModule(IODPaletteColorLUTModule& mod, OFBool makeSigned, Uint8 bits, const Uint16 numEntries, const Uint16*& data)
{
    OFCHECK_MSG(mod.setRedPaletteColorLookupTableData(data, numEntries).good(), "Cannot set Red Palette Color Lookup Table Data");
    OFCHECK_MSG(mod.setGreenPaletteColorLookupTableData(data, numEntries).good(), "Cannot set Green Palette Color Lookup Table Data");
    OFCHECK_MSG(mod.setBluePaletteColorLookupTableData(data, numEntries).good(), "Cannot set Blue Palette Color Lookup Table Data");
    OFCHECK_MSG(mod.setRedPaletteColorLookupTableDescriptor(numEntries, (Uint16)0, 16).good(), "Cannot set Red Palette Color Lookup Table Descriptor (0,0,16)");
    OFCHECK_MSG(mod.setGreenPaletteColorLookupTableDescriptor(numEntries, (Uint16)0, 16).good(), "Cannot set Green Palette Color Lookup Table Descriptor (0,0,16)");
    OFCHECK_MSG(mod.setBluePaletteColorLookupTableDescriptor(numEntries, (Uint16)0, 16).good(), "Cannot set Blue Palette Color Lookup Table Descriptor (0,0,16)");
    OFCHECK_MSG(mod.setPaletteColorLookupTableUID("1.2.3.4").good(), "Cannot set Palette Color Lookup Table UID (1.2.3.4)");
}


static void checkNonSegmentedPaletteModule(IODPaletteColorLUTModule& mod, OFBool isSigned, Uint8 bits, const Uint16 numEntries, const Uint16*& data)
{
    DcmItem item;
    OFString uid;
    const Uint16 *dataFound = NULL;
    size_t entriesFound = 0;

    OFLOG_DEBUG(tLog, "Writing Palette Color Lookup Table Module to DcmItem, configuration is " << (isSigned ? "signed" : "unsigned") << " with " << (Uint16)bits << " bits");
    OFCHECK_MSG(mod.write(item).good(), "Cannot write Palette Color Lookup Table Module to DcmItem");
    //item.print(std::cout);
    OFCHECK_MSG(mod.isSigned() == isSigned, "isSigned() returns wrong value for Palette Color Lookup Table Module");
    OFCHECK_MSG(mod.numBits() == bits, "getBits() returns wrong value for Palette Color Lookup Table Module");
    OFCHECK_MSG(mod.getPaletteColorLookupTableUID(uid).good(), "Cannot get Palette Color Lookup Table UID");
    OFCHECK(uid == "1.2.3.4");
    OFCHECK(mod.getRedPaletteColorLookupTableData(dataFound, entriesFound).good());
    OFCHECK(entriesFound == numEntries);
    OFCHECK(verifyPixelData(dataFound, data, numEntries));
}
