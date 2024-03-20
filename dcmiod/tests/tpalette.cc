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
template<typename T> static void checkNonSegmentedPaletteModule(IODPaletteColorLUTModule& mod, Uint8 bits, const Uint16 numEntries, const T*& data);
template<typename T> static void createNonSegmentedPaletteModule(IODPaletteColorLUTModule& mod, Uint8 bits, const Uint16 numEntries, const T*& data);

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
    createNonSegmentedPaletteModule(mod, num_bits, num_entries, data16bit);
    checkNonSegmentedPaletteModule(mod, num_bits, num_entries, data16bit);
    clear(mod);
    delete[] data16bit;

    // 16 bit max entries
    num_entries = 65536;
    num_bits = 16;
    const Uint16* data16bitMax = makePixelData<Uint16>(num_entries);
    createNonSegmentedPaletteModule(mod, num_bits, 0, data16bitMax);
    checkNonSegmentedPaletteModule(mod, num_bits, 0, data16bitMax);
    clear(mod);
    delete[] data16bitMax;

    // 8 bit data
    num_entries = 255;
    num_bits = 8;
    const Uint8* data8bit = makePixelData<Uint8>(num_entries);
    createNonSegmentedPaletteModule(mod, num_bits, num_entries, data8bit);
    checkNonSegmentedPaletteModule(mod, num_bits, num_entries, data8bit);
    clear(mod);
    delete[] data8bit;

    // 8 bit max entries
    num_entries = 256;
    num_bits = 8;
    const Uint8* data8bitMax = makePixelData<Uint8>(num_entries);
    createNonSegmentedPaletteModule(mod, num_bits, 0, data8bitMax);
    checkNonSegmentedPaletteModule(mod, num_bits, 0, data8bitMax);
    clear(mod);
    delete[] data8bitMax;
}

