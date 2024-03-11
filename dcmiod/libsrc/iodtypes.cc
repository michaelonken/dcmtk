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
 *  Purpose: Class for managing attribute rules as found in DICOM modules
 *
 */

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmiod/iodtypes.h"
#include "dcmtk/oflog/oflog.h"
#include "dcmtk/dcmdata/dcerror.h"

OFLogger DCM_dcmiodLogger = OFLog::getLogger("dcmtk.dcmiod");

/*---------------------------------*
 *  constant definitions
 *---------------------------------*/

// conditions
makeOFConditionConst(IOD_EC_WrongSOPClass, OFM_dcmiod, 1, OF_error, "Wrong SOP Class");
makeOFConditionConst(IOD_EC_MissingAttribute, OFM_dcmiod, 2, OF_error, "Missing Attribute(s)");
makeOFConditionConst(IOD_EC_MissingSequenceData, OFM_dcmiod, 3, OF_error, "Missing Sequence Data");
makeOFConditionConst(IOD_EC_InvalidDimensions, OFM_dcmiod, 4, OF_error, "Invalid dimension information");
makeOFConditionConst(IOD_EC_CannotInsertFrame, OFM_dcmiod, 5, OF_error, "Cannot insert frame");
makeOFConditionConst(IOD_EC_InvalidPixelData, OFM_dcmiod, 6, OF_error, "Invalid Pixel Data");
makeOFConditionConst(IOD_EC_InvalidObject, OFM_dcmiod, 7, OF_error, "Invalid Object");
makeOFConditionConst(IOD_EC_CannotDecompress, OFM_dcmiod, 8, OF_error, "Cannot decompress");
makeOFConditionConst(IOD_EC_NoSuchRule, OFM_dcmiod, 9, OF_error, "No such IOD rule");
makeOFConditionConst(
    IOD_EC_InvalidLaterality, OFM_dcmiod, 10, OF_error, "Invalid value for 'Laterality' (only 'L' or 'R' permitted)");
makeOFConditionConst(IOD_EC_InvalidElementValue, OFM_dcmiod, 11, OF_error, "Value not allowed for element");
makeOFConditionConst(IOD_EC_InvalidReference, OFM_dcmiod, 12, OF_error, "One or more invalid SOP references");
makeOFConditionConst(
    IOD_EC_ReferencesOmitted, OFM_dcmiod, 13, OF_error, "One or more SOP references have been omitted");
makeOFConditionConst(IOD_EC_InvalidColorPalette, OFM_dcmiod, 14, OF_error, "Invalid Color Palette LUT");


template <>
OFCondition DcmIODTypes::Frame<Uint16>::getUint8AtIndex(Uint8 &byteVal, const size_t index) {
    if (index >= length) {
        return EC_IllegalCall;
    }
    if (pixData[index] > 255) {
        DCMIOD_ERROR("Value in the frame is too large to be cast to 8 bits");
        return EC_IllegalCall;
    }
    byteVal = static_cast<Uint8>(pixData[index]);
    return EC_Normal;
}

