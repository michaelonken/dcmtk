/*
 *
 *  Copyright (C) 2015-2024, Open Connections GmbH
 *  All rights reserved.  See COPYRIGHT file for details.
 *
 *  This software and supporting documentation are maintained by
 *
 *    OFFIS e.V.
 *    R&D Division Health
 *    Escherweg 2
 *    D-26121 Oldenburg, Germany
 *
 *
 *  Module: dcmiod
 *
 *  Author: Michael Onken
 *
 *  Purpose: Class for managing the Palette Color LUT Module
 *
 */

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */

#include "dcmtk/dcmdata/dcelem.h"
#include "dcmtk/dcmdata/dcvrobow.h"
#include "dcmtk/dcmdata/dcvrui.h"
#include "dcmtk/dcmiod/iodtypes.h"
#include "dcmtk/dcmiod/modpalettecolorlut.h"
#include "dcmtk/ofstd/ofcond.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmdata/dcvrus.h"
#include "dcmtk/dcmiod/iodutil.h"
#include "dcmtk/dcmimgle/diluptab.h"
#include "dcmtk/ofstd/oftypes.h"

const OFString IODPaletteColorLUTModule::m_ModuleName = "PaletteColorLookupTableModule";

IODPaletteColorLUTModule::IODPaletteColorLUTModule(OFshared_ptr<DcmItem> item, OFshared_ptr<IODRules> rules)
    : IODModule(item, rules)
    , m_IsSigned(OFFalse)
{
    // reset element rules
    resetRules();
}

IODPaletteColorLUTModule::IODPaletteColorLUTModule()
    : IODModule()
{
    resetRules();
}


IODPaletteColorLUTModule::~IODPaletteColorLUTModule()
{
    // nothing to do
}

void IODPaletteColorLUTModule::resetRules()
{
    m_Rules->addRule(new IODRule(DCM_RedPaletteColorLookupTableDescriptor, "3", "1", getName(), DcmIODTypes::IE_IMAGE),
                     OFTrue);
    m_Rules->addRule(
        new IODRule(DCM_GreenPaletteColorLookupTableDescriptor, "3", "1", getName(), DcmIODTypes::IE_IMAGE), OFTrue);
    m_Rules->addRule(new IODRule(DCM_BluePaletteColorLookupTableDescriptor, "3", "1", getName(), DcmIODTypes::IE_IMAGE),
                     OFTrue);
    m_Rules->addRule(new IODRule(DCM_PaletteColorLookupTableUID, "1", "1", getName(), DcmIODTypes::IE_IMAGE), OFTrue);
    m_Rules->addRule(new IODRule(DCM_RedPaletteColorLookupTableData, "1", "1C", getName(), DcmIODTypes::IE_IMAGE),
                     OFTrue);
    m_Rules->addRule(new IODRule(DCM_GreenPaletteColorLookupTableData, "1", "1C", getName(), DcmIODTypes::IE_IMAGE),
                     OFTrue);
    m_Rules->addRule(new IODRule(DCM_BluePaletteColorLookupTableData, "1", "1C", getName(), DcmIODTypes::IE_IMAGE),
                     OFTrue);
    m_Rules->addRule(
        new IODRule(DCM_SegmentedRedPaletteColorLookupTableData, "1", "1C", getName(), DcmIODTypes::IE_IMAGE), OFTrue);
    m_Rules->addRule(
        new IODRule(DCM_SegmentedGreenPaletteColorLookupTableData, "1", "1C", getName(), DcmIODTypes::IE_IMAGE),
        OFTrue);
    m_Rules->addRule(
        new IODRule(DCM_SegmentedBluePaletteColorLookupTableData, "1", "1C", getName(), DcmIODTypes::IE_IMAGE), OFTrue);
}

OFString IODPaletteColorLUTModule::getName() const
{
    return m_ModuleName;
}

OFCondition IODPaletteColorLUTModule::read(DcmItem& source, const OFBool clearOldData)
{
    if (clearOldData)
        clearData();

    IODComponent::read(source, OFFalse /* data already cleared */);
    checkLUT(DCM_RedPaletteColorLookupTableDescriptor, DCM_RedPaletteColorLookupTableData);
    checkLUT(DCM_GreenPaletteColorLookupTableDescriptor, DCM_GreenPaletteColorLookupTableData);
    checkLUT(DCM_BluePaletteColorLookupTableDescriptor, DCM_BluePaletteColorLookupTableData);
    checkDescriptorConsistency(OFFalse /* only warn */);
    checkSegmentConditions(OFFalse);
    return EC_Normal;
}