OFTEST(dcmiod_palette_color_lut_module_extra_checks)
{
    // 16 bit data with 8 bit descriptor fails
    IODPaletteColorLUTModule mod;
    unsigned long num_entries = 255;
    const Uint16* data16bit = makePixelData<Uint16>(num_entries);
    OFCHECK(mod.setRedPaletteColorLookupTableDescriptor(num_entries, 0, 8).good());
    OFCHECK(mod.setGreenPaletteColorLookupTableDescriptor(num_entries, 0, 8).good());
    OFCHECK(mod.setBluePaletteColorLookupTableDescriptor(num_entries, 0, 8).good());
    // Now try to set 16 LUT Data
    OFCHECK(mod.setRedPaletteColorLookupTableData(data16bit, num_entries).bad());
    OFCHECK(mod.setGreenPaletteColorLookupTableData(data16bit, num_entries).bad());
    OFCHECK(mod.setBluePaletteColorLookupTableData(data16bit, num_entries).bad());
    delete[] data16bit;
    mod.clearData();

    // 8 bit data with 16 bit descriptor fails
    num_entries = 65535;
    const Uint8* data8bit = makePixelData<Uint8>(num_entries);
    OFCHECK(mod.setRedPaletteColorLookupTableDescriptor(num_entries, 0, 16).good());
    OFCHECK(mod.setGreenPaletteColorLookupTableDescriptor(num_entries, 0, 16).good());
    OFCHECK(mod.setBluePaletteColorLookupTableDescriptor(num_entries, 0, 16).good());
    // Now try to set 8 LUT Data
    OFCHECK(mod.setRedPaletteColorLookupTableData(data8bit, num_entries).bad());
    OFCHECK(mod.setGreenPaletteColorLookupTableData(data8bit, num_entries).bad());
    OFCHECK(mod.setBluePaletteColorLookupTableData(data8bit, num_entries).bad());
    delete[] data8bit;

    // mix of segment and non segment data fails
    num_entries = 65535;
    DcmItem item;
    data16bit = makePixelData<Uint16>(num_entries);
    OFCHECK(mod.setRedPaletteColorLookupTableDescriptor(num_entries, 0, 16).good());
    OFCHECK(mod.setGreenPaletteColorLookupTableDescriptor(num_entries, 0, 16).good());
    OFCHECK(mod.setBluePaletteColorLookupTableDescriptor(num_entries, 0, 16).good());
    OFCHECK(mod.setSegmentedRedPaletteColorLookupTableData(data16bit, num_entries).good());
    OFCHECK(mod.setSegmentedGreenPaletteColorLookupTableData(data16bit, num_entries).good());
    OFCHECK(mod.setSegmentedBluePaletteColorLookupTableData(data16bit, num_entries).good());
    // this does not make sense:
    OFCHECK(mod.setSegmentedRedPaletteColorLookupTableData(data16bit, num_entries).good());
    OFCHECK(mod.setSegmentedGreenPaletteColorLookupTableData(data16bit, num_entries).good());
    OFCHECK(mod.setSegmentedBluePaletteColorLookupTableData(data16bit, num_entries).good());
    OFCHECK(mod.write(item).bad());
    delete[] data16bit;

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
static void createNonSegmentedPaletteModule(IODPaletteColorLUTModule& mod, Uint8 bits, const Uint16 numEntries, const T*& data)
{
    OFCHECK_MSG(mod.setRedPaletteColorLookupTableDescriptor(numEntries, 0, bits).good(), "Cannot set Red Palette Color Lookup Table Descriptor");
    OFCHECK_MSG(mod.setGreenPaletteColorLookupTableDescriptor(numEntries, 0, bits).good(), "Cannot set Green Palette Color Lookup Table Descriptor");
    OFCHECK_MSG(mod.setBluePaletteColorLookupTableDescriptor(numEntries, 0, bits).good(), "Cannot set Blue Palette Color Lookup Table Descriptor");
    OFCHECK_MSG(mod.setPaletteColorLookupTableUID("1.2.3.4").good(), "Cannot set Palette Color Lookup Table UID (1.2.3.4)");

    OFCHECK_MSG(mod.setRedPaletteColorLookupTableData(data, numEntries).good(), "Cannot set Red Palette Color Lookup Table Data");
    OFCHECK_MSG(mod.setGreenPaletteColorLookupTableData(data, numEntries).good(), "Cannot set Green Palette Color Lookup Table Data");
    OFCHECK_MSG(mod.setBluePaletteColorLookupTableData(data, numEntries).good(), "Cannot set Blue Palette Color Lookup Table Data");
}


template<typename T>
static void checkNonSegmentedPaletteModule(IODPaletteColorLUTModule& mod, Uint8 bits, const Uint16 numEntries, const T*& data)
{
    DcmItem item;
    OFString uid;
    size_t entriesFound = 0;
    Uint16 d1,d2,d3;
    d1 = d2 = d3 = 1; // not used in test data
    OFCHECK_MSG(mod.write(item).good(), "Cannot write Palette Color Lookup Table Module to DcmItem");
    OFCHECK_MSG(mod.numBits() == bits, "getBits() returns wrong value for Palette Color Lookup Table Module");
    OFCHECK_MSG(mod.getPaletteColorLookupTableUID(uid).good(), "Cannot get Palette Color Lookup Table UID");
    OFCHECK(mod.getRedPaletteColorLookupTableDescriptor(d1, 0).good());
    OFCHECK(mod.getRedPaletteColorLookupTableDescriptor(d2, 1).good());
    OFCHECK(mod.getRedPaletteColorLookupTableDescriptor(d3, 2).good());
    OFCHECK(d1 == numEntries);
    OFCHECK(d2 == 0);
    OFCHECK(d3 == bits);
    d1 = d2 = d3 = 1; // not used in test data
    OFCHECK(mod.getGreenPaletteColorLookupTableDescriptor(d1, 0).good());
    OFCHECK(mod.getGreenPaletteColorLookupTableDescriptor(d2, 1).good());
    OFCHECK(mod.getGreenPaletteColorLookupTableDescriptor(d3, 2).good());
    OFCHECK(d1 == numEntries);
    OFCHECK(d2 == 0);
    OFCHECK(d3 == bits);
    d1 = d2 = d3 = 1; // not used in test data
    OFCHECK(mod.getBluePaletteColorLookupTableDescriptor(d1, 0).good());
    OFCHECK(mod.getBluePaletteColorLookupTableDescriptor(d2, 1).good());
    OFCHECK(mod.getBluePaletteColorLookupTableDescriptor(d3, 2).good());
    OFCHECK(d1 == numEntries);
    OFCHECK(d2 == 0);
    OFCHECK(d3 == bits);

    OFCHECK(uid == "1.2.3.4");
    // 16 bit data
    if (bits == 16)
    {
        const Uint16 *dataFound16 = NULL;
        OFCHECK(mod.getRedPaletteColorLookupTableData(dataFound16, entriesFound).good());
        OFCHECK(entriesFound == ((numEntries == 0) ? 65536 : numEntries));
        OFCHECK(verifyPixelData(dataFound16, (const Uint16*)data, numEntries));
        delete[] dataFound16; dataFound16 = NULL;
        OFCHECK(mod.getGreenPaletteColorLookupTableData(dataFound16, entriesFound).good());
        OFCHECK(entriesFound == ((numEntries == 0) ? 65536 : numEntries));
        OFCHECK(verifyPixelData(dataFound16, (const Uint16*)data, numEntries));
        delete[] dataFound16; dataFound16 = NULL;
        OFCHECK(mod.getBluePaletteColorLookupTableData(dataFound16, entriesFound).good());
        OFCHECK(entriesFound == ((numEntries == 0) ? 65536 : numEntries));
        OFCHECK(verifyPixelData(dataFound16, (const Uint16*)data, numEntries));
        delete[] dataFound16;
    }
    // 8 bit data
    else if (bits == 8)
    {
        const Uint8 *dataFound8 = NULL;
        OFCHECK(mod.getRedPaletteColorLookupTableData(dataFound8, entriesFound).good());
        OFCHECK(entriesFound == ((numEntries == 0) ? 256 : numEntries));
        OFCHECK(verifyPixelData(dataFound8, (const Uint8*)data, numEntries));
        delete[] dataFound8; dataFound8 = NULL;
        OFCHECK(mod.getGreenPaletteColorLookupTableData(dataFound8, entriesFound).good());
        OFCHECK(entriesFound == ((numEntries == 0) ? 256 : numEntries));
        OFCHECK(verifyPixelData(dataFound8, (const Uint8*)data, numEntries));
        delete[] dataFound8; dataFound8 = NULL;
        OFCHECK(mod.getBluePaletteColorLookupTableData(dataFound8, entriesFound).good());
        OFCHECK(entriesFound == ((numEntries == 0) ? 256 : numEntries));
        OFCHECK(verifyPixelData(dataFound8, (const Uint8*)data, numEntries));
        delete[] dataFound8;
    }
    else
    {
        OFCHECK_FAIL("Unsupported bit depth");
    }
}
