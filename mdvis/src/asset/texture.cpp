#include "Engine.h"

extern "C" {
#include "jpeglib.h"
#include "jerror.h"
}
#include "lodepng.h"

std::unordered_map<GLuint, int> Texture::_refcnt;

Texture::Texture(const std::string& path, bool mipmap, TEX_FILTERING filter, byte aniso, TEX_WRAPING warp) :
	Texture(path, mipmap, filter, aniso, (warp == TEX_WRAP_CLAMP) ? GL_CLAMP_TO_EDGE : GL_REPEAT, (warp == TEX_WRAP_CLAMP) ? GL_CLAMP_TO_EDGE : GL_REPEAT) {}

Texture::Texture(const std::string& path, bool mipmap, TEX_FILTERING filter, byte aniso, GLenum wrapx, GLenum wrapy) {
	std::string sss = path.substr(path.find_last_of('.') + 1, std::string::npos);
	byte *data;
	std::vector<byte> dataV;
	byte chn;
	//std::cout << "opening image at " << path << std::endl;
	GLenum rgb = GL_RGB, rgba = GL_RGBA;
	if (sss == "bmp") {
		if (!LoadBMP(path, width, height, chn, &data)) {
			Debug::Warning("Texture", "load bmp failed! " + path);
			return;
		}
		rgb = GL_BGR;
		rgba = GL_BGRA;
	}
	else if (sss == "jpg") {
		if (!LoadJPEG(path, width, height, chn, &data)) {
			Debug::Warning("Texture", "load jpg failed! " + path);
			return;
		}
	}
	else if (sss == "png") {
		if (!LoadPNG(path, width, height, chn, dataV)) {
			Debug::Warning("Texture", "load png failed! " + path);
			return;
		}
		data = &dataV[0];
	}
	else {
		Debug::Warning("Texture", "invalid extension! " + sss);
		return;
	}

	glGenTextures(1, &pointer);
	glBindTexture(GL_TEXTURE_2D, pointer);
	if (chn == 3) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, rgb, GL_UNSIGNED_BYTE, data);
	else glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, rgba, GL_UNSIGNED_BYTE, data);
	if (mipmap) glGenerateMipmap(GL_TEXTURE_2D);
	SetTexParams<>(aniso, wrapx, wrapy,
		(mipmap && (filter == TEX_FILTER_TRILINEAR)) ? GL_LINEAR_MIPMAP_LINEAR : (filter == TEX_FILTER_POINT) ? GL_NEAREST : GL_LINEAR,
		(filter == TEX_FILTER_POINT) ? GL_NEAREST : GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	if (dataV.size() == 0) delete[](data);
	_refcnt[pointer]++;
	loaded = true;
}

