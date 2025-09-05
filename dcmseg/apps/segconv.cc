/*
 *
 *  Copyright (C) 2025, OFFIS e.V.
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
 *  Purpose: Convert binary segmentation to label map segmentation
 *
 */

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */

#include "dcmtk/dcmseg/bin2label.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/oflog/oflog.h"
#include "dcmtk/dcmdata/dcuid.h"       /* for dcmtk version name */
#include "dcmtk/dcmdata/dcostrmz.h"    /* for dcmZlibCompressionLevel */
#include "dcmtk/dcmdata/dcistrmz.h"    /* for dcmZlibExpectRFC1950Encoding */
#include "dcmtk/dcmdata/dcrledrg.h"    /* for DcmRLEDecoderRegistration */
#include "dcmtk/dcmdata/dcrleerg.h"    /* for DcmRLEEncoderRegistration */
#include "dcmtk/dcmdata/dcdict.h"      /* for dcmDataDict.rdlock() */

#ifdef WITH_ZLIB
#include <zlib.h>                      /* for zlibVersion() */
#endif

#define OFFIS_CONSOLE_APPLICATION "segconv"

static OFLogger segconvLogger = OFLog::getLogger("dcmtk.apps." OFFIS_CONSOLE_APPLICATION);

static char rcsid[] = "$dcmtk: " OFFIS_CONSOLE_APPLICATION " v"
  OFFIS_DCMTK_VERSION " " OFFIS_DCMTK_RELEASEDATE " $";

// ********************************************


#define SHORTCOL 3
#define LONGCOL 21


