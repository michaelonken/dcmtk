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

template<typename T> static T* makePixelData();
static void checkNonSegmentedPaletteModule(IODPaletteColorLUTModule& mod, OFBool isSigned, Uint8 bits);
static void createNonSegmentedPaletteModule(IODPaletteColorLUTModule& mod, OFBool makeSigned, Uint8 bits);

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
    createNonSegmentedPaletteModule(mod, OFFalse, 16);
    checkNonSegmentedPaletteModule(mod, OFFalse, 16);

}


template<typename T>
static T* makePixelData()
{
    T* data = new T[256];
    for (int i = 0; i < 256; i++)
    {
        if (OFnumeric_limits<T>::is_signed)
            data[i] = i - 128;
        else
            data[i] = i;
    }
    return data;
}

static void createNonSegmentedPaletteModule(IODPaletteColorLUTModule& mod, OFBool makeSigned, Uint8 bits)
{
    Uint16 NUM_ENTRIES_16_BIT = 1000;
    Uint16 *data = new Uint16[1000];
    for (int i = 0; i < NUM_ENTRIES_16_BIT; i++)
    {
        data[i] = i;
    }
    OFCHECK_MSG(mod.setRedPaletteColorLookupTableData(data, NUM_ENTRIES_16_BIT).good(), "Cannot set Red Palette Color Lookup Table Data");
    OFCHECK_MSG(mod.setGreenPaletteColorLookupTableData(data, NUM_ENTRIES_16_BIT).good(), "Cannot set Green Palette Color Lookup Table Data");
    OFCHECK_MSG(mod.setBluePaletteColorLookupTableData(data, NUM_ENTRIES_16_BIT).good(), "Cannot set Blue Palette Color Lookup Table Data");
    OFCHECK_MSG(mod.setRedPaletteColorLookupTableDescriptor(NUM_ENTRIES_16_BIT, (Uint16)0, 16).good(), "Cannot set Red Palette Color Lookup Table Descriptor (0,0,16)");
    OFCHECK_MSG(mod.setGreenPaletteColorLookupTableDescriptor(NUM_ENTRIES_16_BIT, (Uint16)0, 16).good(), "Cannot set Green Palette Color Lookup Table Descriptor (0,0,16)");
    OFCHECK_MSG(mod.setBluePaletteColorLookupTableDescriptor(NUM_ENTRIES_16_BIT, (Uint16)0, 16).good(), "Cannot set Blue Palette Color Lookup Table Descriptor (0,0,16)");
    OFCHECK_MSG(mod.setPaletteColorLookupTableUID("1.2.3.4").good(), "Cannot set Palette Color Lookup Table UID (1.2.3.4)");
}


static void checkNonSegmentedPaletteModule(IODPaletteColorLUTModule& mod, OFBool isSigned, Uint8 bits)
{
    DcmItem item;
    OFLOG_DEBUG(tLog, "Writing Palette Color Lookup Table Module to DcmItem, configuration is " << (isSigned ? "signed" : "unsigned") << " with " << (Uint16)bits << " bits");
    OFCHECK_MSG(mod.write(item).good(), "Cannot write Palette Color Lookup Table Module to DcmItem");
    item.print(std::cout);
    OFCHECK_MSG(mod.isSigned() == isSigned, "isSigned() returns wrong value for Palette Color Lookup Table Module");
    OFCHECK_MSG(mod.numBits() == bits, "getBits() returns wrong value for Palette Color Lookup Table Module");
}
