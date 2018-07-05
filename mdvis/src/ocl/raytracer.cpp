#include "raytracer.h"

RayTracer::info_st RayTracer::info = {};

bool RayTracer::live = false;

GLuint RayTracer::resTex = 0;

cl_context RayTracer::_ctx;
cl_command_queue RayTracer::_que;
cl_kernel RayTracer::_kernel;

cl_mem _bufr;

bool RayTracer::Init(){
	cl_platform_id platform;
	clGetPlatformIDs(1, &platform, 0);
	cl_device_id device;
	clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, 0);
	_ctx = clCreateContext(0, 1, &device, 0, 0, 0);
	_que = clCreateCommandQueue(_ctx, device, 0, 0);
	
	auto fl = IO::GetText(IO::path + "/ocl/test.txt");
	const char* flc[] = { fl.c_str() };
	cl_program prog = clCreateProgramWithSource(_ctx, 1, flc, 0, 0);
	cl_int err = clBuildProgram(prog, 1, &device, 0, 0, 0);
	if (err != CL_SUCCESS) {
		size_t lsz;
		clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, 0, 0, &lsz);
		char* log = new char[lsz];
		clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, lsz, log, 0);
		std::cout << log << std::endl;
		delete[](log);
		Debug::Error("RayTracer", "program error: " + std::to_string(err));
		return false;
	}
	_kernel = clCreateKernel(prog, "_main_", 0);
	
	_bufr = clCreateBuffer(_ctx, CL_MEM_WRITE_ONLY, 200 * 150 * 4 * sizeof(cl_float), 0, 0);
	clSetKernelArg(_kernel, 0, sizeof(_bufr), (void*)&_bufr);

	live = true;
}

void RayTracer::SetTex(uint w, uint h){
	
}

uint _cntt = 0;
float _texx[200*150*4];

void RayTracer::Render(){
	if (!live) return;

	if (!resTex) glGenTextures(1, &resTex);

	info.w = 200;
	info.h = 150;
	info.rand = (cl_int)(Random::Value() * 10000);
	memcpy(info.IP, glm::value_ptr(glm::inverse(MVP::projection())), 16 * sizeof(float));
	clSetKernelArg(_kernel, 1, sizeof(info), &info);
	
	size_t ws = info.w*info.h;
	clEnqueueNDRangeKernel(_que, _kernel, 1, 0, &ws, 0, 0, 0, 0);
	clFinish(_que);

	cl_float* bufrp = (cl_float*)clEnqueueMapBuffer(_que, _bufr, CL_TRUE, CL_MAP_READ, 0, ws * 4 * sizeof(cl_float), 0, 0, 0, 0);
	if (!_cntt) {
		memcpy(_texx, bufrp, 200 * 150 * 4 * sizeof(float));
	}
	else {
		for (uint a = 0; a < 200 * 150 * 4; a++) {
			_texx[a] = (_texx[a] * _cntt + bufrp[a]) / (_cntt+1);
		}
	}
	_cntt++;
	
	glBindTexture(GL_TEXTURE_2D, resTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, info.w, info.h, 0, GL_RGBA, GL_FLOAT, _texx);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	clEnqueueUnmapMemObject(_que, _bufr, bufrp, 0, 0, 0);

	//
	//live = false;
}

void RayTracer::Draw() {
	Engine::DrawQuad(200, 100, 400, 300, RayTracer::resTex);
	UI::Label(202, 102, 12, "Samples: " + std::to_string(_cntt));
}