int main(int argc, char *argv[])
{
  const char *opt_ifname = NULL;
  const char *opt_ofname = NULL;

  E_FileReadMode opt_readMode = ERM_autoDetect;
  E_FileWriteMode opt_writeMode = EWM_createNewMeta;
  E_TransferSyntax opt_oxfer = EXS_Unknown;
  E_GrpLenEncoding opt_oglenc = EGL_recalcGL;
  E_EncodingType opt_oenctype = EET_ExplicitLength;
  E_PaddingEncoding opt_opadenc = EPD_noChange;
  DcmSegmentation::LoadingFlags opt_loadFlags;
  DcmBinToLabelConverter::ConversionFlags opt_convFlags;
#ifdef WITH_ZLIB
  OFCmdUnsignedInt opt_compressionLevel = 0;
#endif

  OFConsoleApplication app(OFFIS_CONSOLE_APPLICATION , "Convert DICOM segmentation objects", rcsid);
  OFCommandLine cmd;
  cmd.setOptionColumns(LONGCOL, SHORTCOL);
  cmd.setParamColumn(LONGCOL + SHORTCOL + 4);

  cmd.addParam("dcmfile-in",  "DICOM input filename to be converted\n(\"-\" for stdin)");
  cmd.addParam("dcmfile-out", "DICOM output filename\n(\"-\" for stdout)");

  cmd.addGroup("general options:", LONGCOL, SHORTCOL + 2);
    cmd.addOption("--help",                  "-h",     "print this help text and exit", OFCommandLine::AF_Exclusive);
    cmd.addOption("--version",                         "print version information and exit", OFCommandLine::AF_Exclusive);
    OFLog::addOptions(cmd);

  cmd.addGroup("input options:");
    cmd.addSubGroup("input file format:");
      cmd.addOption("--read-file",           "+f",     "read file format or data set (default)");
      cmd.addOption("--read-file-only",      "+fo",    "read file format only");
      cmd.addOption("--read-dataset",        "-f",     "read data set without file meta information");
    cmd.addSubGroup("input transfer syntax:", LONGCOL, SHORTCOL);
      cmd.addOption("--read-xfer-auto",      "-t=",    "use TS recognition (default)");
      cmd.addOption("--read-xfer-detect",    "-td",    "ignore TS specified in the file meta header");
      cmd.addOption("--read-xfer-little",    "-te",    "read with explicit VR little endian TS");
      cmd.addOption("--read-xfer-big",       "-tb",    "read with explicit VR big endian TS");
      cmd.addOption("--read-xfer-implicit",  "-ti",    "read with implicit VR little endian TS");
      cmd.addOption("--read-xfer-rle",       "-tr",    "read with RLE lossless TS");
#ifdef WITH_ZLIB
    cmd.addSubGroup("bitstream format of deflated input:");
      cmd.addOption("--bitstream-deflated",  "+bd",    "expect deflated bitstream (default)");
      cmd.addOption("--bitstream-zlib",      "+bz",    "expect deflated zlib bitstream");
#endif

  cmd.addGroup("processing options:");
    cmd.addOption("--num-threads",                           "-j",      1, "[n]um threads: integer (default: 1)",
                                                                                                          "use n threads if possible");
    cmd.addOption("--disable-fg-check",        "-fgc",   "disable checking of functional groups\nwhen writing");
    cmd.addOption("--disable-value-check",      "-vc",   "disable checking of values\nwhen writing");
  cmd.addGroup("output options:");
    cmd.addSubGroup("output file format:");
      cmd.addOption("--write-new-meta-info", "+Fm",    "write file format\nwith new meta information (default)");
      cmd.addOption("--write-file",          "+F",     "write file format");
      cmd.addOption("--write-dataset",       "-F",     "write data set without file meta information");
    cmd.addSubGroup("output transfer syntax:");
      cmd.addOption("--write-xfer-same",     "+t=",    "write with same TS as input (default)");
      cmd.addOption("--write-xfer-little",   "+te",    "write with explicit VR little endian TS");
      cmd.addOption("--write-xfer-big",      "+tb",    "write with explicit VR big endian TS");
      cmd.addOption("--write-xfer-implicit", "+ti",    "write with implicit VR little endian TS");
      cmd.addOption("--write-xfer-rle",      "+tr",    "write with RLE lossless TS");
#ifdef WITH_ZLIB
      cmd.addOption("--write-xfer-deflated", "+td",    "write with deflated explicit VR little endian TS");
#endif
    cmd.addSubGroup("length encoding in sequences and items:");
      cmd.addOption("--length-explicit",     "+e",     "write with explicit lengths (default)");
      cmd.addOption("--length-undefined",    "-e",     "write with undefined lengths");
      cmd.addOption("--write-oversized",     "+eo",    "write oversized explicit length sequences\nand items with undefined length (default)");
      cmd.addOption("--abort-oversized",     "-eo",    "abort on oversized explicit sequences/items");
#ifdef WITH_ZLIB
    cmd.addSubGroup("deflate compression level (only with --write-xfer-deflated):");
      cmd.addOption("--compression-level",   "+cl", 1, "[l]evel: integer (default: 6)",
                                                       "0=uncompressed, 1=fastest, 9=best compression");
#endif

    /* evaluate command line */
    prepareCmdLineArgs(argc, argv, OFFIS_CONSOLE_APPLICATION);
    if (app.parseCommandLine(cmd, argc, argv))
    {
      /* check exclusive options first */
      if (cmd.hasExclusiveOption())
      {
          if (cmd.findOption("--version"))
          {
              app.printHeader(OFTrue /*print host identifier*/);
              COUT << OFendl << "External libraries used:";
#if !defined(WITH_ZLIB) && !defined(DCMTK_ENABLE_CHARSET_CONVERSION)
              COUT << " none" << OFendl;
#else
              COUT << OFendl;
#endif
#ifdef WITH_ZLIB
              COUT << "- ZLIB, Version " << zlibVersion() << OFendl;
#endif
              return 0;
          }
      }

      /* command line parameters */
      cmd.getParam(1, opt_ifname);
      cmd.getParam(2, opt_ofname);

      /* general options */
      OFLog::configureFromCommandLine(cmd, app);

      /* input options */
      cmd.beginOptionBlock();
      if (cmd.findOption("--read-file")) opt_readMode = ERM_autoDetect;
      if (cmd.findOption("--read-file-only")) opt_readMode = ERM_fileOnly;
      if (cmd.findOption("--read-dataset")) opt_readMode = ERM_dataset;
      cmd.endOptionBlock();

      cmd.beginOptionBlock();
      if (cmd.findOption("--read-xfer-auto"))
        opt_loadFlags.m_readTransferSyntax = EXS_Unknown;
      if (cmd.findOption("--read-xfer-detect"))
        dcmAutoDetectDatasetXfer.set(OFTrue);
      if (cmd.findOption("--read-xfer-little"))
      {
        app.checkDependence("--read-xfer-little", "--read-dataset", opt_readMode == ERM_dataset);
        opt_loadFlags.m_readTransferSyntax = EXS_LittleEndianExplicit;
      }
      if (cmd.findOption("--read-xfer-big"))
      {
        app.checkDependence("--read-xfer-big", "--read-dataset", opt_readMode == ERM_dataset);
        opt_loadFlags.m_readTransferSyntax = EXS_BigEndianExplicit;
      }
      if (cmd.findOption("--read-xfer-implicit"))
      {
        app.checkDependence("--read-xfer-implicit", "--read-dataset", opt_readMode == ERM_dataset);
        opt_loadFlags.m_readTransferSyntax = EXS_LittleEndianImplicit;
      }
      if (cmd.findOption("--read-xfer-rle"))
      {
        app.checkDependence("--read-xfer-rle", "--read-dataset", opt_readMode == ERM_dataset);
        opt_loadFlags.m_readTransferSyntax = EXS_RLELossless;
        DcmRLEDecoderRegistration::registerCodecs();
      }
      cmd.endOptionBlock();

#ifdef WITH_ZLIB
      cmd.beginOptionBlock();
      if (cmd.findOption("--bitstream-deflated"))
      {
        dcmZlibExpectRFC1950Encoding.set(OFFalse);
      }
      if (cmd.findOption("--bitstream-zlib"))
      {
        dcmZlibExpectRFC1950Encoding.set(OFTrue);
      }
      cmd.endOptionBlock();
#endif

      /* processing options */
      cmd.beginOptionBlock();
      if (cmd.findOption("--num-threads"))
      {
        OFCmdUnsignedInt opt_numThreads = 1;
        cmd.getValueAndCheckMinMax(opt_numThreads, 1, 255);
        opt_loadFlags.m_numThreads = OFstatic_cast(Uint32, opt_numThreads); // safe
        opt_convFlags.m_numThreads = OFstatic_cast(Uint32, opt_numThreads); // safe
      }
      cmd.endOptionBlock();
      if (cmd.findOption("--disable-fg-check"))
      {
          opt_convFlags.m_checkExportFG = OFTrue;
      }
      if (cmd.findOption("--disable-value-check"))
      {
          opt_convFlags.m_checkExportValues= OFTrue;
      }

      /* output options */
      cmd.beginOptionBlock();
      if (cmd.findOption("--write-file")) opt_writeMode = EWM_fileformat;
      if (cmd.findOption("--write-new-meta-info")) opt_writeMode = EWM_createNewMeta;
      if (cmd.findOption("--write-dataset")) opt_writeMode = EWM_dataset;
      cmd.endOptionBlock();

      cmd.beginOptionBlock();
      if (cmd.findOption("--write-xfer-same")) opt_oxfer = EXS_Unknown;
      if (cmd.findOption("--write-xfer-little")) opt_oxfer = EXS_LittleEndianExplicit;
      if (cmd.findOption("--write-xfer-big")) opt_oxfer = EXS_BigEndianExplicit;
      if (cmd.findOption("--write-xfer-implicit")) opt_oxfer = EXS_LittleEndianImplicit;
      if (cmd.findOption("--write-xfer-rle"))
      {
          opt_oxfer = EXS_RLELossless;
          DcmRLEEncoderRegistration::registerCodecs();
      }
#ifdef WITH_ZLIB
      if (cmd.findOption("--write-xfer-deflated")) opt_oxfer = EXS_DeflatedLittleEndianExplicit;
#endif
      cmd.endOptionBlock();

      cmd.beginOptionBlock();
      if (cmd.findOption("--length-explicit")) opt_oenctype = EET_ExplicitLength;
      if (cmd.findOption("--length-undefined")) opt_oenctype = EET_UndefinedLength;
      cmd.endOptionBlock();

      cmd.beginOptionBlock();
      if (cmd.findOption("--write-oversized")) dcmWriteOversizedSeqsAndItemsUndefined.set(OFTrue);
      if (cmd.findOption("--abort-oversized")) dcmWriteOversizedSeqsAndItemsUndefined.set(OFFalse);
      cmd.endOptionBlock();

#ifdef WITH_ZLIB
      if (cmd.findOption("--compression-level"))
      {
        app.checkDependence("--compression-level", "--write-xfer-deflated", opt_oxfer == EXS_DeflatedLittleEndianExplicit);
        app.checkValue(cmd.getValueAndCheckMinMax(opt_compressionLevel, 0, 9));
        dcmZlibCompressionLevel.set(OFstatic_cast(int, opt_compressionLevel));
      }
#endif
    }

    /* print resource identifier */
    OFLOG_DEBUG(segconvLogger, rcsid << OFendl);

    /* make sure data dictionary is loaded */
    if (!dcmDataDict.isDictionaryLoaded())
    {
        OFLOG_WARN(segconvLogger, "no data dictionary loaded, check environment variable: "
            << DCM_DICT_ENVIRONMENT_VARIABLE);
    }

    /* open input file */

    /* open input file */
    if ((opt_ifname == NULL) || (strlen(opt_ifname) == 0))
    {
        OFLOG_FATAL(segconvLogger, "invalid filename: <empty string>");
        return 1;
    }

    OFLOG_INFO(segconvLogger, "open input file " << opt_ifname);

    DcmBinToLabelConverter converter;
    converter.setInput(opt_ifname, opt_loadFlags);
    OFCondition error = converter.convert(opt_convFlags);
    if (error.bad())
    {
        OFLOG_FATAL(segconvLogger, error.text() << ": converting file: " <<  opt_ifname);
        return 1;
    }

    // write output file

    OFunique_ptr<DcmDataset> labelMap(new DcmDataset());
    error = converter.getOutputDataset(*labelMap);
    if (error.bad())
    {
        OFLOG_FATAL(segconvLogger, error.text() << ": writing segmentation into dataset");
        return 1;
    }
    if (opt_oxfer == EXS_Unknown)
    {
        // use input transfer syntax
        opt_oxfer = converter.getInputTransferSyntax();
        if (opt_oxfer == EXS_Unknown)
        {
          // can theoretically be returned in case of concatenations in the underlying API,
          // but those are not yet supported by this tool. Add it already to not forget it later
          // in case support gets added in the future.
          OFLOG_DEBUG(segconvLogger, "cannot determine transfer syntax of input file, using " << DcmXfer(EXS_LittleEndianExplicit).getXferName());
          opt_oxfer = EXS_LittleEndianExplicit;
        }
    }
    if (labelMap->chooseRepresentation(opt_oxfer, NULL).bad() || !labelMap->canWriteXfer(opt_oxfer))
    {
        OFLOG_FATAL(segconvLogger, "no conversion to transfer syntax " << DcmXfer(opt_oxfer).getXferName() << " possible!");
        return 1;
    }
    DcmXfer opt_oxferSyn(opt_oxfer);
    if (error.good())
    {
        OFLOG_INFO(segconvLogger, "output transfer syntax " << opt_oxferSyn.getXferName() << " can be written");
    } else {
        OFLOG_FATAL(segconvLogger, "no conversion to transfer syntax " << opt_oxferSyn.getXferName() << " possible!");
        return 1;
    }

    // actually write output file
    OFLOG_INFO(segconvLogger, "create output file " << opt_ofname);
    DcmFileFormat fileformat(labelMap.release(), OFFalse /* no deep copy */);
    error = fileformat.saveFile(opt_ofname, opt_oxfer, opt_oenctype, opt_oglenc, opt_opadenc,
        0, 0, opt_writeMode);

    if (error.bad())
    {
        OFLOG_FATAL(segconvLogger, error.text() << ": writing file: " <<  opt_ofname);
        return 1;
    }

    OFLOG_INFO(segconvLogger, "conversion successful");

    return 0;
}
