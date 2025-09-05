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


#include "dcmtk/config/osconfig.h" // include OS configuration first
#include "dcmtk/dcmseg/bin2label.h"
#include "dcmtk/dcmiod/iodutil.h"
#include "dcmtk/dcmdata/dcuid.h"
#include "dcmtk/dcmfg/fgfact.h"
#include "dcmtk/dcmfg/fgfracon.h"
#include "dcmtk/dcmseg/segtypes.h"
#include <zconf.h>

DcmBinToLabelConverter::DcmBinToLabelConverter()
    : m_loadFlags()
    , m_convFlags()
    , m_inputDataset(OFnullptr)
    , m_inputFileName()
    , m_inputSeg(OFnullptr)
    , m_inputXfer(E_TransferSyntax::EXS_Unknown)
    , m_outputSeg(OFnullptr)
    , m_use16Bit(OFFalse)
    , m_overlapUtil()
{
}


void DcmBinToLabelConverter::setInput(const DcmSegmentation* inputSeg)
{
   clear();
   if (inputSeg)
   {
       m_inputSeg.reset(new DcmSegmentation(*inputSeg));
   }
}


void DcmBinToLabelConverter::setInput(DcmDataset* inputDataset, const DcmSegmentation::LoadingFlags& loadFlags)
{
    clear();
    m_inputDataset = inputDataset;
    m_loadFlags = loadFlags;
}


void DcmBinToLabelConverter::setInput(OFFilename filename, const DcmSegmentation::LoadingFlags& loadFlags)
{
    clear();
    m_inputFileName = filename;
    m_loadFlags = loadFlags;
}


void DcmBinToLabelConverter::clear()
{
    m_inputSeg.reset();
    m_inputDataset = OFnullptr;
    m_inputFileName.clear();
    m_loadFlags.clear();
    m_convFlags.clear();
    m_outputSeg.reset();
    m_use16Bit = OFFalse;
    m_overlapUtil.clear();
}


OFCondition DcmBinToLabelConverter::convert(const ConversionFlags& convFlags)
{
    // Check whether input is set appropriately; loads input segmentation (if necessary)
    // and checks whether its a binary segmentation object
    m_convFlags = convFlags;
    OFCondition result = loadInput();
    if (result.bad())
    {
        return result;
    }

    // Check for overlaps which would prevent conversion
    m_overlapUtil.setSegmentationObject(m_inputSeg.get());
    if (m_overlapUtil.hasOverlappingSegments())
    {
        return SG_EC_OverlappingSegments;
    }
    // Get number of segments to find out whether we need 16 bit data.
    // We will use MONOCHROME2 color model (palette is not supported during conversion for now).
    size_t numSegments = m_inputSeg->getNumberOfSegments();
    m_use16Bit    = (numSegments > 256);
    DCMSEG_DEBUG("Using " << (m_use16Bit ? "16" : "8") << " bit pixel data for " << numSegments << " segments");

    // Copy Content Identification Macro required for labelmap creation call
    ContentIdentificationMacro content;
    result = copyComponent(&(m_inputSeg->getContentIdentification()), &content);

    // Create Labelmap
    if (result.good())
    {
        DcmSegmentation* temp = NULL;
        DCMSEG_DEBUG("Creating labelmap output segmentation object");
        result
            = DcmSegmentation::createLabelmapSegmentation(temp,
                                                            m_inputSeg->getRows(),
                                                            m_inputSeg->getColumns(),
                                                            m_inputSeg->getEquipment().getEquipmentInfo(),
                                                            content,
                                                            m_use16Bit,
                                                            DcmSegTypes::SLCM_MONOCHROME2);
        // Remember output segmentation in converter but also return in parameter
        m_outputSeg = temp;
    }
    // Copy all common information (patient, study, series and some instance level data)
    if (result.good())
    {
        DCMSEG_DEBUG("Copying common modules from input to output segmentation");
        result = copyCommonModules(m_inputSeg.get(), m_outputSeg.get());
    }
    // Copy Segments
    if (result.good())
    {
        DCMSEG_DEBUG("Copying segments from input to output segmentation");
        result = copySegments(m_inputSeg.get(), m_outputSeg.get());
    }
    // Copy pixel data and per-frame functional groups (excluding some that cannot be migrated)
    if (result.good())
    {
        DCMSEG_DEBUG("Copying per-frame information (pixel data and FGs) to output segmentation");
        result = copyPerFrameInfo(m_inputSeg.get());
    }

    return result;
}