OFCondition IODPaletteColorLUTModule::write(DcmItem& destination)
{
    OFBool valid(OFTrue);
    valid = checkLUT(DCM_RedPaletteColorLookupTableDescriptor, DCM_RedPaletteColorLookupTableData);
    if (valid)
        valid = checkLUT(DCM_GreenPaletteColorLookupTableDescriptor, DCM_GreenPaletteColorLookupTableData);
    if (valid)
        valid = checkLUT(DCM_BluePaletteColorLookupTableDescriptor, DCM_BluePaletteColorLookupTableData);
    if (!valid)
        return IOD_EC_InvalidColorPalette;
    valid = checkDescriptorConsistency(OFTrue /* report as errors */);
    return IODComponent::write(destination);
}

template <>
OFCondition IODPaletteColorLUTModule::getRedPaletteColorLookupTableDescriptor(Uint16& value,
                                                                              const unsigned long pos) const
{
    OFCondition result;
    return m_Item->findAndGetUint16(DCM_RedPaletteColorLookupTableDescriptor, value, pos);
}

template <>
OFCondition IODPaletteColorLUTModule::getGreenPaletteColorLookupTableDescriptor(Uint16& value,
                                                                                const unsigned long pos) const
{
    return m_Item->findAndGetUint16(DCM_GreenPaletteColorLookupTableDescriptor, value, pos);
}

template <>
OFCondition IODPaletteColorLUTModule::getBluePaletteColorLookupTableDescriptor(Uint16& value,
                                                                               const unsigned long pos) const
{
    return m_Item->findAndGetUint16(DCM_BluePaletteColorLookupTableDescriptor, value, pos);
}

OFCondition IODPaletteColorLUTModule::getPaletteColorLookupTableUID(OFString& value, const signed long pos) const
{
    return DcmIODUtil::getStringValueFromItem(DCM_PaletteColorLookupTableUID, *m_Item, value, pos);
}

OFCondition IODPaletteColorLUTModule::getRedPaletteColorLookupTableData(const Uint16*& data, size_t& numEntries)
{
    return m_Item->findAndGetUint16Array(DCM_RedPaletteColorLookupTableData, data, NULL, OFFalse);
}

OFCondition IODPaletteColorLUTModule::getGreenPaletteColorLookupTableData(const Uint16*& data, size_t& numEntries)
{
    return m_Item->findAndGetUint16Array(DCM_GreenPaletteColorLookupTableData, data, NULL, OFFalse);
}

OFCondition IODPaletteColorLUTModule::getBluePaletteColorLookupTableData(const Uint16*& data, size_t& numEntries)
{
    return m_Item->findAndGetUint16Array(DCM_BluePaletteColorLookupTableData, data, NULL, OFFalse);
}

OFCondition IODPaletteColorLUTModule::getSegmentedRedPaletteColorLookupTableData(const Uint16*& data, size_t& numEntries)
{
    return m_Item->findAndGetUint16Array(DCM_SegmentedRedPaletteColorLookupTableData, data, NULL, OFFalse);
}

OFCondition IODPaletteColorLUTModule::getSegmentedGreenPaletteColorLookupTableData(const Uint16*& data, size_t& numEntries)
{
    return m_Item->findAndGetUint16Array(DCM_SegmentedGreenPaletteColorLookupTableData, data, NULL, OFFalse);
}

OFCondition IODPaletteColorLUTModule::getSegmentedBluePaletteColorLookupTableData(const Uint16*& data, size_t& numEntries)
{
    return m_Item->findAndGetUint16Array(DCM_SegmentedBluePaletteColorLookupTableData, data, NULL, OFFalse);
}

// -------------------- set() --------------------

template <>
OFCondition IODPaletteColorLUTModule::setRedPaletteColorLookupTableDescriptor(const Uint16& value,
                                                                              const unsigned long pos)
{
    OFCondition result = m_Item->putAndInsertUint16(DCM_RedPaletteColorLookupTableDescriptor, value, pos);
    if (result.good())
    {
        m_IsSigned = OFFalse;
    }
    return result;
}

template <>
OFCondition IODPaletteColorLUTModule::setRedPaletteColorLookupTableDescriptor(const Sint16& value,
                                                                              const unsigned long pos)
{
    OFCondition result = m_Item->putAndInsertSint16(DCM_RedPaletteColorLookupTableDescriptor, value, pos);
    if (result.good())
    {
        m_IsSigned = OFTrue;
    }
    return result;
}

