/*
 *
 *  Copyright (C) 2024, Open Connections GmbH
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
 *  Purpose: Class for managing the Palette Color Lookup Table Module
 *
 */

#ifndef MODPALETTECOLORLUT_H
#define MODPALETTECOLORLUT_H

#include "dcmtk/config/osconfig.h"

#include "dcmtk/ofstd/ofmem.h"
#include "dcmtk/dcmiod/iodrules.h"
#include "dcmtk/dcmiod/modbase.h"
#include "dcmtk/ofstd/ofcond.h"

/** Class representing the Palette Color Lookup Table Module:
 *
 * Red Palette Color Lookup Table​ Descriptor (0028,1101): (US or SS, 3, 1)
 * Green Palette Color Lookup Table​ Descriptor (0028,1102): (US or SS, 3, 1)
 * Blue Palette Color Lookup Table​ Descriptor (0028,1103): (US or SS, 3, 1)
 * Palette Color Lookup Table UID (0028,1199): (UI, 1, 3)
 * Red Palette Color Lookup Table​ Data​ (0028,1201): (OW, 1-n, 1C)
 * Green Palette Color Lookup Table​ Data​ (0028,1202): (OW, 1-n, 1C)
 * Blue Palette Color Lookup Table​ Data​ (0028,1203): (OW, 1-n, 1C)
 * Segmented Red Palette Color Lookup Table​ Data​ (0028,1221): (OW, 1-n, 1C)
 * Segmented Green Palette Color Lookup Table​ Data​ (0028,1222): (OW, 1-n, 1C)
 * Segmented Blue Palette Color Lookup Table​ Data​ (0028,1223): (OW, 1-n, 1C)
 */
class DCMTK_DCMIOD_EXPORT IODPaletteColorLUTModule : public IODModule
{

public:
    /** Constructor
     *  @param  item The item to be used for data storage. If NULL, the
     *          class creates an empty data container.
     *  @param  rules The rule set for this class. If NULL, the class creates
     *          one from scratch and adds its values.
     */
    IODPaletteColorLUTModule(OFshared_ptr<DcmItem> item, OFshared_ptr<IODRules> rules);

    /** Constructor
     */
    IODPaletteColorLUTModule();

    /** Destructor
     */
    virtual ~IODPaletteColorLUTModule();

    /** Clear all attributes from the data that are handled by this module.
     *  An attribute is considered belonging to the module if there are rules
     *  marked as belonging to this module via the rule's module name.
     */
    virtual void clearData();

    /** Resets rules to their original values
     */
    virtual void resetRules();

    /** Get name of module
     *  @return Name of the module ("PaletteColorLookupTableModule")
     */
    virtual OFString getName() const;

    /** Read attributes from given item into this class
     *  @param source  The source to read from
     *  @param clearOldData If OFTrue, old data is cleared before reading. Otherwise
     *         old data is overwritten (or amended)
     *  @result EC_Normal if reading was successful, error otherwise
     */
    virtual OFCondition read(DcmItem& source, const OFBool clearOldData = OFTrue);

    /** Write attributes from this class into given item
     *  @param  destination The item to write to
     *  @result EC_Normal if writing was successful, error otherwise
     */
    virtual OFCondition write(DcmItem& destination);

    // ---------------- Getters -----------------------------

    /** Return whether palette data is signed or not
     *  @return OFTrue if signed, OFFalse otherwise
     */
    virtual OFBool isSigned() const;

    virtual Uint8 numBits();

    /** Get the Red Palette Color Lookup Table​ Descriptor
     *  @param  value The value of Red Palette Color Lookup Table​ Descriptor
     *  @param  pos The position of the value to be retrieved (0..2)
     *  @return EC_Normal if value is found, an error code otherwise
     */
    template <typename T>
    OFCondition getRedPaletteColorLookupTableDescriptor(T& value, const unsigned long pos = 0) const;

    /** Get the Green Palette Color Lookup Table​ Descriptor
     *  @param  value The value of Green Palette Color Lookup Table​ Descriptor
     *  @param  pos The position of the value to be retrieved (0..2)
     *  @return EC_Normal if value is found, an error code otherwise
     */
    template <typename T>
    OFCondition getGreenPaletteColorLookupTableDescriptor(T& value, const unsigned long pos = 0) const;

    /** Get the Blue Palette Color Lookup Table​ Descriptor
     *  @param  value The value of Blue Palette Color Lookup Table​ Descriptor
     *  @param  pos The position of the value to be retrieved (0..2)
     *  @return EC_Normal if value is found, an error code otherwise
     */
    template <typename T>
    OFCondition getBluePaletteColorLookupTableDescriptor(T& value, const unsigned long pos = 0) const;

    /** Get the Palette Color Lookup Table UID
     *  @param  value Reference to variable in which the value should be stored
     *  @param  pos Index of the value to get (0..vm-1), -1 for all components
     *  @return EC_Normal if successful, an error code otherwise
     */
    virtual OFCondition getPaletteColorLookupTableUID(OFString& value, const signed long pos = 0) const;