OFCondition DcmBinToLabelConverter::copySegments(DcmSegmentation* src, DcmSegmentation* dest)
{
    OFCondition result;
    DCMSEG_DEBUG("Copying segments from source to destination");
    // Iterate over all segments in the source segmentation
    OFMap<Uint16, DcmSegment*>::const_iterator srcSegment = src->getSegments().begin();
    while (srcSegment != src->getSegments().end() && result.good())
    {
        // Get segment from source segmentation
        // Note that segment number is not relevant for labelmaps, so we don't pass it to addSegment
        // (it will be assigned automatically in addSegment)
        DCMSEG_DEBUG("Copying segment number " << srcSegment->first);
        if (srcSegment->second)
        {
            // Clone the segment and add it to the destination segmentation
            DcmSegment* clonedSegment = srcSegment->second->clone(dest);
            if (clonedSegment)
            {
                Uint16 segNumber = srcSegment->first;
                result = dest->addSegment(clonedSegment, segNumber);
                if (result.bad())
                {
                    DCMSEG_ERROR("Failed to add cloned segment to destination");
                    delete clonedSegment;
                    break;
                }
            }
            else
            {
                DCMSEG_ERROR("Failed to clone source segment");
                result = EC_MemoryExhausted;
                break;
            }
        }
        srcSegment++;
    }
    return result;
}

DcmBinToLabelConverter::~DcmBinToLabelConverter()
{
    // Nothing to do here, no dynamic memory allocated
}

