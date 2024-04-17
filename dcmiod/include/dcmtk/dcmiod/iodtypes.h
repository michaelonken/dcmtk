/*
 *
 *  Copyright (C) 2015-2019, Open Connections GmbH
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
 *  Purpose: Class for managing common types needed by dcmiod module
 *
 */

#ifndef IODTYPES_H
#define IODTYPES_H

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmiod/ioddef.h"
#include "dcmtk/oflog/oflog.h"
#include "dcmtk/ofstd/ofcond.h"
#include "dcmtk/dcmdata/dcerror.h"
#include <cstddef>

// ----------------------------------------------------------------------------
// Define the loggers for this module
// ----------------------------------------------------------------------------

extern DCMTK_DCMIOD_EXPORT OFLogger DCM_dcmiodLogger;

#define DCMIOD_TRACE(msg) OFLOG_TRACE(DCM_dcmiodLogger, msg)
#define DCMIOD_DEBUG(msg) OFLOG_DEBUG(DCM_dcmiodLogger, msg)
#define DCMIOD_INFO(msg) OFLOG_INFO(DCM_dcmiodLogger, msg)
#define DCMIOD_WARN(msg) OFLOG_WARN(DCM_dcmiodLogger, msg)
#define DCMIOD_ERROR(msg) OFLOG_ERROR(DCM_dcmiodLogger, msg)
#define DCMIOD_FATAL(msg) OFLOG_FATAL(DCM_dcmiodLogger, msg)

// ----------------------------------------------------------------------------
// Error constants
// ----------------------------------------------------------------------------

extern DCMTK_DCMIOD_EXPORT const OFConditionConst IOD_EC_WrongSOPClass;
extern DCMTK_DCMIOD_EXPORT const OFConditionConst IOD_EC_MissingAttribute;
extern DCMTK_DCMIOD_EXPORT const OFConditionConst IOD_EC_MissingSequenceData;
extern DCMTK_DCMIOD_EXPORT const OFConditionConst IOD_EC_InvalidDimensions;
extern DCMTK_DCMIOD_EXPORT const OFConditionConst IOD_EC_CannotInsertFrame;
extern DCMTK_DCMIOD_EXPORT const OFConditionConst IOD_EC_InvalidPixelData;
extern DCMTK_DCMIOD_EXPORT const OFConditionConst IOD_EC_InvalidObject;
extern DCMTK_DCMIOD_EXPORT const OFConditionConst IOD_EC_CannotDecompress;
extern DCMTK_DCMIOD_EXPORT const OFConditionConst IOD_EC_NoSuchRule;
extern DCMTK_DCMIOD_EXPORT const OFConditionConst IOD_EC_InvalidLaterality;
extern DCMTK_DCMIOD_EXPORT const OFConditionConst IOD_EC_InvalidElementValue;
extern DCMTK_DCMIOD_EXPORT const OFConditionConst IOD_EC_InvalidReference;
extern DCMTK_DCMIOD_EXPORT const OFConditionConst IOD_EC_ReferencesOmitted;
extern DCMTK_DCMIOD_EXPORT const OFConditionConst IOD_EC_InvalidColorPalette;

/** Class that wraps some constant definitions in the context of IODs
 */
class DCMTK_DCMIOD_EXPORT DcmIODTypes
{

public:

    struct FrameBase
    {
        /// Destructor
        FrameBase() {}
        /** Returns pixel data size in bytes
         *  @return Size of pixel data in bytes
         */
        virtual size_t getLengthInBytes() const = 0;
        virtual void* getPixelData() = 0;
        virtual Uint8 bytesPerPixel() const = 0;
        virtual OFCondition getUint8AtIndex(Uint8 &byteVal, const size_t index) =0;
        virtual OFCondition getUint16AtIndex(Uint16 &shortVal, const size_t index) =0;
        virtual void setReleaseMemory(OFBool release) = 0;
        virtual OFString print() = 0;
        virtual ~FrameBase() {}
    };

    /** Struct representing a single frame
     */
    template<typename PixelType>
    struct Frame : public FrameBase
    {
        Frame() : m_pixData(NULL), m_numPixels(0), m_releaseMemory(OFTrue) {}

        Frame(const size_t numPixels) : m_pixData(NULL), m_numPixels(numPixels), m_releaseMemory(OFTrue)
        {
            m_pixData = new PixelType[numPixels];
        }

