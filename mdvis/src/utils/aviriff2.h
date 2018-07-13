//------------------------------------------------------------------------------
// File: AVIRIFF.h
//
// Desc: Structures and defines for the RIFF AVI file format extended to
//       handle very large/LONG_T files.
//
// Copyright (c) 1996 - 2001, Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------
// Modified for cross platform

#if !defined AVIRIFF_H
#define AVIRIFF_H

#include <stdint.h>

typedef char TCHAR;
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t UINT;
typedef UINT LONG_T;
typedef UINT DWORD_T;
typedef UINT FOURCC;
typedef uint64_t DWORDLONG;

#if !defined NUMELMS
#define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif

// all structures in this file are packed on word boundaries
//
#include <pshpack2.h>

/*
* heres the general layout of an AVI riff file (new format)
*
* RIFF (3F??????) AVI       <- not more than 1 GB in size
*     LIST (size) hdrl
*         avih (0038)
*         LIST (size) strl
*             strh (0038)
*             strf (0028)
*             //indx (3ff8)   <- size may vary, should be sector sized
*         //LIST (size) strl
*             //strh (0038)
*             //strf (????)
*             //indx (3ff8)   <- size may vary, should be sector sized
*         //LIST (size) odml
*             //dmlh (????)
*         JUNK (size)       <- fill to align to sector - 12
*     LIST (7f??????) movi  <- aligned on sector - 12
*         00dc (size)       <- sector aligned
*         01wb (size)       <- sector aligned
*         ix00 (size)       <- sector aligned
*     idx1 (00??????)       <- sector aligned
* RIFF (7F??????) AVIX
*     JUNK (size)           <- fill to align to sector -12
*     LIST (size) movi
*         00dc (size)       <- sector aligned
* RIFF (7F??????) AVIX      <- not more than 2GB in size
*     JUNK (size)           <- fill to align to sector - 12
*     LIST (size) movi
*         00dc (size)       <- sector aligned
*
*-===================================================================*/

//
// structures for manipulating RIFF headers
//
#define FCC(ch4) ((((DWORD_T)(ch4) & 0xFF) << 24) |     \
                  (((DWORD_T)(ch4) & 0xFF00) << 8) |    \
                  (((DWORD_T)(ch4) & 0xFF0000) >> 8) |  \
                  (((DWORD_T)(ch4) & 0xFF000000) >> 24))

typedef struct _riffchunk {
	FOURCC fcc;
	DWORD_T  cb;
} RIFFCHUNK, *LPRIFFCHUNK;
typedef struct _rifflist {
	FOURCC fcc;
	DWORD_T  cb;
	FOURCC fccListType;
} RIFFLIST, *LPRIFFLIST;

#define RIFFROUND(cb) ((cb) + ((cb)&1))
#define RIFFNEXT(pChunk) (LPRIFFCHUNK)((LPBYTE)(pChunk) \
                          + sizeof(RIFFCHUNK) \
                          + RIFFROUND(((LPRIFFCHUNK)pChunk)->cb))


//
// ==================== avi header structures ===========================
//

// main header for the avi file (compatibility header)
//
#define ckidMAINAVIHEADER FCC('avih')
typedef struct _avimainheader {
	FOURCC fcc;                    // 'avih'
	const DWORD_T  cb = 0x38;                     // size of this structure -8
	DWORD_T  dwMicroSecPerFrame;     // frame display rate (or 0L)
	DWORD_T  dwMaxBytesPerSec;       // max. transfer rate
	DWORD_T  dwPaddingGranularity;   // pad to multiples of this size; normally 2K.
	DWORD_T  dwFlags;                // the ever-present flags
#define AVIF_HASINDEX        0x00000010 // Index at end of file?
#define AVIF_MUSTUSEINDEX    0x00000020
#define AVIF_ISINTERLEAVED   0x00000100
#define AVIF_TRUSTCKTYPE     0x00000800 // Use CKType to find key frames
#define AVIF_WASCAPTUREFILE  0x00010000
#define AVIF_COPYRIGHTED     0x00020000
	DWORD_T  dwTotalFrames;          // # frames in first movi list
	DWORD_T  dwInitialFrames;
	DWORD_T  dwStreams;
	DWORD_T  dwSuggestedBufferSize;
	DWORD_T  dwWidth;
	DWORD_T  dwHeight;
	DWORD_T  dwReserved[4];
} AVIMAINHEADER;

