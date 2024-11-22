/*
 *
 *  Copyright (C) 2022-2024, OFFIS e.V.
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
 *  Module:  dcmseg
 *
 *  Author:  Michael Onken
 *
 *  Purpose: Test for creating and reading Segmentation with many frames/segments
 *
 */

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmseg/segtypes.h" /* for DCMSEG_DEBUG */
#include "dcmtk/dcmseg/segutils.h"

#include "dcmtk/ofstd/ofcond.h"
#include "dcmtk/dcmseg/segdoc.h"

#include <iostream>

static void printFile(const OFString& file1);

// Read a segmentation file from first command line argument and
// write it to output file name provided second command line argument.
int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        DCMSEG_ERROR("Usage: " << argv[0] << " <input-file> <output-file>");
        return 1;
    }
    dcmtk::log4cplus::Logger rootLogger = dcmtk::log4cplus::Logger::getRoot();
    //rootLogger.setLogLevel(dcmtk::log4cplus::DEBUG_LOG_LEVEL);
    DcmSegmentation* segdoc = NULL;
    OFCondition result = DcmSegmentation::loadFile(argv[1], segdoc);
    if (result.bad())
    {
        DCMSEG_ERROR("Error loading segmentation file: " << result.text());
        return 1;
    }
    result = segdoc->saveFile(argv[2]);
    if (result.bad())
    {
        DCMSEG_ERROR("Error saving segmentation file: " << result.text());
        return 1;
    }

    // Open both files and compare pixel data element values
    DcmFileFormat ff;
    result = ff.loadFile(argv[1]);
    if (result.bad())
    {
        DCMSEG_ERROR("Error loading file " << argv[1] << ": " << result.text());
        return 1;
    }
    DcmDataset* dset1 = ff.getDataset();
    result = ff.loadFile(argv[2]);
    if (result.bad())
    {
        DCMSEG_ERROR("Error loading file " << argv[2] << ": " << result.text());
        return 1;
    }
    DcmDataset* dset2 = ff.getDataset();
    const Uint8* pixData1 = NULL;
    const Uint8* pixData2 = NULL;
    unsigned long count1 = 0;
    unsigned long count2 = 0;
    result = dset1->findAndGetUint8Array(DCM_PixelData, pixData1, &count1);
    if (result.bad())
    {
        DCMSEG_ERROR("Error reading pixel data from file " << argv[1] << ": " << result.text());
        return 1;
    }
    result = dset2->findAndGetUint8Array(DCM_PixelData, pixData2, &count2);
    if (result.bad())
    {
        DCMSEG_ERROR("Error reading pixel data from file " << argv[2] << ": " << result.text());
        return 1;
    }
    if (count1 != count2)
    {
        DCMSEG_ERROR("Pixel data length differs between files: " << count1 << " vs. " << count2);
        return 1;
    }
    if (memcmp(pixData1, pixData2, OFstatic_cast(size_t, count1)) != 0)
    {
        DCMSEG_ERROR("Pixel data differs between files");
        return 1;
    }

    std::cout << "Pixel data is identical in both files" << std::endl;

    // printFile(argv[1]);
    // std::cout << std::endl;
    // printFile(argv[2]);

    return 0;
}


void printFile(const OFString& file1)
{
    // Now, load both files again into a DcmFileformat, get pixel data and dump it
    // bitwise (1101.... ).  Make a new line after each frame.
    DcmFileFormat dcmff1;
    OFCondition result = dcmff1.loadFile(file1.c_str());
    if (result.bad())
    {
        DCMSEG_ERROR("Error loading file " << file1 << ": " << result.text());
        return;
    }
    const Uint8* pixData1 = NULL;
    Uint16 rows1 = 0;
    Uint16 cols1 = 0;
    Sint32 numberOfFrames1 = 0;
    dcmff1.getDataset()->findAndGetUint16(DCM_Rows, rows1);
    dcmff1.getDataset()->findAndGetUint16(DCM_Columns, cols1);
    dcmff1.getDataset()->findAndGetSint32(DCM_NumberOfFrames, numberOfFrames1);
    dcmff1.getDataset()->findAndGetUint8Array(DCM_PixelData, pixData1);
    DCMSEG_DEBUG("Pixel data from file " << file1 << " (" << rows1 << "x" << cols1 << "x" << numberOfFrames1 << "):");
    DcmSegUtils::debugDumpBin(OFconst_cast(Uint8*, pixData1), OFstatic_cast(size_t, rows1) * cols1 * numberOfFrames1, file1.c_str(), OFTrue /* raw */);
    return;
}