#include "raytracer.h"
#include "oclcode.h"
#include "hdr.h"
#include "md/Particles.h"
#include "md/ParMenu.h"
#include "utils/dialog.h"
#include "vis/pargraphics.h"
#include "ui/ui_ext.h"
#ifdef PLATFORM_OSX
#include <OpenCL/cl_gl_ext.h>
#include <OpenGL/CGLDevice.h>
#include <OpenGL/CGLCurrent.h>
#include <OpenCL/cl_gl.h>
#else
#include <CL/cl_gl.h>
#ifdef PLATFORM_LNX
#include <GL/glx.h>
#endif
#endif

RayTracer::info_st RayTracer::info = {};

bool RayTracer::live = false, RayTracer::expDirty = false;
BVH::Node* RayTracer::bvh;
uint RayTracer::bvhSz;
Mat4x4 RayTracer::IP;
string RayTracer::bgName;
float RayTracer::prvRes = 1;
uint RayTracer::resw, RayTracer::resh;
uint RayTracer::prvSmp = 50;

GLuint RayTracer::resTex = 0;

cl_context RayTracer::_ctx;
cl_command_queue RayTracer::_que;
cl_kernel RayTracer::_kernel;

cl_mem _bufr, _bg, _bls, _cns, _cls, _rds;

bool RayTracer::Init(){
	cl_platform_id platform;
	clGetPlatformIDs(1, &platform, 0);
	cl_device_id device;
	clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, 0);

#ifdef PLATFORM_OSX
	CGLContextObj kCGLContext = CGLGetCurrentContext();
	CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
	cl_context_properties props[] =
	{
		CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup,
		0
	};
	_ctx = clCreateContext(props, 0, 0, 0, 0, 0);
#else
#ifdef PLATFORM_WIN
	cl_context_properties props[] =
	{
		CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
		CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
		0
	};
#else
	cl_context_properties props[] =
	{
		CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
		CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
		0
	};
#endif
	_ctx = clCreateContext(props, 1, &device, 0, 0, 0);
#endif

	_que = clCreateCommandQueue(_ctx, device, 0, 0);
	
	cl_program prog = clCreateProgramWithSource(_ctx, 1, &ocl::code, 0, 0);
	cl_int err = clBuildProgram(prog, 1, &device, 0, 0, 0);

	size_t lsz;
	clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, 0, 0, &lsz);
	char* log = new char[lsz];
	clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, lsz, log, 0);
	std::cout << log << std::endl;
	delete[](log);
	if (err != CL_SUCCESS) {
		Debug::Error("RayTracer", "program error: " + std::to_string(err));
		return false;
	}
	_kernel = clCreateKernel(prog, "_main_", 0);
	
	CheckRes();

	info.str = 2;
	info.mat.specular = 0.3f;
	info.mat.rough = 0.2f;
	info.mat.gloss = 0.95f;
	info.mat.ior = 1;
	SetBg(IO::path + "/res/refl.hdr");

	live = true;
	return true;
}