Texture::Texture(const byte* data, const uint dataSz, TEX_FILTERING filter, TEX_WRAPING wrap) {
	std::vector<byte> dataV;
	uint err = lodepng::decode(dataV, width, height, data, dataSz);
	if (!!err) {
		Debug::Error("PNG reader", "Read PNG error: " + std::string(lodepng_error_text(err)));
		return;
	}
	InvertPNG(dataV, width, height);

	glGenTextures(1, &pointer);
	glBindTexture(GL_TEXTURE_2D, pointer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &dataV[0]);
	SetTexParams<>(1, (wrap == TEX_WRAP_CLAMP) ? GL_CLAMP_TO_EDGE : GL_REPEAT,
		(wrap == TEX_WRAP_CLAMP) ? GL_CLAMP_TO_EDGE : GL_REPEAT,
		(filter == TEX_FILTER_POINT) ? GL_NEAREST : GL_LINEAR,
		(filter == TEX_FILTER_POINT) ? GL_NEAREST : GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	_refcnt[pointer]++;
	loaded = true;
}

Texture& Texture::operator= (const Texture& t) {
	width = t.width;
	height = t.height;
	pointer = t.pointer;
	loaded = t.loaded;
	if (loaded) _refcnt[pointer]++;
	return *this;
}

Texture::~Texture() {
	auto rc = --_refcnt[pointer];
	if (pointer>0 && rc == 0)
		glDeleteTextures(1, &pointer);
}

byte* Texture::LoadPixels(const std::string& path, byte& chn, uint& w, uint& h) {
	std::string sss = path.substr(path.find_last_of('.'), std::string::npos);
	byte *data;
	std::vector<byte> dataV;
	if (sss == ".bmp") {
		if (!LoadBMP(path, w, h, chn, &data)) {
			Debug::Error("Texture", "load bmp failed! " + path);
			return nullptr;
		}
	}
	else if (sss == ".jpg") {
		if (!LoadJPEG(path, w, h, chn, &data)) {
			Debug::Error("Texture", "load jpg failed! " + path);
			return nullptr;
		}
	}
	else if (sss == ".png") {
		if (!LoadPNG(path, w, h, chn, dataV)) {
			Debug::Error("Texture", "load png failed! " + path);
			return nullptr;
		}
		data = new byte[w*h*chn];
		memcpy(data, &dataV[0], w*h*chn);
	}
	else {
		Debug::Error("Texture", "Image extension invalid! " + path);
		return nullptr;
	}
	return data;
}

byte* Texture::LoadPixels(const byte* data, const uint dataSz, uint& w, uint& h) {
	std::vector<byte> dataV;
	uint err = lodepng::decode(dataV, w, h, data, dataSz);
	if (!!err) {
		Debug::Error("PNG reader", "Read PNG error: " + std::string(lodepng_error_text(err)));
		return nullptr;
	}
	InvertPNG(dataV, w, h);
	byte* res = new byte[w * h * 4];
	memcpy(res, &dataV[0], w*h * 4);
	return res;
}

void Texture::ToPNG(std::vector<byte>& data, uint w, uint h, const std::string& loc) {
	std::vector<byte> res;
	InvertPNG(data, w, h);
	lodepng::encode(res, &data[0], w, h);
	std::ofstream strm(loc, std::ios::binary);
	strm.write((char*)&res[0], res.size());
}

bool Texture::LoadJPEG(std::string fileN, uint &x, uint &y, byte& channels, byte** data)
{
	//unsigned int texture_id;
	unsigned long data_size;     // length
	unsigned char * rowptr[1];
	struct jpeg_decompress_struct info; //for our jpeg info
	struct jpeg_error_mgr err;          //the error handler

	FILE* file;
	fopen_s(&file, fileN.c_str(), "rb");  //open the file
	info.err = jpeg_std_error(&err);
	jpeg_create_decompress(&info);   //fills info structure

	if (!file) {					//if the jpeg file doesn't load
		return false;
	}

	jpeg_stdio_src(&info, file);
	jpeg_read_header(&info, TRUE);   // read jpeg file header

	jpeg_start_decompress(&info);    // decompress the file

	x = info.output_width;
	y = info.output_height;
	channels = info.num_components;

	data_size = x * y * 3;

	*data = (unsigned char *)malloc(data_size);
	while (info.output_scanline < y) // loop
	{
		// Enable jpeg_read_scanlines() to fill our jdata array
		rowptr[0] = (unsigned char *)*data + (3 * x * (y - info.output_scanline - 1));

		jpeg_read_scanlines(&info, rowptr, 1);
	}
	//---------------------------------------------------

	jpeg_finish_decompress(&info);   //finish decompressing
	jpeg_destroy_decompress(&info);
	fclose(file);
	return true;
}

void Texture::InvertPNG(std::vector<byte>& data, uint x, uint y) {
#pragma omp parallel for
	for (int a = 0; a < y / 2; a++) {
		std::vector<byte> tmp(x * 4);
		memcpy(&tmp[0], &data[a*x * 4], x * 4);
		memcpy(&data[a*x * 4], &data[(y - a - 1)*x * 4], x * 4);
		memcpy(&data[(y - a - 1)*x * 4], &tmp[0], x * 4);
	}
}

bool Texture::LoadPNG(std::string fileN, uint &x, uint &y, byte& channels, std::vector<byte>& data) {
	channels = 4;
	uint err = lodepng::decode(data, x, y, fileN.c_str());
	if (err) {
		Debug::Error("PNG reader", "Read PNG error: " + std::string(lodepng_error_text(err)));
		return false;
	}
	InvertPNG(data, x, y);
	return true;
}

bool Texture::LoadBMP(std::string fileN, uint &x, uint &y, byte& channels, byte** data) {

	char header[54]; // Each BMP file begins by a 54-bytes header
	unsigned int dataPos;     // Position in the file where the actual data begins
	unsigned int imageSize;   // = width*height*3
	unsigned short bpi;

	std::ifstream strm(fileN.c_str(), std::ios::in | std::ios::binary);

	if (!strm.is_open()) {
		printf("Image could not be opened\n");
		return false;
	}
	strm.read(header, 54);
	if (strm.bad()) { // If not 54 bytes read : problem
		printf("Not a correct BMP file\n");
		return false;
	}
	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		return false;
	}
	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	x = *(int*)&(header[0x12]);
	y = *(int*)&(header[0x16]);
	bpi = *(short*)&(header[0x1c]);
	if (bpi != 24 && bpi != 32)
		return false;
	else channels = (bpi == 24) ? 3 : 4;
	// Some BMP files are misformatted, guess missing information
	if (imageSize == 0)    imageSize = x * y * channels; // 3 : one byte for each Red, Green and Blue component
	if (dataPos == 0)      dataPos = 54; // The BMP header is done that way
	*data = new unsigned char[imageSize];
	// Read the actual data from the file into the buffer
	strm.read(*(char**)data, imageSize);
	return true;
}