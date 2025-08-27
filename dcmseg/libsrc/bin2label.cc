/*
 *  Copyright (C) 2015-2025, Open Connections GmbH
 *
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

#include "dcmtk/dcmseg/bin2label.h"
#include "dcmtk/config/osconfig.h" // include OS configuration first
#include "dcmtk/dcmdata/dcuid.h"
#include "dcmtk/dcmfg/fgfact.h"
#include "dcmtk/dcmseg/segtypes.h"
#include <zconf.h>

DcmBinToLabelConverter::DcmBinToLabelConverter(const DcmSegmentation::LoadingFlags& loadFlags,
                                               const DcmBinToLabelConverter::ConversionFlags& convFlags)
    : m_loadFlags(loadFlags)
    , m_convFlags(convFlags)
    , m_inputSeg(nullptr)
    , m_outputSeg(nullptr)
    , m_use16Bit(OFFalse)
    , m_overlapUtil()
{
}

OFCondition DcmBinToLabelConverter::convertDataset(DcmDataset& dataset,
                                                   DcmSegmentation*& segmentation,
                                                   const DcmSegmentation::LoadingFlags& loadFlags,
                                                   const DcmBinToLabelConverter::ConversionFlags& convFlags)
{
    DcmBinToLabelConverter converter(loadFlags, convFlags);
    // Check whether SOP Class and Segmentation Type are suitable for conversion
    OFCondition result = converter.checkSOPClassAndSegtype(dataset);
    // If this is not a binary segmentation, already a label map, or has invalid
    // data in SOP Class UID and/or Segmentation Type, return here.
    if (result != EC_Normal)
        return result;

    DcmSegmentation* temp = NULL;
    DCMSEG_DEBUG("Loading input dataset into segmentation object");
    result                = DcmSegmentation::loadDataset(dataset, temp, loadFlags);
    if (result.good())
    {
        converter.m_inputSeg.reset(temp);
        // Check for overlaps which would prevent conversion
        DCMSEG_DEBUG("Checking for overlapping segments");
        converter.m_overlapUtil.setSegmentationObject(converter.m_inputSeg.get());
        if (converter.m_overlapUtil.hasOverlappingSegments())
        {
            return SG_EC_OverlappingSegments;
        }
        // Get number of segments to find out whether we need 16 bit data.
        // We will use MONOCHROME2 color model (palette is not supported during conversion for now).
        DCMSEG_DEBUG("Checking number of segments to decide whether to use 8 or 16 bit pixel data");
        size_t numSegments = converter.m_inputSeg->getNumberOfSegments();
        converter.m_use16Bit    = (numSegments > 256);
        DCMSEG_DEBUG("Using " << (converter.m_use16Bit ? "16" : "8") << " bit pixel data for " << numSegments << " segments");


        OFString TODO;
        ContentIdentificationMacro& tempc = converter.m_inputSeg->getContentIdentification();
        tempc.getInstanceNumber(TODO);
        std::cout << "TODO Instance Number: " << TODO << std::endl;
        tempc.getContentLabel(TODO);
        std::cout << "TODO Content Label: " << TODO << std::endl;

        ContentIdentificationMacro content;
        result = copyComponent(&(converter.m_inputSeg->getContentIdentification()), &content);

        if (result.good())
        {
            DCMSEG_DEBUG("Creating labelmap output segmentation object");
            result
                = DcmSegmentation::createLabelmapSegmentation(temp,
                                                              converter.m_inputSeg->getRows(),
                                                              converter.m_inputSeg->getColumns(),
                                                              converter.m_inputSeg->getEquipment().getEquipmentInfo(),
                                                              content,
                                                              converter.m_use16Bit,
                                                              DcmSegTypes::SLCM_MONOCHROME2);
            if (result.good())
            {
                converter.m_outputSeg.reset(temp);
                // Copy all components except pixel data
                DCMSEG_DEBUG("Copying common modules from input to output segmentation");
                result = copyCommonModules(converter.m_inputSeg.get(), converter.m_outputSeg.get());
            }
            if (result.good())
            {
                DCMSEG_DEBUG("Copying per-frame information (pixel data and FGs) to output segmentation");
                result = converter.copyPerFrameInfo(converter.m_inputSeg.get());
            }
        }
    }
    return result;
}

OFCondition DcmBinToLabelConverter::convertFile(const OFString& filename,
                                                DcmSegmentation*& segmentation,
                                                const DcmSegmentation::LoadingFlags& loadFlags,
                                                const DcmBinToLabelConverter::ConversionFlags& convFlags)
{
    // Load the segmentation object from the file
    DcmFileFormat fileformat;
    OFCondition result = fileformat.loadFile(filename);
    if (result.good())
    {
        // Convert the loaded dataset to a segmentation object
        result = convertDataset(*fileformat.getDataset(), segmentation, loadFlags, convFlags);
    }
    return result;
}

DcmBinToLabelConverter::~DcmBinToLabelConverter()
{
    // Nothing to do here, no dynamic memory allocated
}

OFCondition DcmBinToLabelConverter::checkSOPClassAndSegtype(DcmDataset& dataset)
{
    DCMSEG_DEBUG("Checking SOP Class and Segmentation Type");
    // Check if the SOP Class UID is correct
    OFString sopClassUID;
    OFCondition result;
    if (dataset.findAndGetOFString(DCM_SOPClassUID, sopClassUID).good())
    {
        if ((sopClassUID != UID_SegmentationStorage) && (sopClassUID != UID_LabelMapSegmentationStorage))
        {
            return SG_EC_NoSegmentationBasedSOPClass;
        }
        else if (sopClassUID == UID_LabelMapSegmentationStorage)
        {
            if (m_convFlags.m_errorIfAlreadyLabelMap)
            {
                return SG_EC_AlreadyLabelMap;
            }
            else
            {
                result = SG_EC_NoConversionRequired;
            }
        }
        else if (sopClassUID == UID_SegmentationStorage)
        {
            result = EC_Normal;
        }
        else
        {
            return SG_EC_NoSegmentationBasedSOPClass;
        }
    }
    else
    {
        return IOD_EC_InvalidObject;
    }

    // Check if the Segmentation Type is valid
    DCMSEG_DEBUG("SOP Class acceptable for conversion to labelmap, checking Segmentation Type");
    OFString segType;
    if (dataset.findAndGetOFString(DCM_SegmentationType, segType).bad()
        || DcmSegTypes::OFString2Segtype(segType) == DcmSegTypes::ST_UNKNOWN)
    {
        return IOD_EC_InvalidObject;
    }

    // check whether sop class and segmentation type match
    if (sopClassUID == UID_LabelMapSegmentationStorage
        && DcmSegTypes::OFString2Segtype(segType) == DcmSegTypes::ST_LABELMAP)
    {
        // result is already set to SG_EC_NoConversionRequired
        DCMSEG_DEBUG("Segmentation object for conversion is a label map");
    }
    else if (sopClassUID == UID_SegmentationStorage && DcmSegTypes::OFString2Segtype(segType) == DcmSegTypes::ST_BINARY)
    {
        DCMSEG_DEBUG("Segmentation object for conversion is a binary segmentation");
    }
    else if (sopClassUID == UID_SegmentationStorage
             && DcmSegTypes::OFString2Segtype(segType) == DcmSegTypes::ST_FRACTIONAL)
    {
        DCMSEG_DEBUG("Segmentation object for conversion is a fractional segmentation");
        result = SG_EC_CannotConvertFractionalToLabelmap;
    }
    else
    {
        DCMSEG_ERROR("SOP Class UID " << sopClassUID << " does not match Segmentation Type " << segType);
        result = IOD_EC_InvalidObject;
    }
    return result;
}

template <typename T>
OFCondition DcmBinToLabelConverter::copyComponent(T* src, T* dest)
{
    OFCondition result;
    if ((src && dest) && (src != dest))
    {
        DcmItem item;
        DCMSEG_DEBUG("Writing component into temporary item");
        result = src->write(item);
        if (result.good())
        {

            DCMSEG_DEBUG("Reading temporary item component into destination");
            result = dest->read(item);
        }
    }
    else
    {
        result = EC_IllegalParameter;
    }
    return result;
}

OFCondition DcmBinToLabelConverter::copyCommonModules(DcmSegmentation* src, DcmSegmentation* dest)
{
    OFCondition result;

    if (src && dest)
    {
        // Copy all components except pixel data:

        // Start with modules from IODImage:
        // Patient Module, General Study Module, General Equipment Module,
        // General Series Module, Frame of Reference Module.
        // This skips Image Pixel Module and SOP Common
        DCMSEG_DEBUG("Copying Patient Module from input to output segmentation");
        result = copyComponent(&(src->getPatient()), &dest->getPatient());
        if (result.good())
        {
            DCMSEG_DEBUG("Copying General Study Module from input to output segmentation");
            result = copyComponent(&(src->getStudy()), &dest->getStudy());
        }
        if (result.good())
        {
            DCMSEG_DEBUG("Copying General Equipment Module from input to output segmentation");
            result = copyComponent(&(src->getEquipment()), &dest->getEquipment());
        }
        if (result.good())
        {
            // TODO fix series instance UID?
            DCMSEG_DEBUG("Copying General Series Module from input to output segmentation");
            result = copyComponent(&(src->getSeries()), &dest->getSeries());
        }
        if (result.good())
        {
            DCMSEG_DEBUG("Copying Frame of Reference Module from input to output segmentation");
            result = copyComponent(&(src->getFrameOfReference()), &dest->getFrameOfReference());
        }
        if (result.good())
        {
            DCMSEG_DEBUG("Copying General Image Module from input to output segmentation");
            result = copyComponent(&(src->getGeneralImage()), &dest->getGeneralImage());
        }
        if (result.bad())
            return result;

        // Continue with all others:
        // - Segmentation Series Module
        // - Multi-Frame Dimension Module
        // - Common Instance Reference Module
        // - Multi-frame Functional Group Module
        // This skips:
        // - Palette Color LUT Module (not set in binary segmentations)
        // - Segmentation Image Module (rewritten for labelmaps)
        DCMSEG_DEBUG("Copying Dimension Module from input to output segmentation");
        result = copyComponent(&(src->getDimensions()), &dest->getDimensions());
        if (result.good())
        {
            DCMSEG_DEBUG("Copying Common Instance Reference Module from input to output segmentation");
            result = copyComponent(&(src->getCommonInstanceReference()), &dest->getCommonInstanceReference());
        }
        if (result.bad())
            return result;

        // Copy shared Functional Groups
        DCMSEG_DEBUG("Copying shared Functional Groups from input to output segmentation");
        const FunctionalGroups* sharedFGs = src->getFunctionalGroups().getShared();
        FunctionalGroups::const_iterator it = sharedFGs->begin();
        while (result.good() && it != sharedFGs->end())
        {
            FGBase* sharedDest = FGFactory::instance().create(it->second->getType());
            if (sharedDest)
            {
                result = copyComponent(it->second, sharedDest);
                if (result.good())
                {
                    result = dest->getFunctionalGroups().addShared(*sharedDest);
                }
            }
            delete sharedDest;
            ++it;
        }
    }
    else
    {
        result = EC_IllegalParameter;
    }
    return result;
}

OFCondition DcmBinToLabelConverter::copyPerFrameInfo(DcmSegmentation* src)
{
    OFCondition result;

    if (src)
    {
        // Walk through segments, and for each segments, get all the related frames
        // and construct a new destination frame if we dont have a corresponding one
        // at the same position in space. So input frames at the same position will
        // result in a new destination frame being created.
        OverlapUtil::DistinctFramePositions framesAtPositions;
        result = m_overlapUtil.getFramesByPosition(framesAtPositions);
        if (result.good())
        {
            // Iterate over all positions, and create a new destination frame
            DCMSEG_DEBUG("Creating new destination frames for each input frame position");
            OFVector<OverlapUtil::LogicalFrame>::iterator it = framesAtPositions.begin();
            while (result.good()&& (it != framesAtPositions.end()))
            {
                // Delete Segmentation Functional group (not permitted as per-frame)
                src->getFunctionalGroups().deletePerFrame(DcmFGTypes::EFG_SEGMENTATION);
                // use per-frame information of first frame. We need them in a vector...
                OFVector<FGBase*> perFrameInfo;
                const FunctionalGroups* sourceFGs = src->getFunctionalGroups().getPerFrame(it->at(0));
                FunctionalGroups::const_iterator fgIt = sourceFGs->begin();
                while (fgIt != sourceFGs->end())
                {
                    perFrameInfo.push_back(fgIt->second);
                    fgIt++;
                }
                // addFrame() will copy functional groups. Memory is still handled by source object, so
                // no need to delete them.
                if (m_use16Bit)
                {
                    DCMSEG_DEBUG("Creating new 16 bit destination frame");
                    // create a new destination frame
                    Uint16* newFrame = new Uint16[src->getRows() * src->getColumns()];
                    if (newFrame) result = m_outputSeg->addFrame(newFrame, 0 /* ignored for labelmaps */, perFrameInfo);
                    else result = EC_MemoryExhausted;
                }
                else // 8 bit
                {
                    DCMSEG_DEBUG("Creating new 8 bit destination frame");
                    Uint8* newFrame = new Uint8[src->getRows() * src->getColumns()];
                    if (newFrame) result = m_outputSeg->addFrame(newFrame, 0 /* ignored for labelmaps */, perFrameInfo);
                    else result = EC_MemoryExhausted;
                }
                it++;
            }
        }
    }
    else
    {
        result = EC_IllegalParameter;
    }
    return result;
}