void RayTracer::SetScene() {
	if (!live) return;

	if (!resTex) {
		std::cout << "Generating Bounds..." << std::endl;
		auto rads = new byte[Particles::particleSz]{};
		ParGraphics::FillRad(rads);
		BVH::Ball* objs = new BVH::Ball[Particles::particleSz];
		for (uint a = 0; a < Particles::particleSz; a++) {
			objs[a].orig = Particles::particles_Pos[a];
			objs[a].rad = rads[a] * 0.1f / 127;
		}
		std::cout << "Generating BVH..." << std::endl;
		BVH::Calc(objs, Particles::particleSz, bvh, bvhSz, BBox(0,0,0,0,0,0));
		delete[](objs);
		std::cout << "Settings kernel args..." << std::endl;
		int vl = 20;
		cl_int err = CL_SUCCESS;
		//_bls = clCreateBuffer(_ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, vl * sizeof(Vec3), Particles::particles_Pos, &err);
		_bls = clCreateFromGLBuffer(_ctx, CL_MEM_READ_ONLY, Particles::posBuffer, &err);
		assert(err == CL_SUCCESS);
		clEnqueueAcquireGLObjects(_que, 1, &_bls, 0, 0, 0);
		clSetKernelArg(_kernel, 3, sizeof(_bls), (void*)&_bls);
		clSetKernelArg(_kernel, 4, sizeof(cl_int), &vl);
		
		vl = min(17U, Particles::connSz);
		if (!!Particles::connSz) {
			//_cns = clCreateBuffer(_ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, vl * 2 * sizeof(cl_int), Particles::particles_Conn, &err);
			_cns = clCreateFromGLBuffer(_ctx, CL_MEM_READ_ONLY, Particles::connBuffer, &err);
			clEnqueueAcquireGLObjects(_que, 1, &_cns, 0, 0, 0);
			assert(err == CL_SUCCESS);
		}
		clSetKernelArg(_kernel, 5, sizeof(_cns), (void*)&_cns);
		clSetKernelArg(_kernel, 6, sizeof(cl_int), &vl);
		_cls = clCreateFromGLBuffer(_ctx, CL_MEM_READ_ONLY, Particles::colIdBuffer, &err);
		clEnqueueAcquireGLObjects(_que, 1, &_cls, 0, 0, 0);
		assert(err == CL_SUCCESS);
		clSetKernelArg(_kernel, 7, sizeof(_cls), &_cls);
		_rds = clCreateBuffer(_ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, Particles::particleSz * sizeof(cl_uchar), rads, &err);
		assert(err == CL_SUCCESS);
		delete[](rads);
		clSetKernelArg(_kernel, 8, sizeof(_rds), &_rds);
		clFinish(_que);

		glGenTextures(1, &resTex);
	}
}

uint _cntt = 0;
std::vector<float> _texx;

void RayTracer::CheckRes() {
	uint w = (uint)(Display::width * prvRes);
	uint h = (uint)(Display::height * prvRes);
	if (w != resw || h != resh) {
		info.w = resw = w;
		info.h = resh = h;
		clReleaseMemObject(_bufr);
		_bufr = clCreateBuffer(_ctx, CL_MEM_WRITE_ONLY, resw * resh * 4 * sizeof(cl_float), 0, 0);
		clSetKernelArg(_kernel, 0, sizeof(_bufr), (void*)&_bufr);
		_cntt = 0;
		_texx.resize(resw * resh * 4);
	}
}

void RayTracer::SetTex(uint w, uint h){
	
}

void RayTracer::SetBg(string fl) {
	byte* d = hdr::read_hdr(fl.c_str(), (uint*)&info.bg_w, (uint*)&info.bg_h);
	if (!d) return;
	std::vector<float> dv;
	hdr::to_float(d, info.bg_w, info.bg_h, &dv);
	delete[](d);

#ifdef PLATFORM_WIN
	std::replace(fl.begin(), fl.end(), '\\', '/');
#endif
	bgName = fl.substr(fl.find_last_of('/') + 1);

	if (_bg) clReleaseMemObject(_bg);
	_bg = clCreateBuffer(_ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, info.bg_w * info.bg_h * 3 * sizeof(cl_float), &dv[0], 0);
	clSetKernelArg(_kernel, 2, sizeof(_bg), (void*)&_bg);
}

