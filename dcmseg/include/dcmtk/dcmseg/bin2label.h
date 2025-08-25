/*
 *
 *  Copyright (C) 2015-2025, Open Connections GmbH
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
 *  Module:  dcmseg
 *
 *  Author:  Michael Onken
 *
 *  Purpose: Class for converting binary to label map segmentations
 *
 */

#ifndef BIN2LABEL
#define BIN2LABEL

#include "dcmtk/config/osconfig.h" // include OS configuration first

#include "dcmtk/dcmseg/segdoc.h" // for DcmSegmentation
#include "overlaputil.h"

/** Class representing an object of the "Segmentation SOP Class".
 */

class DCMTK_DCMSEG_EXPORT DcmBinToLabelConverter
{

public:
    struct LoadingFlags;
    struct ConversionFlags;

    // -------------------- destruction -------------------------------

    /** Destructor, frees memory
     */
    virtual ~DcmBinToLabelConverter();

    // -------------------- loading and saving ---------------------

    /** Static method to load a Segmentation object from a file.
     *  The memory of the resulting Segmentation object has to be freed by the
     *  caller.
     *  @param  filename The file to read from
     *  @param  segmentation  The resulting segmentation object. NULL if dataset
     *          could not be read successfully.
     *  @param  flags Flags to configure the loading of the segmentation object
     *  @return EC_Normal if reading was successful, error otherwise
     */
    OFCondition convertFile(const OFString& filename,
                            DcmSegmentation*& segmentation,
                            const DcmSegmentation::LoadingFlags& loadFlags            = DcmSegmentation::LoadingFlags(),
                            const DcmBinToLabelConverter::ConversionFlags& convFlags = ConversionFlags());

    /** Static method to load a Segmentation object from a dataset object.
     *  The memory of the resulting Segmentation object has to be freed by the
     *  caller.
     *  @param  dataset The dataset to read from
     *  @param  segmentation  The resulting segmentation object. NULL if dataset
     *          could not be read successfully.
     *  @param  flags Flags to configure the loading of the segmentation object
     *  @return EC_Normal if reading was successful, error otherwise
     */
    static OFCondition convertDataset(DcmDataset& dataset,
                                      DcmSegmentation*& segmentation,
                                      const DcmSegmentation::LoadingFlags& loadFlags = DcmSegmentation::LoadingFlags(),
                                      const DcmBinToLabelConverter::ConversionFlags& convFlags = ConversionFlags());

    /// Flags for converting binary segmentation objects to label map segmentations
    struct ConversionFlags
    {
        // Return error if object is already a label map object
        OFBool m_errorIfAlreadyLabelMap;

        // Constructor to initialize the flags
        ConversionFlags()
            : m_errorIfAlreadyLabelMap(OFFalse)
        {
        }
    };

protected:

    // Default constructor, non-public
    DcmBinToLabelConverter(const DcmSegmentation::LoadingFlags& loadFlags,
                           const DcmBinToLabelConverter::ConversionFlags& convFlags);

    template<typename T>
    static OFCondition copyComponent (T* src, T* dest);
    static OFCondition copyCommonModules(DcmSegmentation* src, DcmSegmentation* dest);
    OFCondition copyPixelDataFrom(DcmSegmentation* src);

private:

    // Disable copy constructor and assignment operator
    DcmBinToLabelConverter(const DcmBinToLabelConverter&);
    DcmBinToLabelConverter& operator=(const DcmBinToLabelConverter&);

    OFCondition checkSOPClassAndSegtype(DcmDataset& dataset);


    DcmSegmentation::LoadingFlags m_loadFlags;
    ConversionFlags m_convFlags;
    OFunique_ptr<DcmSegmentation> m_inputSeg;
    OFunique_ptr<DcmSegmentation> m_outputSeg;
    OFBool m_use16Bit;
    OverlapUtil m_overlapUtil;
};

#endif // BIN2LABEL