template <>
OFCondition IODPaletteColorLUTModule::setGreenPaletteColorLookupTableDescriptor(const Uint16& value,
                                                                                const unsigned long pos)
{
    OFCondition result = m_Item->putAndInsertUint16(DCM_GreenPaletteColorLookupTableDescriptor, value, pos);
    if (result.good())
    {
        m_IsSigned = OFFalse;
    }
    return result;
}

template <>
OFCondition IODPaletteColorLUTModule::setGreenPaletteColorLookupTableDescriptor(const Sint16& value,
                                                                                const unsigned long pos)
{
    OFCondition result = m_Item->putAndInsertSint16(DCM_GreenPaletteColorLookupTableDescriptor, value, pos);
    if (result.good())
    {
        m_IsSigned = OFTrue;
    }
    return result;
}


template <>
OFCondition IODPaletteColorLUTModule::setBluePaletteColorLookupTableDescriptor(const Uint16& value,
                                                                               const unsigned long pos)
{
    OFCondition result = m_Item->putAndInsertUint16(DCM_BluePaletteColorLookupTableDescriptor, value, pos);
    if (result.good())
    {
        m_IsSigned = OFFalse;
    }
    return result;
}

template <>
OFCondition IODPaletteColorLUTModule::setBluePaletteColorLookupTableDescriptor(const Sint16& value,
                                                                               const unsigned long pos)
{
    OFCondition result = m_Item->putAndInsertSint16(DCM_BluePaletteColorLookupTableDescriptor, value, pos);
    if (result.good())
    {
        m_IsSigned = OFTrue;
    }
    return result;
}


OFCondition IODPaletteColorLUTModule::setPaletteColorLookupTableUID(const OFString& value, const OFBool checkValue)
{
    OFCondition result = (checkValue) ? DcmUniqueIdentifier::checkStringValue(value, "1") : EC_Normal;
    if (result.good()) result = m_Item->putAndInsertOFStringArray(DCM_PaletteColorLookupTableUID, value);
    return result;
}

OFCondition IODPaletteColorLUTModule::setRedPaletteColorLookupTableData(const Uint16* data,
                                                                        const size_t numEntries,
                                                                        const OFBool)
{
    return m_Item->putAndInsertUint16Array(DCM_RedPaletteColorLookupTableData, data, numEntries);
}

OFCondition IODPaletteColorLUTModule::setGreenPaletteColorLookupTableData(const Uint16* data,
                                                                          const size_t numEntries,
                                                                          const OFBool)
{
    return m_Item->putAndInsertUint16Array(DCM_GreenPaletteColorLookupTableData, data, numEntries);
}

OFCondition IODPaletteColorLUTModule::setBluePaletteColorLookupTableData(const Uint16* data,
                                                                         const size_t numEntries,
                                                                         const OFBool)
{
    return m_Item->putAndInsertUint16Array(DCM_BluePaletteColorLookupTableData, data, numEntries);
}

OFCondition IODPaletteColorLUTModule::setSegmentedRedPaletteColorLookupTableData(const Uint16* data,
                                                                                 const size_t numEntries,
                                                                                 const OFBool)
{
    return m_Item->putAndInsertUint16Array(DCM_SegmentedRedPaletteColorLookupTableData, data, numEntries);
}

OFCondition IODPaletteColorLUTModule::setSegmentedGreenPaletteColorLookupTableData(const Uint16* data,
                                                                                   const size_t numEntries,
                                                                                   const OFBool)
{
    return m_Item->putAndInsertUint16Array(DCM_SegmentedGreenPaletteColorLookupTableData, data, numEntries);
}

OFCondition IODPaletteColorLUTModule::setSegmentedBluePaletteColorLookupTableData(const Uint16* data,
                                                                                  const size_t numEntries,
                                                                                  const OFBool)
{
    return m_Item->putAndInsertUint16Array(DCM_SegmentedBluePaletteColorLookupTableData, data, numEntries);
}


OFCondition IODPaletteColorLUTModule::setPaletteColorLookupTableData(const Uint16* redData,
                                                                     const Uint16* greenData,
                                                                     const Uint16* blueData,
                                                                     const size_t numEntries,
                                                                     const OFBool checkValue)
{
    OFCondition result = setRedPaletteColorLookupTableData(redData, numEntries, checkValue);
    if (result.good()) result = setGreenPaletteColorLookupTableData(greenData, numEntries, checkValue);
    if (result.good()) result = setBluePaletteColorLookupTableData(blueData, numEntries, checkValue);
    if (result.good())
    {
        m_IsSigned = OFFalse;
    }
    return result;
}