#define ckidODML          FCC('odml')
#define ckidAVIEXTHEADER  FCC('dmlh')
typedef struct _aviextheader {
	FOURCC  fcc;                    // 'dmlh'
	DWORD_T   cb;                     // size of this structure -8
	DWORD_T   dwGrandFrames;          // total number of frames in the file
	DWORD_T   dwFuture[61];           // to be defined later
} AVIEXTHEADER;

//
// structure of an AVI stream header riff chunk
//
#define ckidSTREAMLIST   FCC('strl')

#ifndef ckidSTREAMHEADER
#define ckidSTREAMHEADER FCC('strh')
#endif
typedef struct _avistreamheader {
	FOURCC fcc;          // 'strh'
	const DWORD_T  cb = 0x38;           // size of this structure - 8

	FOURCC fccType;      // stream type codes

#ifndef streamtypeVIDEO
#define streamtypeVIDEO FCC('vids')
#define streamtypeAUDIO FCC('auds')
#define streamtypeMIDI  FCC('mids')
#define streamtypeTEXT  FCC('txts')
#endif

	FOURCC fccHandler;
	DWORD_T  dwFlags;
#define AVISF_DISABLED          0x00000001
#define AVISF_VIDEO_PALCHANGES  0x00010000

	WORD   wPriority;
	WORD   wLanguage;
	DWORD_T  dwInitialFrames;
	DWORD_T  dwScale;
	DWORD_T  dwRate;       // dwRate/dwScale is stream tick rate in ticks/sec
	DWORD_T  dwStart;
	DWORD_T  dwLength;
	DWORD_T  dwSuggestedBufferSize;
	DWORD_T  dwQuality;
	DWORD_T  dwSampleSize;
	struct {
		WORD left;
		WORD top;
		WORD right;
		WORD bottom;
	}   rcFrame;
} AVISTREAMHEADER;


//
// structure of an AVI stream format chunk
//
#ifndef ckidSTREAMFORMAT
#define ckidSTREAMFORMAT FCC('strf')
#endif
//
// avi stream formats are different for each stream type
//
// BITMAPINFOHEADER for video streams
// WAVEFORMATEX or PCMWAVEFORMAT for audio streams
// nothing for text streams
// nothing for midi streams
typedef struct {
	const DWORD_T biSize = 0x28;
	LONG_T  biWidth;
	LONG_T  biHeight;
	WORD  biPlanes;
	WORD  biBitCount;
	DWORD_T biCompression;
	DWORD_T biSizeImage;
	LONG_T  biXPelsPerMeter;
	LONG_T  biYPelsPerMeter;
	DWORD_T biClrUsed;
	DWORD_T biClrImportant;
} AVISTREAMFORMAT;

#pragma warning(disable:4200)
//
// structure of old style AVI index
//
#define ckidAVIOLDINDEX FCC('idx1')
typedef struct _avioldindex {
	FOURCC  fcc;        // 'idx1'
	DWORD_T   cb;         // size of this structure -8
	struct _avioldindex_entry {
		DWORD_T   dwChunkId;
		DWORD_T   dwFlags;

#ifndef AVIIF_LIST
#define AVIIF_LIST       0x00000001
#define AVIIF_KEYFRAME   0x00000010
#endif

#define AVIIF_NO_TIME    0x00000100
#define AVIIF_COMPRESSOR 0x0FFF0000  // unused?
		DWORD_T   dwOffset;    // offset of riff chunk header for the data
		DWORD_T   dwSize;      // size of the data (excluding riff header size)
	} aIndex[];          // size of this array
} AVIOLDINDEX;


//
// ============ structures for timecode in an AVI file =================
//

#ifndef TIMECODE_DEFINED
#define TIMECODE_DEFINED

// defined
// timecode time structure
//
typedef union _timecode {
	struct {
		WORD   wFrameRate;
		WORD   wFrameFract;
		LONG_T   cFrames;
	};
	DWORDLONG  qw;
} TIMECODE;

#endif // TIMECODE_DEFINED

#define TIMECODE_RATE_30DROP 0   // this MUST be zero

// struct for all the SMPTE timecode info
//
typedef struct _timecodedata {
	TIMECODE time;
	DWORD_T    dwSMPTEflags;
	DWORD_T    dwUser;
} TIMECODEDATA;