    /** Get the Red Palette Color Lookup Table​ Data​
     *  @param  value Reference to variable in which the value should be stored
     *  @param  numEntries Number of entries in the lookup table
     *  @return EC_Normal if successful, an error code otherwise
     */
    virtual OFCondition getRedPaletteColorLookupTableData(const Uint16*& value, unsigned long& num_entries);

    /** Get the Green Palette Color Lookup Table​ Data​
     *  @param  value Reference to variable in which the value should be stored
     *  @param  numEntries Number of entries in the lookup table
     *  @return EC_Normal if successful, an error code otherwise
     */
    virtual OFCondition getGreenPaletteColorLookupTableData(const Uint16*& value, unsigned long& num_entries);

    /** Get the Blue Palette Color Lookup Table​ Data​
     *  @param  value Reference to variable in which the value should be stored
     *  @param  numEntries Number of entries in the lookup table
     *  @return EC_Normal if successful, an error code otherwise
     */
    virtual OFCondition getBluePaletteColorLookupTableData(const Uint16*& value, unsigned long& num_entries);

    /** Get the Segmented Red Palette Color Lookup Table​ Data​
     *  @param  value Reference to variable in which the value should be stored
     *  @param  numEntries Number of entries in the lookup table
     *  @return EC_Normal if successful, an error code otherwise
     */
    virtual OFCondition getSegmentedRedPaletteColorLookupTableData(const Uint16*& value, unsigned long& num_entries);

    /** Get the Segmented Green Palette Color Lookup Table​ Data​
     *  @param  value Reference to variable in which the value should be stored
     *  @param  numEntries Number of entries in the lookup table
     *  @return EC_Normal if successful, an error code otherwise
     */
    virtual OFCondition getSegmentedGreenPaletteColorLookupTableData(const Uint16*& value, unsigned long& num_entries);

    /** Get the Segmented Blue Palette Color Lookup Table​ Data​
     *  @param  value Reference to variable in which the value should be stored
     *  @param  numEntries Number of entries in the lookup table
     *  @return EC_Normal if successful, an error code otherwise
     */
    virtual OFCondition getSegmentedBluePaletteColorLookupTableData(const Uint16*& value, unsigned long& num_entries);

    // ---------------- Setters -----------------------------

    /** Set the Red Palette Color Lookup Table​ Descriptor
     *  @param  value The value of Red Palette Color Lookup Table​ Descriptor
     *  @param  pos The position of the value to be set (0..2)
     *  @return EC_Normal if value is set, an error code otherwise
     */
    template <typename T>
    OFCondition setRedPaletteColorLookupTableDescriptor(const T& value, const unsigned long pos = 0);

    /** Set the Green Palette Color Lookup Table​ Descriptor
     *  @param  value The value of Green Palette Color Lookup Table​ Descriptor
     *  @param  pos The position of the value to be set (0..2)
     *  @return EC_Normal if value is set, an error code otherwise
     */
    template <typename T>
    OFCondition setGreenPaletteColorLookupTableDescriptor(const T& value, const unsigned long pos = 0);

    template <typename T>
    OFCondition setBluePaletteColorLookupTableDescriptor(const T& value, const unsigned long pos = 0);

    /** Set the Palette Color Lookup Table UID
     *  @param  value The value of Palette Color Lookup Table UID
     *  @param  checkValue Check value for conformance with VR (UI) and VM (1) if OFTrue
     *  @return EC_Normal if value is set, an error code otherwise
     */
    virtual OFCondition setPaletteColorLookupTableUID(const OFString& value, const OFBool checkValue = OFTrue);

    /** Set the Red Palette Color Lookup Table​ Data​
     *  @param  value The value of Red Palette Color Lookup Table​ Data​
     *  @param  numEntries Number of entries in the lookup table
     *  @param  checkValue Check value for conformance with VR (OW) and VM (1-n) if OFTrue
     *  @return EC_Normal if value is set, an error code otherwise
     */
    virtual OFCondition
    setRedPaletteColorLookupTableData(const Uint16* value, const size_t numEntries, const OFBool checkValue = OFTrue);

    /** Set the Green Palette Color Lookup Table​ Data​
     *  @param  value The value of Red Palette Color Lookup Table​ Data​
     *  @param  numEntries Number of entries in the lookup table
     *  @param  checkValue Check value for conformance with VR (OW) and VM (1-n) if OFTrue
     *  @return EC_Normal if value is set, an error code otherwise
     */
    virtual OFCondition
    setGreenPaletteColorLookupTableData(const Uint16* value, const size_t numEntries, const OFBool checkValue = OFTrue);

    /** Set the Blue Palette Color Lookup Table​ Data​
     *  @param  value The value of Red Palette Color Lookup Table​ Data​
     *  @param  numEntries Number of entries in the lookup table
     *  @param  checkValue Check value for conformance with VR (OW) and VM (1-n) if OFTrue
     *  @return EC_Normal if value is set, an error code otherwise
     */
    virtual OFCondition
    setBluePaletteColorLookupTableData(const Uint16* value, const size_t numEntries, const OFBool checkValue = OFTrue);

