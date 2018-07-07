#include "raytracer.h"
#include "hdr.h"
#include "md/Particles.h"
#include <CL/cl_gl.h>
#ifdef PLATFORM_LNX
#include <GL/glx.h>
#endif

#define RESW 400
#define RESH 300

RayTracer::info_st RayTracer::info = {};

bool RayTracer::live = false;

GLuint RayTracer::resTex = 0;

cl_context RayTracer::_ctx;
cl_command_queue RayTracer::_que;
cl_kernel RayTracer::_kernel;

cl_mem _bufr, _bg, _bls, _cns;

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
	
	auto fl = IO::GetText(IO::path + "/ocl/test.txt");
	const char* flc[] = { fl.c_str() };
	cl_program prog = clCreateProgramWithSource(_ctx, 1, flc, 0, 0);
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
	
	_bufr = clCreateBuffer(_ctx, CL_MEM_WRITE_ONLY, RESW * RESH * 4 * sizeof(cl_float), 0, 0);
	clSetKernelArg(_kernel, 0, sizeof(_bufr), (void*)&_bufr);

	byte* d = hdr::read_hdr((IO::path + "/res/bigsight.hdr").c_str(), (uint*)&info.bg_w, (uint*)&info.bg_h);
	auto dv = hdr::to_float(d, info.bg_w, info.bg_h);
	delete[](d);

	_bg = clCreateBuffer(_ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, info.bg_w * info.bg_h * 3 * sizeof(cl_float), &dv[0], 0);
	clSetKernelArg(_kernel, 2, sizeof(_bg), (void*)&_bg);

	live = true;
	return true;
}

void RayTracer::SetScene() {
	info.str = 2;
	info.mat.specular = 0.2f;
	info.mat.rough = 0.15f;
	info.mat.gloss = 0.7f;
	//memcpy(info.IP, glm::value_ptr(glm::inverse(MVP::projection()*MVP::modelview())), 16 * sizeof(float));

	if (!resTex) {
		//clSetKernelArg(_kernel, 1, sizeof(info), &info);

		int vl = 200;
		cl_int err = CL_SUCCESS;
		//_bls = clCreateBuffer(_ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, vl * sizeof(Vec3), Particles::particles_Pos, &err);
		_bls = clCreateFromGLBuffer(_ctx, CL_MEM_READ_ONLY, Particles::posBuffer, &err);
		assert(err == CL_SUCCESS);
		clEnqueueAcquireGLObjects(_que, 1, &_bls, 0, 0, 0);
		clSetKernelArg(_kernel, 3, sizeof(_bls), (void*)&_bls);
		clSetKernelArg(_kernel, 4, sizeof(cl_int), &vl);
		
		vl = 50;
		if (!!Particles::connSz) {
			//_cns = clCreateBuffer(_ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, vl * 2 * sizeof(cl_int), Particles::particles_Conn, &err);
			_cns = clCreateFromGLBuffer(_ctx, CL_MEM_READ_ONLY, Particles::connBuffer, &err);
			clEnqueueAcquireGLObjects(_que, 1, &_cns, 0, 0, 0);
			assert(err == CL_SUCCESS);
		}
		clSetKernelArg(_kernel, 5, sizeof(_cns), (void*)&_cns);
		clSetKernelArg(_kernel, 6, sizeof(cl_int), &vl);
		clFinish(_que);
	}
}

void RayTracer::SetTex(uint w, uint h){
	
}

uint _cntt = 0;
float _texx[RESW*RESH*4];

void RayTracer::Render(){
	if (!live) return;

	if (!resTex) glGenTextures(1, &resTex);

	info.w = RESW;
	info.h = RESH;
	memcpy(info.IP, glm::value_ptr(glm::inverse(MVP::projection()*MVP::modelview())), 16 * sizeof(float));
	clSetKernelArg(_kernel, 1, sizeof(info), &info);
	
	size_t ws = info.w*info.h;
	info.rand = (cl_int)(rand() | (rand() << 16));
	clEnqueueNDRangeKernel(_que, _kernel, 1, 0, &ws, 0, 0, 0, 0);
	clFinish(_que);

	cl_float* bufrp = (cl_float*)clEnqueueMapBuffer(_que, _bufr, CL_TRUE, CL_MAP_READ, 0, ws * 4 * sizeof(cl_float), 0, 0, 0, 0);
	if (!_cntt) {
		memcpy(_texx, bufrp, RESW * RESH * 4 * sizeof(float));
		glBindTexture(GL_TEXTURE_2D, resTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, RESW, RESH, 0, GL_RGBA, GL_FLOAT, _texx);
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
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, RESW, RESH, GL_RGBA, GL_FLOAT, _texx);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	_cntt++;
	

	clEnqueueUnmapMemObject(_que, _bufr, bufrp, 0, 0, 0);

	//
	//live = false;
}

void RayTracer::Draw() {
	Engine::DrawQuad(200, 100, 400, 300, RayTracer::resTex);
	UI::Label(202, 102, 12, "Samples: " + std::to_string(_cntt), white());
	auto s = Engine::DrawSliderFill(210, 410, 200, 10, 0, 5, info.str, blue(), white());
	if (s != info.str) {
		info.str = s;
		_cntt = 0;
	}

	auto& mt = info.mat;
	UI::Label(210, 440, 12, "Specular");
	auto o = Engine::DrawSliderFill(350, 440, 200, 10, 0, 1, mt.specular, blue(), white());
	if (o != mt.specular) {
		mt.specular = o;
		_cntt = 0;
	}
	UI::Label(210, 460, 12, "Roughness (Diffuse)");
	o = Engine::DrawSliderFill(350, 460, 200, 10, 0, 1, mt.rough, blue(), white());
	if (o != mt.rough) {
		mt.rough = o;
		_cntt = 0;
	}
	UI::Label(210, 480, 12, "Gloss (Specular)");
	o = Engine::DrawSliderFill(350, 480, 200, 10, 0, 1, mt.gloss, blue(), white());
	if (o != mt.gloss) {
		mt.gloss = o;
		_cntt = 0;
	}
	UI::Label(210, 500, 12, "Metallic");
	o = Engine::DrawSliderFill(350, 500, 200, 10, 0, 1, mt.metallic, blue(), white());
	if (o != mt.metallic) {
		mt.metallic = o;
		_cntt = 0;
	}
}