        Frame(PixelType* pixelData, const size_t lengthInBytes) : m_pixData(pixelData), m_numPixels(lengthInBytes), m_releaseMemory(OFTrue)
        {
        }

        Frame(const Frame& rhs)
        {
            delete[] m_pixData;
            m_pixData = new PixelType[rhs.m_numPixels];
            memcpy(m_pixData, rhs.m_pixData, rhs.m_numPixels);
            m_numPixels = rhs.m_numPixels;
            m_releaseMemory = rhs.m_releaseMemory;
        };

        /// Assignment operator
        Frame& operator=(const Frame& rhs)
        {
            if (this != &rhs)
            {
                delete[] m_pixData;
                m_pixData = new PixelType[rhs.m_numPixels];
                memcpy(m_pixData, rhs.m_pixData, rhs.m_numPixels);
                m_numPixels = rhs.m_numPixels;
                m_releaseMemory = rhs.m_releaseMemory;
            }
            return *this;
        }

        virtual void setReleaseMemory(OFBool release)
        {
            m_releaseMemory = release;
        }

        /** Get size of pixel data in bytes
         *  @return Size of pixel data in bytes
         */
        virtual size_t getLengthInBytes() const
        {
            return m_numPixels * bytesPerPixel(); // PixelType is always 1 or 2 bytes
        }

        virtual void* getPixelData()
        {
            return m_pixData;
        }

        virtual PixelType* getPixelDataTyped()
        {
            return m_pixData;
        }

        virtual Uint8 bytesPerPixel() const
        {
            return sizeof(PixelType);
        }

        virtual OFCondition getUint8AtIndex(Uint8 &byteVal, const size_t index)
        {
            if (index >= m_numPixels) {
                return EC_IllegalCall;
            }
            byteVal = static_cast<Uint8>(m_pixData[index]);
            return EC_Normal;
        }

        virtual OFCondition getUint16AtIndex(Uint16 &shortVal, const size_t index)
        {
            if (index >= m_numPixels) {
                return EC_IllegalCall;
            }
            shortVal = static_cast<Uint16>(m_pixData[index]);
            return EC_Normal;
        }

        virtual OFString print()
        {
            OFStringStream ss;
            ss << "Frame with " << m_numPixels + " bytes:\n";
            for (size_t i = 0; i < m_numPixels; i++)
            {
                ss << STD_NAMESPACE hex << (Uint16)(m_pixData[i]) + " ";
            }
            ss << "\n";
            return ss.str().c_str();
        }

        /// Array for the pixel data bytes
        PixelType* m_pixData;
        /// Number of pixel data in bytes
        size_t m_numPixels;
        // Denote whether to release memory in destructor
        OFBool m_releaseMemory;
        /// Destructor, frees memory
        ~Frame()
        {
            if (m_releaseMemory)
            {
                delete[] m_pixData;
                m_pixData = NULL;
            }
        }
    };

    /** IOD Information Entities (incomplete list, extended as needed)
     */
    enum IOD_IE
    {
        /// Undefined Information Entity (i.e.\ no value set)
        IE_UNDEFINED,
        /// Patient Entity
        IE_PATIENT,
        /// Study Entity
        IE_STUDY,
        /// Series Entity
        IE_SERIES,
        /// Frame of Reference Entity
        IE_FOR,
        /// Equipment Entity
        IE_EQUIPMENT,
        /// Image Entity
        IE_IMAGE,
        //// Meta Entity: Instance covering image, waveform, etc.
        IE_INSTANCE
    };

    /** Enumerated values for attribute "Laterality"
     */
    enum IOD_LATERALITY
    {
        /// Undefined (e.g.\ value not set)
        LATERALITY_UNDEFINED,
        /// Left body part
        LATERALITY_L,
        /// Right body part
        LATERALITY_R
    };

    /** Enhanced US Image Module: Image Type (first value)
     */
    enum IOD_ENHUSIMAGETYPE
    {
        /// Unknown
        IMAGETYPE_UNKNOWN,
        /// ORIGINAL
        IMAGETYPE_ORIGINAL,
        /// DERIVED
        IMAGETYPE_DERIVED
    };

private:
    /** Private undefined default constructor
     */
    DcmIODTypes() {};

    /** Private undefined destructor
     */
    ~DcmIODTypes() {};
};


#endif // IODTYPES_H
