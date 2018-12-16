#pragma once

#include <iostream>
#include <fstream>
#include <vector>

class hdr {
public:
	static const char* szSignature, *szFormat;
	static unsigned char * read_hdr(const char* filename, unsigned int* w, unsigned int* h);
	static void write_hdr(const char* filename, unsigned int w, unsigned int h, unsigned char* data);
	static void to_float(unsigned char imagergbe[], int w, int h, float* res);
	static void to_rgbe(float* rgb, int w, int h, unsigned char* res);
};