void RayTracer::Render() {
	if (!live || !resTex) return;

	if (expDirty) {
		expDirty = false;
		_cntt = 0;
	}

	CheckRes();

	if (_cntt >= prvSmp) return;
	else if (!_cntt) IP = glm::inverse(MVP::projection()*MVP::modelview());

	memcpy(info.IP, glm::value_ptr(IP), 16 * sizeof(float));
	clSetKernelArg(_kernel, 1, sizeof(info), &info);
	
	size_t ws = info.w*info.h;
	info.rand = (cl_int)(rand() | (rand() << 16));
	clEnqueueNDRangeKernel(_que, _kernel, 1, 0, &ws, 0, 0, 0, 0);
	clFinish(_que);

	cl_float* bufrp = (cl_float*)clEnqueueMapBuffer(_que, _bufr, CL_TRUE, CL_MAP_READ, 0, ws * 4 * sizeof(cl_float), 0, 0, 0, 0);
	if (!_cntt) {
		memcpy(&_texx[0], bufrp, resw * resh * 4 * sizeof(float));
		glBindTexture(GL_TEXTURE_2D, resTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, resw, resh, 0, GL_RGBA, GL_FLOAT, &_texx[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else {
		for (uint a = 0; a < ws * 4; a++) {
			_texx[a] = (_texx[a] * _cntt + bufrp[a]) / (_cntt+1);
		}
		glBindTexture(GL_TEXTURE_2D, resTex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, resw, resh, GL_RGBA, GL_FLOAT, &_texx[0]);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	_cntt++;
	

	clEnqueueUnmapMemObject(_que, _bufr, bufrp, 0, 0, 0);
}

void RayTracer::Clear() {
	clEnqueueReleaseGLObjects(_que, 1, &_bls, 0, 0, 0);
	clEnqueueReleaseGLObjects(_que, 1, &_cns, 0, 0, 0);
	clEnqueueReleaseGLObjects(_que, 1, &_cls, 0, 0, 0);
	clFinish(_que);
	clReleaseMemObject(_bls);
	clReleaseMemObject(_cns);
	clReleaseMemObject(_cls);
	clReleaseMemObject(_rds);
	glDeleteTextures(1, &resTex);
	resTex = 0;
	_cntt = 0;
	Scene::dirty = true;
}

void RayTracer::DrawMenu() {
#define SV(vl, b) auto b = vl; vl

	auto& expandPos = ParMenu::expandPos;
	auto& mt = info.mat;

	if (Engine::Button(expandPos - 148, 20, 146, 16, resTex ? Vec4(0.4f, 0.2f, 0.2f, 1) : Vec4(0.2f, 0.4f, 0.2f, 1), resTex ? "Disable (Shift-X)" : "Enable (Shift-X)", 12, white(), true) == MOUSE_RELEASE) {
		if (resTex) Clear();
		else SetScene();
	}

	float off = 17 * 3 + 1;
	if (resTex) {
		UI::Label(expandPos - 148, 17 * 2, 12, "Samples: " + std::to_string(_cntt), white(0.5f));
		off += 17;
	}

	UI::Label(expandPos - 148, off, 12, "Preview", white());
	Engine::DrawQuad(expandPos - 149, off + 17, 148, 17 * 2 + 2, white(0.9f, 0.1f));
	off++;
	prvRes = UI2::Slider(expandPos - 147, off + 17, 147, "Quality", 0.1f, 1, prvRes, std::to_string(int(prvRes * 100)) + "%");
	prvSmp = TryParse(UI2::EditText(expandPos - 147, off + 17 * 2, 147, "Samples", std::to_string(prvSmp)), 50U);

	off += 17 * 3 * 2;

	UI::Label(expandPos - 148, off, 12, "Background", white());
	Engine::DrawQuad(expandPos - 149, off + 17, 148, 17 * 2 + 2, white(0.9f, 0.1f));
	off++;
	UI2::File(expandPos - 147, off + 17, 147, "File", bgName, [](std::vector<string> res) {
		SetBg(res[0]);
		_cntt = 0;
	});
	SV(info.str, str) = UI2::Slider(expandPos - 147, off + 17 * 2, 147, "Strength", 0, 5, info.str);

	off += 17 * 3 + 2;

	UI::Label(expandPos - 148, off, 12, "Material", white());
	Engine::DrawQuad(expandPos - 149, off + 17, 148, 17 * 2 + 2, white(0.9f, 0.1f));
	off++;
	SV(mt.specular, spc) = UI2::Slider(expandPos - 147, off + 17, 147, "Specular", 0, 1, mt.specular);
	SV(mt.gloss, gls) = UI2::Slider(expandPos - 147, off + 17 * 2, 147, "Gloss", 0, 1, mt.gloss);

	if ((info.str != str) || (mt.specular != spc) || mt.gloss != gls) {
		mt.rough = (1-mt.gloss)*0.5f;
		_cntt = 0;
	}
}