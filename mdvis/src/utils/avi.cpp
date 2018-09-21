#include "avi.h"
#include "aviriff2.h"

void AVI::Encode(const std::string& path, byte** frames, uint frameC, int w, int h, byte chn) {
	std::ofstream strm(path, std::ios::binary);
	strm << "RIFFXXXXAVILISTXXXXhdrl";
	std::streampos rfps = 4, l1ps = 15;
	auto avih = AVIMAINHEADER();
	avih.dwMicroSecPerFrame = 41667;
	avih.dwMaxBytesPerSec = w * h * chn * 24;
	avih.dwPaddingGranularity = 2;
	avih.dwTotalFrames = frameC;
	avih.dwStreams = 1;
	avih.dwSuggestedBufferSize = w * h * chn;
	avih.dwWidth = w;
	avih.dwHeight = h;

	_StreamWrite(&avih, &strm, sizeof(avih));
	strm << "LIST";
	auto l2ps = strm.tellp();
	strm << "XXXXstrl";
	auto strh = AVISTREAMHEADER();
	strh.fccType = streamtypeVIDEO;
	strh.wPriority = 1;
	strh.dwScale = 1;
	strh.dwRate = 24;
	strh.dwLength = frameC;
	strh.dwQuality = -1;
	strh.dwSampleSize = w*h*chn;
	strh.rcFrame.right = w;
	strh.rcFrame.top = h;

	_StreamWrite(&strh, &strm, sizeof(strh));
	auto strf = AVISTREAMFORMAT();
	strf.biWidth = w;
	strf.biHeight = h;
	strf.biPlanes = 1;
	strf.biBitCount = chn * 8;
	strf.biCompression = 0;//BI_RGB: BI_BITFIELDS;
	strf.biXPelsPerMeter = w;
	strf.biYPelsPerMeter = h;
	
	_StreamWrite(&strf, &strm, sizeof(strf));
	auto pl = strm.tellp();
	strm.seekp(l1ps);
	l1ps = pl - l1ps - 4;
	strm.write((char*)&l1ps, 4);
	strm.seekp(l2ps);
	l2ps = pl - l2ps - 4;
	strm.write((char*)&l2ps, 4);
	strm.seekp(pl);
	
	strm << "LIST";
	auto l3ps = strm.tellp();
	strm << "XXXXmovi";

}