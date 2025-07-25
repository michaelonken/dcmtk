/*
 *
 *  Copyright (C) 1994-2025, OFFIS e.V.
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
 *  Module:  dcmdata
 *
 *  Author:  Andrew Hewett
 *
 *  Purpose: Error handling, codes and strings
 *
 */


#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */

#include "dcmtk/dcmdata/dcerror.h"

makeOFConditionConst(EC_InvalidTag,                      OFM_dcmdata,  1, OF_error, "Invalid tag"                                );
makeOFConditionConst(EC_TagNotFound,                     OFM_dcmdata,  2, OF_error, "Tag not found"                              );
makeOFConditionConst(EC_InvalidVR,                       OFM_dcmdata,  3, OF_error, "Invalid VR"                                 );
makeOFConditionConst(EC_InvalidStream,                   OFM_dcmdata,  4, OF_error, "Invalid stream"                             );
makeOFConditionConst(EC_EndOfStream,                     OFM_dcmdata,  5, OF_error, "End of stream"                              );
makeOFConditionConst(EC_CorruptedData,                   OFM_dcmdata,  6, OF_error, "Corrupted data"                             );
makeOFConditionConst(EC_IllegalCall,                     OFM_dcmdata,  7, OF_error, "Illegal call, perhaps wrong parameters"     );
makeOFConditionConst(EC_SequEnd,                         OFM_dcmdata,  8, OF_error, "Sequence end"                               );
makeOFConditionConst(EC_DoubledTag,                      OFM_dcmdata,  9, OF_error, "Doubled tag"                                );
makeOFConditionConst(EC_StreamNotifyClient,              OFM_dcmdata, 10, OF_error, "I/O suspension or premature end of stream"  );
makeOFConditionConst(EC_WrongStreamMode,                 OFM_dcmdata, 11, OF_error, "Mode (R/W, random/sequence) is wrong"       );
makeOFConditionConst(EC_ItemEnd,                         OFM_dcmdata, 12, OF_error, "Item end"                                   );
makeOFConditionConst(EC_RepresentationNotFound,          OFM_dcmdata, 13, OF_error, "Pixel representation not found"             );
makeOFConditionConst(EC_CannotChangeRepresentation,      OFM_dcmdata, 14, OF_error, "Pixel representation cannot be changed"     );
makeOFConditionConst(EC_UnsupportedEncoding,             OFM_dcmdata, 15, OF_error, "Unsupported compression or encryption"      );
// error code 16 is reserved for zlib-related error messages
makeOFConditionConst(EC_PutbackFailed,                   OFM_dcmdata, 17, OF_error, "Parser failure: Putback operation failed"   );
// error code 18 is reserved for file read error messages
// error code 19 is reserved for file write error messages
makeOFConditionConst(EC_DoubleCompressionFilters,        OFM_dcmdata, 20, OF_error, "Too many compression filters"               );
makeOFConditionConst(EC_ApplicationProfileViolated,      OFM_dcmdata, 21, OF_error, "Storage media application profile violated" );
// error code 22 is reserved for dcmodify error messages
makeOFConditionConst(EC_InvalidOffset,                   OFM_dcmdata, 23, OF_error, "Invalid offset"                             );
makeOFConditionConst(EC_TooManyBytesRequested,           OFM_dcmdata, 24, OF_error, "Too many bytes requested"                   );
// error code 25 is reserved for tag path parsing error messages
makeOFConditionConst(EC_InvalidBasicOffsetTable,         OFM_dcmdata, 26, OF_error, "Invalid basic offset table"                 );
makeOFConditionConst(EC_ElemLengthLargerThanItem,        OFM_dcmdata, 27, OF_error, "Length of element larger than explicit length of surrounding item" );
makeOFConditionConst(EC_FileMetaInfoHeaderMissing,       OFM_dcmdata, 28, OF_error, "File meta information header missing"       );
makeOFConditionConst(EC_SeqOrItemContentOverflow,        OFM_dcmdata, 29, OF_error, "Item or sequence content exceeds maximum of 32-bit length field");
makeOFConditionConst(EC_ValueRepresentationViolated,     OFM_dcmdata, 30, OF_error, "Value Representation violated"              );
makeOFConditionConst(EC_ValueMultiplicityViolated,       OFM_dcmdata, 31, OF_error, "Value Multiplicity violated"                );
makeOFConditionConst(EC_MaximumLengthViolated,           OFM_dcmdata, 32, OF_error, "Maximum VR length violated"                 );
makeOFConditionConst(EC_ElemLengthExceeds16BitField,     OFM_dcmdata, 33, OF_error, "Length of element value exceeds maximum of 16-bit length field" );
makeOFConditionConst(EC_SequDelimitationItemMissing,     OFM_dcmdata, 34, OF_error, "Sequence Delimitation Item missing"         );
// error codes 35..36 are reserved for specific character set error messages (see below)
// error code 37 is reserved for XML conversion error messages (see below)
makeOFConditionConst(EC_ItemDelimitationItemMissing,     OFM_dcmdata, 38, OF_error, "Item Delimitation Item missing"             );
makeOFConditionConst(EC_PrematureSequDelimitationItem,   OFM_dcmdata, 39, OF_error, "Sequence Delimitation Item occurred before Item was completely read" );
makeOFConditionConst(EC_InvalidDICOMDIR,                 OFM_dcmdata, 40, OF_error, "Invalid DICOMDIR"                           );
makeOFConditionConst(EC_UnknownVR,                       OFM_dcmdata, 41, OF_error, "Unknown VR: Tag not found in data dictionary" );
makeOFConditionConst(EC_InvalidValue,                    OFM_dcmdata, 42, OF_error, "Invalid Value"                              );
makeOFConditionConst(EC_ItemNotFound,                    OFM_dcmdata, 43, OF_error, "Item not found"                             );
makeOFConditionConst(EC_UnknownTransferSyntax,           OFM_dcmdata, 44, OF_error, "Unknown Transfer Syntax"                    );
makeOFConditionConst(EC_CannotCheck,                     OFM_dcmdata, 45, OF_error, "Cannot perform check"                       );
makeOFConditionConst(EC_MissingValue,                    OFM_dcmdata, 46, OF_error, "Missing value"                              );
makeOFConditionConst(EC_MissingAttribute,                OFM_dcmdata, 47, OF_error, "Missing attribute"                          );
makeOFConditionConst(EC_InternalError,                   OFM_dcmdata, 48, OF_error, "Internal error"                             );
makeOFConditionConst(EC_InvalidCharacter,                OFM_dcmdata, 49, OF_error, "Invalid character"                          );
// error code 50 is reserved for determine start fragment error messages (see below)
makeOFConditionConst(EC_UndefinedLengthOBOW,             OFM_dcmdata, 51, OF_error, "Illegal element with OB or OW Value Representation and undefined length encountered" );
makeOFConditionConst(EC_VOI_LUT_OBOW,                    OFM_dcmdata, 52, OF_error, "Illegal VOI LUT Sequence element with OB or OW Value Representation and explicit length encountered" );
makeOFConditionConst(EC_PixelDataExplLengthIllegal,      OFM_dcmdata, 53, OF_error, "Pixel data in top level dataset in compressed Transfer Syntax uses explicit length" );
makeOFConditionConst(EC_ElemLengthExceeds32BitField,     OFM_dcmdata, 54, OF_error, "Length of element value exceeds maximum of 32-bit length field" );
makeOFConditionConst(EC_CannotWriteJsonNumber,           OFM_dcmdata, 55, OF_error, "Cannot write 'nan' or 'inf' as JSON number" );
makeOFConditionConst(EC_CannotWriteJsonInlineBinary,     OFM_dcmdata, 56, OF_error, "JSON InlineBinary encoding not supported for compressed pixel data" );
makeOFConditionConst(EC_XMLParseError,                   OFM_dcmdata, 57, OF_error, "XML parse error"                            );
makeOFConditionConst(EC_XMLValidationFailure,            OFM_dcmdata, 58, OF_error, "XML validation failure"                     );
makeOFConditionConst(EC_SOPClassMismatch,                OFM_dcmdata, 59, OF_error, "SOP Class mismatch"                         );
makeOFConditionConst(EC_UnknownUIDName,                  OFM_dcmdata, 60, OF_error, "Unknown UID name: No mapping to UID value defined" );
makeOFConditionConst(EC_CannotWriteStringAsJSONNumber,   OFM_dcmdata, 61, OF_error, "Cannot write IS/DS string as JSON number"   );
makeOFConditionConst(EC_CannotWriteBulkDataFile,         OFM_dcmdata, 62, OF_error, "Cannot write bulk data file"                );
makeOFConditionConst(EC_CannotWriteJSONMultiframe,       OFM_dcmdata, 63, OF_error, "JSON encoding not supported for encapsulated multi-frame pixel data" );
makeOFConditionConst(EC_InvalidJSONType,                 OFM_dcmdata, 64, OF_error, "Invalid JSON type"                          );
makeOFConditionConst(EC_InvalidJSONContent,              OFM_dcmdata, 65, OF_error, "Invalid JSON content"                       );
makeOFConditionConst(EC_BulkDataURINotSupported,         OFM_dcmdata, 66, OF_error, "BulkDataURI not yet supported"              );

const unsigned short EC_CODE_CannotSelectCharacterSet     = 35;
const unsigned short EC_CODE_CannotConvertCharacterSet    = 36;
const unsigned short EC_CODE_CannotConvertToXML           = 37;
const unsigned short EC_CODE_CannotDetermineStartFragment = 50;