OFCondition DcmBinToLabelConverter::checkSOPClassAndSegtype(const OFString& sopClassUID,
                                                            const OFString& segType)
{
    DCMSEG_DEBUG("Checking SOP Class and Segmentation Type");
    // Check if the SOP Class UID is correct
    OFCondition result;
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

    // Check if the Segmentation Type is valid
    DCMSEG_DEBUG("SOP Class acceptable for conversion to labelmap, checking Segmentation Type");
    if (DcmSegTypes::OFString2Segtype(segType) == DcmSegTypes::ST_UNKNOWN)
    {
        DCMSEG_ERROR("Segmentation Type is unknown");
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
            // TODO fix series instance UID and Series Number?
            DCMSEG_DEBUG("Copying General Series Module from input to output segmentation");
            result = copyComponent(&(src->getSeries()), &dest->getSeries());
        }
        if (result.good())
        {
            DCMSEG_DEBUG("Copying Segmentation Series Module from input to output segmentation");
            result = copyComponent(&(src->getSegmentationSeriesModule()), &dest->getSegmentationSeriesModule());
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
        // - Multi-frame Functional Group Module (recreate)
        // - Common Instance Reference Module (probably invalid)
        // Skipping:
        // - Palette Color LUT Module (not set in binary segmentations)
        // - Segmentation Image Module (rewritten for labelmaps)
        // - Multi-Frame Dimension Module (probably invalid)

        // TODO: Shall we copy Common Instance Reference module with its original references or just empty?
        // (Referenced Series Sequence must be there at least empty)
        if (result.good())
        {
            DCMSEG_DEBUG("Copying Common Instance Reference Module from input to output segmentation");
            result = copyComponent(&(src->getCommonInstanceReference()), &dest->getCommonInstanceReference());
        }
        if (result.bad())
            return result;

        // Multi-frame Functional Groups Module (through the attributes in General Image Module):
        // Set Instance Number
        if (result.good())
        {
            result = dest->getGeneralImage().setInstanceNumber("1");
        }

        // Multi-frame Dimension Module, re-create:
        // Two artificial dimensions based on Stack ID and In Stack Position Number
        if (result.good())
        {
            // Create new Dimension UID
            char uid[100];
            dcmGenerateUniqueIdentifier(uid, SITE_INSTANCE_UID_ROOT);
            result = dest->getDimensions().addDimensionIndex(DCM_StackID, uid, DCM_FrameContentSequence, "Stack ID");
            if (result.good()) result = dest->getDimensions().addDimensionIndex(DCM_InStackPositionNumber, uid, DCM_FrameContentSequence, "In Stack Position Number");
        }

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
        Uint32 outputFrameNum = 1; // for log output
        if (result.good())
        {
            // Iterate over all positions, and create a new destination frame
            DCMSEG_DEBUG("Creating new destination frames for each input frame position");
            OFVector<OverlapUtil::LogicalFrame>::iterator it = framesAtPositions.begin();
            while (result.good()&& (it != framesAtPositions.end()))
            {
                // Create per-frame functional groups.
                // Re-use Plane Position (Patient) FG from first frame at this position.
                // Create Frame Content FG for the frame
                FGBase* planePos = src->getFunctionalGroups().get(it->at(0), DcmFGTypes::EFG_PLANEPOSPATIENT);
                if (!planePos)
                {
                    DCMSEG_DEBUG("No Plane Position (Patient) FG found for frame #" << it->at(0));
                    result = EC_IllegalParameter; // TODO better code
                    break;
                }
                // Create Frame Content FG for the frame, TODO: refactor into separate method
                FGFrameContent* frameContent = OFstatic_cast(FGFrameContent*, FGFactory::instance().create(DcmFGTypes::EFG_FRAMECONTENT));
                if (frameContent)
                {
                    frameContent->setStackID("Frame Position");
                    frameContent->setInStackPositionNumber(outputFrameNum);
                    result = frameContent->setDimensionIndexValues(1, 0);
                    if (result.good())
                    {
                        result = frameContent->setDimensionIndexValues(outputFrameNum, 1);
                    }

                    // Create list of source frames that has been used for this frame and insert them as a comment
                    // in the form "Original frames at this position: x, y, ..."
                    if (result.good())
                    {
                        OFOStringStream s;
                        s << "Created from original frame numbers: ";
                        OverlapUtil::DistinctFramePositions::iterator frameAtPos = framesAtPositions.begin();
                        while (frameAtPos != framesAtPositions.end())
                        {
                            s << frameAtPos->at(0) << ", ";
                            ++frameAtPos;
                        }
                        OFString frameComments = s.str().c_str();
                        // cut off last comma, if applicable
                        if (frameComments.length() > 2) frameComments = frameComments.substr(0, frameComments.length() - 2);
                        frameContent->setFrameComments(frameComments);
                        result = m_outputSeg->getFunctionalGroups().addPerFrame(OFstatic_cast(Uint32, outputFrameNum - 1), *frameContent);
                    }
                    delete frameContent;
                }
                else
                {
                    result = EC_MemoryExhausted;
                }
                if (result.bad()) break;;

                OFVector<FGBase*> perFrameInfo;
                if (planePos) perFrameInfo.push_back(planePos);
                // addFrame() will copy functional groups. Memory is still handled by source object, so
                // no need to delete them.
                if (m_use16Bit)
                {
                    DCMSEG_DEBUG("Creating new 16 bit destination frame #" << outputFrameNum << "/" << framesAtPositions.size());
                    // create a new destination frame
                    Uint16* newFrame = new Uint16[src->getRows() * src->getColumns()];
                    if (newFrame)
                    {
                        // Initialize new frame with zeros
                        memset(newFrame, 0, src->getRows() * src->getColumns() * sizeof(Uint16));
                        result = setPixelDataForFrame(src, outputFrameNum-1 /* aka current position */, newFrame, src->getRows() * src->getColumns());
                        if (result.good())
                        {
                            result = m_outputSeg->addFrame(newFrame, 0 /* ignored for labelmaps */, perFrameInfo);
                        }
                        delete[] newFrame;
                    }
                    else result = EC_MemoryExhausted;
                }
                else // 8 bit
                {
                    DCMSEG_DEBUG("Creating new 8 bit destination frame #" << outputFrameNum << "/" << framesAtPositions.size());
                    Uint8* newFrame = new Uint8[src->getRows() * src->getColumns()];
                    if (newFrame)
                    {
                        // Initialize new frame with zeros
                        memset(newFrame, 0, src->getRows() * src->getColumns() * sizeof(Uint8));
                        result = setPixelDataForFrame(src, outputFrameNum-1 /* aka current position */, newFrame, src->getRows() * src->getColumns());
                        if (result.good())
                        {
                            result = m_outputSeg->addFrame(newFrame, 0 /* ignored for labelmaps */, perFrameInfo);
                        }
                        delete[] newFrame;
                    }
                    else result = EC_MemoryExhausted;
                }
                it++;
                outputFrameNum++;
            }
        }
    }
    else
    {
        result = EC_IllegalParameter;
    }
    return result;
}