OFCondition IODPaletteColorLUTModule::setSegmentedPaletteColorLookupTableData(const Uint16* redData,
                                                                              const Uint16* greenData,
                                                                              const Uint16* blueData,
                                                                              const size_t numEntries,
                                                                              const OFBool checkValue)
{
    OFCondition result = setSegmentedRedPaletteColorLookupTableData(redData, numEntries, checkValue);
    if (result.good()) result = setSegmentedGreenPaletteColorLookupTableData(greenData, numEntries, checkValue);
    if (result.good()) result = setSegmentedBluePaletteColorLookupTableData(blueData, numEntries, checkValue);
    if (result.good())
    {
        m_IsSigned = OFFalse;
    }
    return result;

}

template <>
OFCondition IODPaletteColorLUTModule::setRedPaletteColorLookupTableDescriptor(const Uint16 numEntries,
                                                                              const Uint16 firstMapped,
                                                                              const Uint8 bitsPerEntry)
{
    // TODO Check value
    OFCondition result;
    Uint16 values[3];
    values[0] = numEntries;
    values[1] = firstMapped;
    values[2] = bitsPerEntry;

    result = m_Item->putAndInsertUint16Array(DCM_RedPaletteColorLookupTableDescriptor, values, 3);
    if (result.good())
    {
        m_IsSigned = OFFalse;
    }
    return result;
}

template <>
OFCondition IODPaletteColorLUTModule::setRedPaletteColorLookupTableDescriptor(const Uint16 numEntries,
                                                                              const Sint16 firstMapped,
                                                                              const Uint8 bitsPerEntry)
{
    // TODO Check value
    OFCondition result;
    Uint16 values[3];
    values[0] = numEntries;
    values[1] = firstMapped;
    values[2] = bitsPerEntry;

    result = m_Item->putAndInsertUint16Array(DCM_RedPaletteColorLookupTableDescriptor, values, 3);
    if (result.good())
    {
        m_IsSigned = OFFalse;
    }
    return result;
}


template <>
OFCondition IODPaletteColorLUTModule::setGreenPaletteColorLookupTableDescriptor(const Uint16 numEntries,
                                                                                const Uint16 firstMapped,
                                                                                const Uint8 bitsPerEntry)
{
    // TODO check value
    OFCondition result;
    Uint16 values[3];
    values[0] = numEntries;
    values[1] = firstMapped;
    values[2] = bitsPerEntry;
    result = m_Item->putAndInsertUint16Array(DCM_GreenPaletteColorLookupTableDescriptor, values, 3);
    if (result.good())
    {
        m_IsSigned = OFFalse;
    }
    return result;
}

template <>
OFCondition IODPaletteColorLUTModule::setGreenPaletteColorLookupTableDescriptor(const Uint16 numEntries,
                                                                                const Sint16 firstMapped,
                                                                                const Uint8 bitsPerEntry)
{
    // TODO check value
    OFCondition result;
    Uint16 values[3];
    values[0] = numEntries;
    values[1] = firstMapped;
    values[2] = bitsPerEntry;
    result = m_Item->putAndInsertUint16Array(DCM_GreenPaletteColorLookupTableDescriptor, values, 3);
    if (result.good())
    {
        m_IsSigned = OFFalse;
    }
    return result;
}


template <>
OFCondition IODPaletteColorLUTModule::setBluePaletteColorLookupTableDescriptor(const Uint16 numEntries,
                                                                               const Uint16 firstMapped,
                                                                               const Uint8 bitsPerEntry)
{
    OFCondition result;
    Uint16 values[3];
    values[0] = numEntries;
    values[1] = firstMapped;
    values[2] = bitsPerEntry;

    result = m_Item->putAndInsertUint16Array(DCM_BluePaletteColorLookupTableDescriptor, values, 3);
    if (result.good())
    {
        m_IsSigned = OFFalse;
    }
    return result;
}

template <>
OFCondition IODPaletteColorLUTModule::setBluePaletteColorLookupTableDescriptor(const Uint16 numEntries,
                                                                               const Sint16 firstMapped,
                                                                               const Uint8 bitsPerEntry)
{
    OFCondition result;
    Uint16 values[3];
    values[0] = numEntries;
    values[1] = firstMapped;
    values[2] = bitsPerEntry;
    result = m_Item->putAndInsertUint16Array(DCM_BluePaletteColorLookupTableDescriptor, values, 3);
    if (result.good())
    {
        m_IsSigned = OFFalse;
    }
    return result;
}