// dwSMPTEflags masks/values
//
#define TIMECODE_SMPTE_BINARY_GROUP 0x07
#define TIMECODE_SMPTE_COLOR_FRAME  0x08

//
// ============ structures for new style AVI indexes =================
//

// index type codes
//
#define AVI_INDEX_OF_INDEXES       0x00
#define AVI_INDEX_OF_CHUNKS        0x01
#define AVI_INDEX_OF_TIMED_CHUNKS  0x02
#define AVI_INDEX_OF_SUB_2FIELD    0x03
#define AVI_INDEX_IS_DATA          0x80

// index subtype codes
//
#define AVI_INDEX_SUB_DEFAULT     0x00

// INDEX_OF_CHUNKS subtype codes
//
#define AVI_INDEX_SUB_2FIELD      0x01

// meta structure of all avi indexes
//
typedef struct _avimetaindex {
	FOURCC fcc;
	UINT   cb;
	WORD   wLongsPerEntry;
	BYTE   bIndexSubType;
	BYTE   bIndexType;
	DWORD_T  nEntriesInUse;
	DWORD_T  dwChunkId;
	DWORD_T  dwReserved[3];
	DWORD_T  adwIndex[];
} AVIMETAINDEX;

#define STDINDEXSIZE 0x4000
#define NUMINDEX(wLongsPerEntry) ((STDINDEXSIZE-32)/4/(wLongsPerEntry))
#define NUMINDEXFILL(wLongsPerEntry) ((STDINDEXSIZE/4) - NUMINDEX(wLongsPerEntry))

// structure of a super index (INDEX_OF_INDEXES)
//
#define ckidAVISUPERINDEX FCC('indx')
typedef struct _avisuperindex {
	FOURCC   fcc;               // 'indx'
	UINT     cb;                // size of this structure
	WORD     wLongsPerEntry;    // ==4
	BYTE     bIndexSubType;     // ==0 (frame index) or AVI_INDEX_SUB_2FIELD 
	BYTE     bIndexType;        // ==AVI_INDEX_OF_INDEXES
	DWORD_T    nEntriesInUse;     // offset of next unused entry in aIndex
	DWORD_T    dwChunkId;         // chunk ID of chunks being indexed, (i.e. RGB8)
	DWORD_T    dwReserved[3];     // must be 0
	struct _avisuperindex_entry {
		DWORDLONG qwOffset;    // 64 bit offset to sub index chunk
		DWORD_T    dwSize;       // 32 bit size of sub index chunk
		DWORD_T    dwDuration;   // time span of subindex chunk (in stream ticks)
	} aIndex[NUMINDEX(4)];
} AVISUPERINDEX;
#define Valid_SUPERINDEX(pi) (*(DWORD_T *)(&((pi)->wLongsPerEntry)) == (4 | (AVI_INDEX_OF_INDEXES << 24)))

// struct of a standard index (AVI_INDEX_OF_CHUNKS)
//
typedef struct _avistdindex_entry {
	DWORD_T dwOffset;       // 32 bit offset to data (points to data, not riff header)
	DWORD_T dwSize;         // 31 bit size of data (does not include size of riff header), bit 31 is deltaframe bit
} AVISTDINDEX_ENTRY;
#define AVISTDINDEX_DELTAFRAME ( 0x80000000) // Delta frames have the high bit set
#define AVISTDINDEX_SIZEMASK   (~0x80000000)

typedef struct _avistdindex {
	FOURCC   fcc;               // 'indx' or '##ix'
	UINT     cb;                // size of this structure
	WORD     wLongsPerEntry;    // ==2
	BYTE     bIndexSubType;     // ==0
	BYTE     bIndexType;        // ==AVI_INDEX_OF_CHUNKS
	DWORD_T    nEntriesInUse;     // offset of next unused entry in aIndex
	DWORD_T    dwChunkId;         // chunk ID of chunks being indexed, (i.e. RGB8)
	DWORDLONG qwBaseOffset;     // base offset that all index intries are relative to
	DWORD_T    dwReserved_3;      // must be 0
	AVISTDINDEX_ENTRY aIndex[NUMINDEX(2)];
} AVISTDINDEX;