E_TransferSyntax DcmBinToLabelConverter::getInputTransferSyntax() const
{
    return m_inputXfer;
}

OFCondition DcmBinToLabelConverter::loadInput()
{
    OFCondition result;
    // Check whether we have a segmentation object as input
    if (m_inputSeg.get() == OFnullptr)
    {
        // If not, check if we have a dataset to load from
        if (m_inputDataset == OFnullptr)
        {
            // If not, check if we can load it from file
            if (m_inputFileName.isEmpty())
            {
                return EC_InvalidFilename;
            }
            else
            {
                // Load file into dataset
                DCMSEG_DEBUG("Loading potential segmentation file into a dataset");
                DcmFileFormat dcmff;
                result = dcmff.loadFile(m_inputFileName);
                if (result.good())
                {
                    // make sure dataset pointer is not freed by DcmFileFormat
                    m_inputDataset = dcmff.getAndRemoveDataset();
                }
                else
                {
                    return result;
                }
            }
        }
        // At this point we have an input dataset, load it into segmentation
        DCMSEG_DEBUG("Loading dataset into DcmSegmentation object");
        DcmSegmentation *loaded = OFnullptr;
        result = DcmSegmentation::loadDataset(*m_inputDataset, loaded, m_loadFlags);
        if (result.good())
        {
            m_inputSeg.reset(loaded);
        }
        else
        {
            return result;
        }
    }

    OFString sop, segtype;
    m_inputDataset->findAndGetOFString(DCM_SOPClassUID, sop);
    m_inputDataset->findAndGetOFString(DCM_SegmentationType, segtype);
    return checkSOPClassAndSegtype(sop, segtype);
}


OFCondition DcmBinToLabelConverter::getOutputSegmentation(OFshared_ptr<DcmSegmentation>& outputSeg)
{
   outputSeg = m_outputSeg;
   return outputSeg ? EC_Normal : EC_IllegalParameter;
}


OFCondition DcmBinToLabelConverter::getOutputDataset(DcmItem& outputDataset)
{
    outputDataset.clear();
    if (m_outputSeg)
    {
        m_outputSeg->getFunctionalGroups().setUseThreads(m_convFlags.m_numThreads);
        m_outputSeg->getFunctionalGroups().setCheckOnWrite(m_convFlags.m_checkExportFG);
        m_outputSeg->setValueCheckOnWrite(m_convFlags.m_checkExportValues);
        OFCondition result = m_outputSeg->writeDataset(outputDataset);
        if (result.bad())
        {
            outputDataset.clear();
            return result;
        }
    }
    return EC_Normal;
}