OFBool IODPaletteColorLUTModule::checkLUT(const DcmTagKey& descriptorTag,
                                          const DcmTagKey& dataTag)
{
    DcmElement* data = NULL;
    DcmElement* descriptor = NULL;
    m_Item->findAndGetElement(dataTag, data);
    m_Item->findAndGetElement(descriptorTag, descriptor);
    // LUT Data always OW in this module, LUT Descriptor is always US or SS
    if ( (data->getTag().getEVR() != EVR_OW) || ( (descriptor->getTag().getEVR() != EVR_US) && (descriptor->getTag().getEVR() != EVR_SS)) )
    {
        reportLUTError(descriptorTag, "Palette Color LUT Data or Descriptor have invalid VR", OFFalse /* warning */);
        return OFFalse;
    }
    DiLookupTable lut(*OFstatic_cast(DcmOtherByteOtherWord*, data), *OFstatic_cast(DcmUnsignedShort*, descriptor), NULL, ELM_CheckValue);
    if (!lut.isValid())
    {
        reportLUTError(descriptorTag, "Palette Color LUT Data is invalid", OFFalse /* warning */);
        return OFFalse;
    }
    return OFTrue;
}


void IODPaletteColorLUTModule::reportLUTError(const DcmTagKey& tag, const OFString& message, const OFBool& warning)
{
    OFString color;
    if (tag == DCM_RedPaletteColorLookupTableDescriptor)
        color = "Red";
    else if (tag == DCM_GreenPaletteColorLookupTableDescriptor)
        color = "Green";
    else if (tag == DCM_BluePaletteColorLookupTableDescriptor)
        color = "Blue";
    if (warning)
        DCMIOD_WARN(color << " " << message);
    else
        DCMIOD_ERROR(color << " " << message);
}

OFBool IODPaletteColorLUTModule::isSigned() const
{
    return m_IsSigned;
}


Uint8 IODPaletteColorLUTModule::numBits()
{
    OFCondition result;
    Uint16 numBitsR = 0;
    Uint16 numBitsG = 0;
    Uint16 numBitsB = 0;
    if (getRedPaletteColorLookupTableDescriptor(numBitsR, 2).good() && getGreenPaletteColorLookupTableDescriptor(numBitsG, 2).good() && getBluePaletteColorLookupTableDescriptor(numBitsB, 2).good())
    {
        if ((numBitsR != numBitsG) || (numBitsR != numBitsB) || (numBitsG != numBitsB))
        {
            DCMIOD_WARN("Bits per entry in Red, Green and Blue Palette Color LUT Descriptor are not equal");
            return 0;
        }

    }
    else
    {
        DCMIOD_WARN("Could not read bits per  entry in Red, Green or Blue Palette Color LUT Descriptor");
        return 0;
    }
    if ((numBitsR != 8) && (numBitsR != 16))
    {
        DCMIOD_WARN("Bits per entry in Red, Green and Blue Palette Color LUT Descriptor are greater than 16");
        return 0;
    }
    return OFstatic_cast(Uint8, numBitsR);
}


OFBool IODPaletteColorLUTModule::isSigned(const DcmTagKey& descriptorTag)
{
    OFBool isSigned = OFFalse;
    DcmElement* descriptor = NULL;
    m_Item->findAndGetElement(descriptorTag, descriptor);
    if (descriptor)
    {
        if (descriptor->getTag().getEVR() == EVR_SS)
        {
            isSigned = OFTrue;
        }
    }
    return isSigned;
}