// struct of a time variant standard index (AVI_INDEX_OF_TIMED_CHUNKS)
//
typedef struct _avitimedindex_entry {
	DWORD_T dwOffset;       // 32 bit offset to data (points to data, not riff header)
	DWORD_T dwSize;         // 31 bit size of data (does not include size of riff header) (high bit is deltaframe bit)
	DWORD_T dwDuration;     // how much time the chunk should be played (in stream ticks)
} AVITIMEDINDEX_ENTRY;

typedef struct _avitimedindex {
	FOURCC   fcc;               // 'indx' or '##ix'
	UINT     cb;                // size of this structure
	WORD     wLongsPerEntry;    // ==3
	BYTE     bIndexSubType;     // ==0
	BYTE     bIndexType;        // ==AVI_INDEX_OF_TIMED_CHUNKS
	DWORD_T    nEntriesInUse;     // offset of next unused entry in aIndex
	DWORD_T    dwChunkId;         // chunk ID of chunks being indexed, (i.e. RGB8)
	DWORDLONG qwBaseOffset;     // base offset that all index intries are relative to
	DWORD_T    dwReserved_3;      // must be 0
	AVITIMEDINDEX_ENTRY aIndex[NUMINDEX(3)];
	DWORD_T adwTrailingFill[NUMINDEXFILL(3)]; // to align struct to correct size
} AVITIMEDINDEX;

// structure of a timecode stream
//
typedef struct _avitimecodeindex {
	FOURCC   fcc;               // 'indx' or '##ix'
	UINT     cb;                // size of this structure
	WORD     wLongsPerEntry;    // ==4
	BYTE     bIndexSubType;     // ==0
	BYTE     bIndexType;        // ==AVI_INDEX_IS_DATA
	DWORD_T    nEntriesInUse;     // offset of next unused entry in aIndex
	DWORD_T    dwChunkId;         // 'time'
	DWORD_T    dwReserved[3];     // must be 0
	TIMECODEDATA aIndex[NUMINDEX(sizeof(TIMECODEDATA) / sizeof(LONG_T))];
} AVITIMECODEINDEX;

// structure of a timecode discontinuity list (when wLongsPerEntry == 7)
//
typedef struct _avitcdlindex_entry {
	DWORD_T    dwTick;           // stream tick time that maps to this timecode value
	TIMECODE time;
	DWORD_T    dwSMPTEflags;
	DWORD_T    dwUser;
	TCHAR    szReelId[12];
} AVITCDLINDEX_ENTRY;

typedef struct _avitcdlindex {
	FOURCC   fcc;               // 'indx' or '##ix'
	UINT     cb;                // size of this structure
	WORD     wLongsPerEntry;    // ==7 (must be 4 or more all 'tcdl' indexes
	BYTE     bIndexSubType;     // ==0
	BYTE     bIndexType;        // ==AVI_INDEX_IS_DATA
	DWORD_T    nEntriesInUse;     // offset of next unused entry in aIndex
	DWORD_T    dwChunkId;         // 'tcdl'
	DWORD_T    dwReserved[3];     // must be 0
	AVITCDLINDEX_ENTRY aIndex[NUMINDEX(7)];
	DWORD_T adwTrailingFill[NUMINDEXFILL(7)]; // to align struct to correct size
} AVITCDLINDEX;

typedef struct _avifieldindex_chunk {
	FOURCC   fcc;               // 'ix##'
	DWORD_T    cb;                // size of this structure
	WORD     wLongsPerEntry;    // must be 3 (size of each entry in
								// aIndex array)
	BYTE     bIndexSubType;     // AVI_INDEX_2FIELD
	BYTE     bIndexType;        // AVI_INDEX_OF_CHUNKS
	DWORD_T    nEntriesInUse;     //
	DWORD_T    dwChunkId;         // '##dc' or '##db'
	DWORDLONG qwBaseOffset;     // offsets in aIndex array are relative to this
	DWORD_T    dwReserved3;       // must be 0
	struct _avifieldindex_entry {
		DWORD_T    dwOffset;
		DWORD_T    dwSize;         // size of all fields
								 // (bit 31 set for NON-keyframes)
		DWORD_T    dwOffsetField2; // offset to second field
	} aIndex[];
} AVIFIELDINDEX, *PAVIFIELDINDEX;


#include <poppack.h>

#endif
