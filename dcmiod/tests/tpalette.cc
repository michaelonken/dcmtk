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
#include "dcmtk/oflog/oflog.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmiod/modpalettecolorlut.h"
#include "dcmtk/ofstd/oftest.h"
#include "dcmtk/ofstd/oflimits.h"

template<typename T> static T* makePixelData(unsigned long& num_entries);
template<typename T> static OFBool verifyPixelData(T* dataFound, T* data, const unsigned long& num_entries);
template<typename T> static void checkNonSegmentedPaletteModule(IODPaletteColorLUTModule& mod, OFBool makeSigned, Uint8 bits, const Uint16 numEntries, const T*& data);
template<typename T> static void createNonSegmentedPaletteModule(IODPaletteColorLUTModule& mod, OFBool makeSigned, Uint8 bits, const Uint16 numEntries, const T*& data);

static void clear(IODPaletteColorLUTModule& mod);

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

    // 16 Bit data
    unsigned long num_entries = 65535;
    Uint8 num_bits = 16;
    const Uint16* data16bit = makePixelData<Uint16>(num_entries);
    createNonSegmentedPaletteModule(mod, OFFalse, num_bits, num_entries, data16bit);
    checkNonSegmentedPaletteModule(mod, OFFalse, num_bits, num_entries, data16bit);
    clear(mod);
    delete[] data16bit;

    // 8 bit data
    num_entries = 255;
    num_bits = 8;
    const Uint8* data8bit = makePixelData<Uint8>(num_entries);
    createNonSegmentedPaletteModule(mod, OFFalse, num_bits, num_entries, data8bit);
    checkNonSegmentedPaletteModule(mod, OFFalse, num_bits, num_entries, data8bit);
    clear(mod);
    delete[] data8bit;
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
    if (!data || !dataFound)
    {
        return OFFalse;
    }

    for (unsigned int i = 0; i < num_entries; i++)
    {
        if (dataFound[i] != data[i])
        {
            OFLOG_DEBUG(tLog, "Pixel data at index " << (Uint16)i << " is " << (Uint16)dataFound[i] << " but should be " << (Uint16)(data[i]));
            return OFFalse;
        }
    }
    return OFTrue;
}

static void clear(IODPaletteColorLUTModule& mod)
{
    mod.clearData();
    OFCHECK(mod.numBits() == 0);
}


template<typename T>
static void createNonSegmentedPaletteModule(IODPaletteColorLUTModule& mod, OFBool makeSigned, Uint8 bits, const Uint16 numEntries, const T*& data)
{
    OFCHECK_MSG(mod.setRedPaletteColorLookupTableDescriptor(numEntries, (Uint16)0, bits).good(), "Cannot set Red Palette Color Lookup Table Descriptor");
    OFCHECK_MSG(mod.setGreenPaletteColorLookupTableDescriptor(numEntries, (Uint16)0, bits).good(), "Cannot set Green Palette Color Lookup Table Descriptor");
    OFCHECK_MSG(mod.setBluePaletteColorLookupTableDescriptor(numEntries, (Uint16)0, bits).good(), "Cannot set Blue Palette Color Lookup Table Descriptor");
    OFCHECK_MSG(mod.setPaletteColorLookupTableUID("1.2.3.4").good(), "Cannot set Palette Color Lookup Table UID (1.2.3.4)");

    OFCHECK_MSG(mod.setRedPaletteColorLookupTableData(data, numEntries).good(), "Cannot set Red Palette Color Lookup Table Data");
    OFCHECK_MSG(mod.setGreenPaletteColorLookupTableData(data, numEntries).good(), "Cannot set Green Palette Color Lookup Table Data");
    OFCHECK_MSG(mod.setBluePaletteColorLookupTableData(data, numEntries).good(), "Cannot set Blue Palette Color Lookup Table Data");
}


template<typename T>
static void checkNonSegmentedPaletteModule(IODPaletteColorLUTModule& mod, OFBool isSigned, Uint8 bits, const Uint16 numEntries, const T*& data)
{
    OFLOG_DEBUG(tLog, "Checking non-segmented palette color with " << (isSigned ? "signed" : "unsigned") << " " << (Uint16)bits << " bit data and " << numEntries << " entries.");
    DcmItem item;
    OFString uid;
    size_t entriesFound = 0;
    OFCHECK_MSG(mod.write(item).good(), "Cannot write Palette Color Lookup Table Module to DcmItem");
    OFCHECK_MSG(mod.numBits() == bits, "getBits() returns wrong value for Palette Color Lookup Table Module");
    OFCHECK_MSG(mod.getPaletteColorLookupTableUID(uid).good(), "Cannot get Palette Color Lookup Table UID");
    OFCHECK(uid == "1.2.3.4");
    // Uint16 / Sint16
    if (bits == 16)
    {
        const Uint16 *dataFound16 = NULL;
        OFCHECK(mod.getRedPaletteColorLookupTableData(dataFound16, entriesFound).good());
        OFCHECK(entriesFound == numEntries);
        OFCHECK(verifyPixelData(dataFound16, (const Uint16*)data, numEntries));
        delete[] dataFound16;
    }
    // Uint8 / Sint8
    else if (bits == 8)
    {
        const Uint8 *dataFound8 = NULL;
        OFCHECK(mod.getRedPaletteColorLookupTableData(dataFound8, entriesFound).good());
        OFCHECK(entriesFound == numEntries);
        OFCHECK(verifyPixelData(dataFound8, (const Uint8*)data, numEntries));
        delete[] dataFound8;
    }
    else
    {
        OFCHECK_FAIL("Unsupported bit depth");
    }
}