OFBool IODPaletteColorLUTModule::checkDescriptorConsistency(const OFBool& isError)
{
    // Check whether all 2nd values ("first value mapped") have same value.
    Uint16 redFirstMapped = 0;
    Uint16 greenFirstMapped = 0;
    Uint16 blueFirstMapped = 0;
    Sint32 r,g,b;
    r=g=b=0;
    if (getRedPaletteColorLookupTableDescriptor(redFirstMapped, 1).good())
    {
        if (getGreenPaletteColorLookupTableDescriptor(greenFirstMapped, 1).good())
        {
            if (getBluePaletteColorLookupTableDescriptor(blueFirstMapped, 1).good())
            {
                if (isSigned(DCM_RedPaletteColorLookupTableDescriptor))
                {
                    r = OFstatic_cast(Sint16, redFirstMapped);
                }
                else
                {
                    r = redFirstMapped;
                }
                if (isSigned(DCM_GreenPaletteColorLookupTableDescriptor))
                {
                    g = OFstatic_cast(Sint16, greenFirstMapped);
                }
                else
                {
                    g = greenFirstMapped;
                }
                if (isSigned(DCM_BluePaletteColorLookupTableDescriptor))
                {
                    b = OFstatic_cast(Sint16, blueFirstMapped);
                }
                else
                {
                    b = blueFirstMapped;
                }
                if ( (r != g) || (r != b) || (g != b) )
                {
                    if (isError)
                    {
                        DCMIOD_ERROR("First value mapped in Red, Green and Blue Palette Color LUT Descriptor are not equal");
                    }
                    else
                    {
                        DCMIOD_WARN("First value mapped in Red, Green and Blue Palette Color LUT Descriptor are not equal");
                    }
                    return OFFalse;
                }
                Uint16 lastRed, lastGreen, lastBlue;
                lastRed = lastGreen = lastBlue = 0;
                if (getRedPaletteColorLookupTableDescriptor(lastRed, 2).good() && getGreenPaletteColorLookupTableDescriptor(lastGreen, 2).good() && getBluePaletteColorLookupTableDescriptor(lastBlue, 2).good())
                {
                    if (lastRed != lastGreen || lastRed != lastBlue || lastGreen != lastBlue)
                    {
                        if (isError)
                        {
                            DCMIOD_ERROR("Bits per entry in Red, Green and Blue Palette Color LUT Descriptor are not equal");
                        }
                        else
                        {
                            DCMIOD_WARN("Bits per entry in Red, Green and Blue Palette Color LUT Descriptor are not equal");
                        }
                        return OFFalse;
                    }
                    return OFTrue;
                }
            }
        }
    }
    if (isError)
    {
        DCMIOD_ERROR("Could not read first value mapped in Red, Green or Blue Palette Color LUT Descriptor");
    }
    else
    {
        DCMIOD_WARN("Could not read first value mapped in Red, Green or Blue Palette Color LUT Descriptor");
    }
    return OFFalse;
}


OFBool IODPaletteColorLUTModule::checkSegmentConditions(const OFBool& isError)
{
    // Check that unsegmented LUTs are used together with segmented LUTs
    OFBool hasSegmentedLUTs = OFFalse;
    OFBool hasUnsegmentedLUTs = OFFalse;
    // Check for segmented LUT descriptors
    if (m_Item->tagExists(DCM_SegmentedRedPaletteColorLookupTableData) || m_Item->tagExists(DCM_SegmentedGreenPaletteColorLookupTableData) || m_Item->tagExists(DCM_SegmentedBluePaletteColorLookupTableData))
    {
        hasSegmentedLUTs = OFTrue;
    }
    // Also check for segmented LUT data
    if (m_Item->tagExists(DCM_SegmentedRedPaletteColorLookupTableData) || m_Item->tagExists(DCM_SegmentedGreenPaletteColorLookupTableData) || m_Item->tagExists(DCM_SegmentedBluePaletteColorLookupTableData))
    {
        hasSegmentedLUTs = OFTrue;
    }
    // Check for unsegmented LUT descriptors
    if (m_Item->tagExists(DCM_RedPaletteColorLookupTableDescriptor) || m_Item->tagExists(DCM_GreenPaletteColorLookupTableDescriptor) || m_Item->tagExists(DCM_BluePaletteColorLookupTableDescriptor))
    {
        hasUnsegmentedLUTs = OFTrue;
    }
    // Check for unsegmented LUT data
    if (m_Item->tagExists(DCM_RedPaletteColorLookupTableData) || m_Item->tagExists(DCM_GreenPaletteColorLookupTableData) || m_Item->tagExists(DCM_BluePaletteColorLookupTableData))
    {
        hasUnsegmentedLUTs = OFTrue;
    }
    // Check that both are not used together
    if (hasSegmentedLUTs && hasUnsegmentedLUTs)
    {
        if (isError)
        {
            DCMIOD_ERROR("Segmented LUT attributes are used together with Unsegmented LUT Data");
        }
        else
        {
            DCMIOD_WARN("Segmented LUT attributes are used together with Unsegmented LUT Data");
        }
        return OFFalse;
    }
    return OFTrue;
}
