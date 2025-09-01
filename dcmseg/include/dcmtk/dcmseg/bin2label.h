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

#include "dcmtk/dcmseg/overlaputil.h"
#include "dcmtk/dcmseg/segdoc.h" // for DcmSegmentation

/** Class representing an object of the "Segmentation SOP Class".
 */

class DCMTK_DCMSEG_EXPORT DcmBinToLabelConverter
{

public:
    struct LoadingFlags;
    struct ConversionFlags;

    // -------------------- construction / destruction -------------------------------

    // Default constructor
    DcmBinToLabelConverter();

    /** Destructor, frees memory
     */
    virtual ~DcmBinToLabelConverter();

    // -------------------- input handling -------------------------------

    void setInput(const DcmSegmentation* m_inputSeg);

    void setInput(DcmDataset* m_inputSeg, const DcmSegmentation::LoadingFlags& loadFlags = DcmSegmentation::LoadingFlags());

    void setInput(OFFilename filename, const DcmSegmentation::LoadingFlags& loadFlags = DcmSegmentation::LoadingFlags());

    // -------------------- processing  ----------------------------------

    E_TransferSyntax getInputTransferSyntax() const;

    /** Static method to load a Segmentation object from a file.
     *  The memory of the resulting Segmentation object has to be freed by the
     *  caller.
     *  @param  filename The file to read from
     *  @param  labelMap  The resulting segmentation object. NULL if dataset
     *          could not be read successfully.
     *  @param  flags Flags to configure the loading of the segmentation object
     *  @return EC_Normal if reading was successful, error otherwise
     */
    OFCondition convert(const ConversionFlags& convFlags = ConversionFlags());

    /// Flags for converting binary segmentation objects to label map segmentations
    struct ConversionFlags
    {
        /// Return error if object is already a label map object
        OFBool m_errorIfAlreadyLabelMap;

        /// Number of threads maximally used (for reading/writing segmentation objects)
        Uint32 m_numThreads;

        /// Enables/disables checking of functional groups on dataset export (default: on)
        OFBool m_checkExportFG;

        /// Enables/disables checking of values on dataset export (default: on)
        OFBool m_checkExportValues;

        /// Constructor to initialize the flags
        ConversionFlags()
        {
            clear();
        }

        /** Clear all flags to their default values */
        void clear()
        {
            m_errorIfAlreadyLabelMap = OFFalse;
            m_numThreads = 1;
            m_checkExportFG = OFTrue;
            m_checkExportValues = OFTrue;
        }
    };

    // -------------------- output handling ----------------------------------

    /** Provides output as label map segmentation object. This is already computed
     *  if convert(...) has been called.
     *  @param  outputSeg Pointer to the output segmentation or NULL if not available
     *  @return EC_Normal if successful, error otherwise
     */
    OFCondition getOutputSegmentation(OFshared_ptr<DcmSegmentation>& outputSeg);

    /** Returns a pointer to the output dataset. This is an expensive operation
     *  since it requires producing the dataset from the internal output segmentation.
     *  The caller must provide an existing DcmItem. It is cleared before use.
     *  @param  outputDataset Output dataset.
     *  @return EC_Normal if successful, error otherwise
     */
    OFCondition getOutputDataset(DcmItem& outputDataset);

    /** Clear all internal data to get ready for processing another segmentation
     *  object.
     */
    void clear();

protected:

    template <typename T>
    static OFCondition copyComponent(T* src, T* dest);
    static OFCondition copyCommonModules(DcmSegmentation* src, DcmSegmentation* dest);
    OFCondition copyPerFrameInfo(DcmSegmentation* src);
    OFCondition copySegments(DcmSegmentation* src, DcmSegmentation* dest);
    OFCondition loadInput();

private:
    // Disable copy constructor and assignment operator
    DcmBinToLabelConverter(const DcmBinToLabelConverter&);
    DcmBinToLabelConverter& operator=(const DcmBinToLabelConverter&);

    OFCondition checkSOPClassAndSegtype(const OFString& sopClassUID,
                                        const OFString& segType);

    DcmSegmentation::LoadingFlags m_loadFlags;
    ConversionFlags m_convFlags;

    // Input sources
    DcmDataset* m_inputDataset;
    OFFilename m_inputFileName;
    OFunique_ptr<DcmSegmentation> m_inputSeg;
    E_TransferSyntax m_inputXfer;
    // Remember output segmentation in converter but also return in parameter,
    // so the caller is responsible for destroying itresult = copySegments(m_inputSeg.get(), m_outputSeg.get());
    OFshared_ptr<DcmSegmentation> m_outputSeg;
    OFBool m_use16Bit;
    OverlapUtil m_overlapUtil;
};

#endif // BIN2LABEL
