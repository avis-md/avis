#include "avi.h"
extern "C" {
#define INT32 J_INT32
#include "jpeglib.h"
#include "jerror.h"
#undef INT32
}

AVI::AVI(const std::string& path, uint w, uint h, uint fps) : _w(w), _h(h) {
	gwavi = gwavi_open(path.c_str(), w, h, "MJPG", fps, nullptr);
}

void AVI::AddFrame(GLuint tex) {
	std::vector<byte> buf(_w*_h*3);
	glBindTexture(GL_TEXTURE_2D, tex);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, &buf[0]);
	glBindTexture(GL_TEXTURE_2D, 0);
	auto path = IO::path + "/tmp/tmpmovieframe.jpg";

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];
	int row_stride;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	FILE* fp;
	fopen_s(&fp, path.c_str(), "wb");
	jpeg_stdio_dest(&cinfo, fp);
	cinfo.image_width = _w;
	cinfo.image_height = _h;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, 75, TRUE);
	jpeg_start_compress(&cinfo, TRUE);
	row_stride = _w * 3;
	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = &buf[(_h-1-cinfo.next_scanline) * row_stride];
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	fclose(fp);
	jpeg_destroy_compress(&cinfo);

	auto data = IO::GetBytes(path);

	gwavi_add_frame(gwavi, &data[0], data.size());
	//remove(path.c_str());
}

void AVI::End() {
	gwavi_close(gwavi);
}