    /** Set the Segmented Red Palette Color Lookup Table​ Data​
     *  @param  value The value of Red Palette Color Lookup Table​ Data​
     *  @param  numEntries Number of entries in the lookup table
     *  @param  checkValue Check value for conformance with VR (OW) and VM (1-n) if OFTrue
     *  @return EC_Normal if value is set, an error code otherwise
     */
    virtual OFCondition setSegmentedRedPaletteColorLookupTableData(const Uint16* value,
                                                           const size_t numEntries,
                                                           const OFBool checkValue = OFTrue);

    /** Set the Segmented Green Palette Color Lookup Table​ Data​
     *  @param  value The value of Red Palette Color Lookup Table​ Data​
     *  @param  numEntries Number of entries in the lookup table
     *  @param  checkValue Check value for conformance with VR (OW) and VM (1-n) if OFTrue
     *  @return EC_Normal if value is set, an error code otherwise
     */
    virtual OFCondition setSegmentedGreenPaletteColorLookupTableData(const Uint16* value,
                                                             const size_t numEntries,
                                                             const OFBool checkValue = OFTrue);

    /** Set the Segmented Blue Palette Color Lookup Table​ Data​
     *  @param  value The value of Red Palette Color Lookup Table​ Data​
     *  @param  numEntries Number of entries in the lookup table
     *  @param  checkValue Check value for conformance with VR (OW) and VM (1-n) if OFTrue
     *  @return EC_Normal if value is set, an error code otherwise
     */
    virtual OFCondition setSegmentedBluePaletteColorLookupTableData(const Uint16* value,
                                                            const size_t numEntries,
                                                            const OFBool checkValue = OFTrue);

    // ---------------- Convenience Setters -----------------------------

    /** Set the Palette Color Lookup Table​ Data​
     *  @param  valueRed The value of Red Palette Color Lookup Table​ Data​
     *  @param  valueGreen The value of Green Palette Color Lookup Table​ Data​
     *  @param  valueBlue The value of Blue Palette Color Lookup Table​ Data​
     *  @param  numEntries Number of entries in each of the lookup tables
     *  @param  checkValue Check value for conformance with VR (OW) and VM (1-n) if OFTrue
     *  @return EC_Normal if value is set, an error code otherwise
     */
    virtual OFCondition setPaletteColorLookupTableData(const Uint16* valueRed,
                                               const Uint16* valueGreen,
                                               const Uint16* valueBlue,
                                               const size_t numEntries,
                                               const OFBool checkValue = OFTrue);

    virtual OFCondition setSegmentedPaletteColorLookupTableData(const Uint16* valueRed,
                                                        const Uint16* valueGreen,
                                                        const Uint16* valueBlue,
                                                        const size_t numEntries,
                                                        const OFBool checkValue = OFTrue);
    /**
     * Sets the red palette color lookup table descriptor.
     * @param numEntries The number of entries in the lookup table.
     * @param firstValueMapped The first value mapped in the lookup table.
     * @param numBitsPerEntry The number of bits per entry in the lookup table.
     *
     * @return EC_Normal if value is set, an error code otherwise
     */
    template <typename T>
    OFCondition setRedPaletteColorLookupTableDescriptor(const Uint16 numEntries,
                                                        const T firstValueMapped,
                                                        const Uint8 numBitsPerEntry);

    /**
     * Sets the green palette color lookup table descriptor.
     * @param numEntries The number of entries in the lookup table.
     * @param firstValueMapped The first value mapped in the lookup table.
     * @param numBitsPerEntry The number of bits per entry in the lookup table.
     *
     * @return EC_Normal if value is set, an error code otherwise
     */
    template <typename T>
    OFCondition setGreenPaletteColorLookupTableDescriptor(const Uint16 numEntries,
                                                          const T firstValueMapped,
                                                          const Uint8 numBitsPerEntry);

    /**
     * Sets the blue palette color lookup table descriptor.
     * @param numEntries The number of entries in the lookup table.
     * @param firstValueMapped The first value mapped in the lookup table.
     * @param numBitsPerEntry The number of bits per entry in the lookup table.
     *
     * @return EC_Normal if value is set, an error code otherwise
     */
    template <typename T>
    OFCondition setBluePaletteColorLookupTableDescriptor(const Uint16 numEntries,
                                                         const T firstValueMapped,
                                                         const Uint8 numBitsPerEntry);

protected:

    virtual OFBool checkLUT(const DcmTagKey& descriptorTag,
                    const DcmTagKey& dataTag);

    virtual OFBool isSigned(const DcmTagKey& descriptorTag);

    virtual OFBool checkDescriptorConsistency(const OFBool& isError);

    virtual OFBool checkSegmentConditions(const OFBool& isError);

    virtual void reportLUTError(const DcmTagKey& descriptorTag,
                        const OFString& message,
                        const OFBool& isError);

private:
    /// The module's name ("GeneralIamgeModule")
    static const OFString m_ModuleName;

    OFBool m_IsSigned;
};

#endif // MODPALETTECOLORLUT_H
