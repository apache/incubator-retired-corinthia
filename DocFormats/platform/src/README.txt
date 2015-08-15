***** Field description *****

Reference: https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT


"version made by" (2 bytes)
    Fixed set to 0x2D

    Upper byte mapping:
         0 - MS-DOS and OS/2 (FAT / VFAT / FAT32 file systems)
         1 - Amiga                     2 - OpenVMS
         3 - UNIX                      4 - VM/CMS
         5 - Atari ST                  6 - OS/2 H.P.F.S.
         7 - Macintosh                 8 - Z-System
         9 - CP/M                     10 - Windows NTFS
        11 - MVS (OS/390 - Z/OS)      12 - VSE
        13 - Acorn Risc               14 - VFAT
        15 - alternate MVS            16 - BeOS
        17 - Tandem                   18 - OS/400
        19 - OS X (Darwin)            20 thru 255 - unused

    Lower byte indicates the ZIP specification version 

 "Version needed to extract" (2 bytes)

    Fixed set to 0x14

    The minimum supported ZIP specification version needed 
    to extract the file, mapped as above.

    Current minimum feature versions are as defined below:
        1.0 - Default value
        1.1 - File is a volume label
        2.0 - File is a folder (directory)
        2.0 - File is compressed using Deflate compression
        2.0 - File is encrypted using traditional PKWARE encryption
        2.1 - File is compressed using Deflate64(tm)
        2.5 - File is compressed using PKWARE DCL Implode 
        2.7 - File is a patch data set 
        4.5 - File uses ZIP64 format extensions
        4.6 - File is compressed using BZIP2 compression*
        5.0 - File is encrypted using DES
        5.0 - File is encrypted using 3DES
        5.0 - File is encrypted using original RC2 encryption
        5.0 - File is encrypted using RC4 encryption
        5.1 - File is encrypted using AES encryption
        5.1 - File is encrypted using corrected RC2 encryption**
        5.2 - File is encrypted using corrected RC2-64 encryption**
        6.1 - File is encrypted using non-OAEP key wrapping***
        6.2 - Central directory encryption
        6.3 - File is compressed using LZMA
        6.3 - File is compressed using PPMd+
        6.3 - File is encrypted using Blowfish
        6.3 - File is encrypted using Twofish

"general purpose bit flag" (2 bytes)

   Fixed set to 0x06


   Bit 0: If set, indicates that the file is encrypted.

   Bit 2 / 1 (For Methods 8 and 9 - Deflating)
       0 / 0  Normal compression option was used.
       0 / 1  Maximum compression option was used.
       1 / 0  Fast compression option was used.
       1 / 1  Super Fast compression option was used.

   Bit 3: fields crc-32, compressed size and uncompressed size are zero
          in the local header.

   Bit 4: enhanced deflating. 

   Bit 5: compressed patched data.

   Bit 6: Strong encryption.

   Bit 7 / 8 / 9 / 10: Currently unused.

   Bit 11: Language encoding flag (using UTF-8).

   Bit 12: enhanced compression.

   Bit 13: encrypting the Central Directory to indicate 
           selected data values in the Local Header are masked to
            hide their actual values.

   Bit 14 / 15 / 16: Reserved by PKWARE.


"compression method"
   Fixed set to 0x08

    0 - The file is stored (no compression)
    1 - The file is Shrunk
    2 - The file is Reduced with compression factor 1
    3 - The file is Reduced with compression factor 2
    4 - The file is Reduced with compression factor 3
    5 - The file is Reduced with compression factor 4
    6 - The file is Imploded
    7 - Reserved for Tokenizing compression algorithm
    8 - The file is Deflated
    9 - Enhanced Deflating using Deflate64(tm)
   10 - PKWARE Data Compression Library Imploding (old IBM TERSE)
   11 - Reserved by PKWARE
   12 - File is compressed using BZIP2 algorithm
   13 - Reserved by PKWARE
   14 - LZMA (EFS)
   15 - Reserved by PKWARE
   16 - Reserved by PKWARE
   17 - Reserved by PKWARE
   18 - File is compressed using IBM TERSE (new)
   19 - IBM LZ77 z Architecture (PFS)
   97 - WavPack compressed data
   98 - PPMd version I, Rev 1


"date and time fields": (2 bytes each)
   Fixed set to 0x0000

   The date and time are encoded in standard MS-DOS format.

"CRC-32": (4 bytes)
    Calculated


"compressed size": (4 bytes)
    data buffer size


"uncompressed size": (4 bytes)
    Calculated


"file name length": (2 bytes)
    Calculated


"extra field length": (2 bytes)
    Fixed set to 0x0000


"file comment length": (2 bytes)
    Fixed set to 0x0000


"disk number start": (2 bytes)
    Fixed set to 0x0000


"internal file attributes": (2 bytes)
    Fixed set to 0x0000

    Bit 0 file is apparently an ASCII or text file.

    Bits 1 are reserved for use by PKWARE.

    Bits 2 a 4 byte variable record length control field precedes each 
        logical record indicating the length of the record.


"external file attributes": (4 bytes)
    Fixed set to 0x0000

    the low order byte is the MS-DOS directory attribute byte.


"relative offset of local header": (4 bytes)
    Calculated

    This is the offset from the start of the first disk on
    which this file appears, to where the local header should
    be found.


"file name": (Variable)
    Copied

    Name of the file, with optional relative path.
    All slashes MUST be forward slashes '/'


"file comment": (Variable)
    Not used


"number of this disk": (2 bytes)
    Fixed set to 0x0000


"number of the disk with the start of the central directory": (2 bytes)
    Fixed set to 0x0000


"total number of entries in the central dir": (2 bytes)
    Calculated

    The number of central directory entries on this disk.


"size of the central directory": (4 bytes)
    Calculated

    The size (in bytes) of the entire central directory.


"offset of start of central directory" (4 bytes)
    Calculated

    Offset of the start of the central directory in the file


".ZIP file comment length": (2 bytes)
    Calculated
    

    The length of the comment for this .ZIP file.

".ZIP file comment": (Variable)
    Set fix to "Created by Corinthia"

    The comment for this .ZIP file.


"zip64 extensible data sector" (variable size)
    Not used


"extra field": (Variable)
    Not used


