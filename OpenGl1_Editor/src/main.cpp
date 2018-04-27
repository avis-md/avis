#define PLATFORM_WIN
#include "ChokoLait.h"
#include "Gromacs.h"

float camz = 10;
Vec3 center;
float rw = 0, rz = 0;
float repx, repy, repz;

Gromacs* gro;
Material* mat, *connMat, *connMat2;
Shader* shad, *connShad, *connShad2;
Mesh* mesh, *connMesh, *connMesh2;
Background* bg;
Font* font;

bool drawMesh;

Mesh* MakeConnMesh(int res) {
	std::vector<Vec3> verts(res * 2), norms(res * 2);
	std::vector<int> tris(res * 6);
	const float da = 2.0f * PI / res;
	for (int a = 0; a < res; a++) {
		verts[a * 2] = Vec3(cosf(a * da), sinf(a * da), 0);
		verts[a * 2 + 1] = Vec3(cosf(a * da), sinf(a * da), 1);
		tris[a * 6] = a * 2;
		tris[a * 6 + 3] = a * 2;
		tris[a * 6 + 5] = a * 2 + 1;
		if (a < res - 1) {
			tris[a * 6 + 1] = a * 2 + 2;
			tris[a * 6 + 2] = a * 2 + 3;
			tris[a * 6 + 4] = a * 2 + 3;
		}
		else {
			tris[a * 6 + 1] = 0;
			tris[a * 6 + 2] = 1;
			tris[a * 6 + 4] = 1;
		}
	}
	return new Mesh(verts, norms, tris);
}

void updateFunc() {
	if (Input::KeyHold(Key_UpArrow)) camz -= 10 * Time::delta;
	else if (Input::KeyHold(Key_DownArrow)) camz += 10 * Time::delta;
	camz = max(camz, 0.0f);
	if (Input::KeyHold(Key_W)) rw -= 100 * Time::delta;
	else if (Input::KeyHold(Key_S)) rw += 100 * Time::delta;
	//camz = Clamp<float>(camz, 0.5f, 10);
	if (Input::KeyHold(Key_A)) rz -= 100 * Time::delta;
	else if (Input::KeyHold(Key_D)) rz += 100 * Time::delta;
	//camz = Clamp<float>(camz, 0.5f, 10);
	if (Input::KeyHold(Key_J)) center.x -= 1 * Time::delta;
	else if (Input::KeyHold(Key_L)) center.x += 1 * Time::delta;
	if (Input::KeyHold(Key_I)) center.y -= 1 * Time::delta;
	else if (Input::KeyHold(Key_K)) center.y += 1 * Time::delta;
	if (Input::KeyHold(Key_U)) center.z -= 1 * Time::delta;
	else if (Input::KeyHold(Key_O)) center.z += 1 * Time::delta;
}

void rendFunc() {
	MVP::Switch(false);
	MVP::Clear();
	float csz = cos(-rz*deg2rad);
	float snz = sin(-rz*deg2rad);
	float csw = cos(rw*deg2rad);
	float snw = sin(rw*deg2rad);
	Mat4x4 mMatrix = Mat4x4(1, 0, 0, 0, 0, csw, snw, 0, 0, -snw, csw, 0, 0, 0, 0, 1) * Mat4x4(csz, 0, -snz, 0, 0, 1, 0, 0, snz, 0, csz, 0, 0, 0, 0, 1);
	MVP::Mul(mMatrix);
	MVP::Translate(-gro->boundingBox.x / 2, -gro->boundingBox.y / 2, -gro->boundingBox.z / 2);
	MVP::Translate(center);
	auto _mv = MVP::modelview();
	auto _p = MVP::projection();
	auto _cpos = ChokoLait::mainCamera->object->transform.position();
	auto _cfwd = ChokoLait::mainCamera->object->transform.forward();
	GLint mv = glGetUniformLocation(shad->pointer, "_MV");
	GLint p = glGetUniformLocation(shad->pointer, "_P");
	GLint cpos = glGetUniformLocation(shad->pointer, "camPos");
	GLint cfwd = glGetUniformLocation(shad->pointer, "camFwd");
	GLint scr = glGetUniformLocation(shad->pointer, "screenSize");

	int rpx = (int)round(repx);
	int rpy = (int)round(repy);
	int rpz = (int)round(repz);
	for (int x = -rpx; x <= rpx; x++) {
		for (int y = -rpy; y <= rpy; y++) {
			for (int z = -rpz; z <= rpz; z++) {
				glUseProgram(shad->pointer);
				glUniformMatrix4fv(mv, 1, GL_FALSE, glm::value_ptr(_mv));
				glUniformMatrix4fv(p, 1, GL_FALSE, glm::value_ptr(_p));
				glUniform3f(cpos, _cpos.x, _cpos.y, _cpos.z);
				glUniform3f(cfwd, _cfwd.x, _cfwd.y, _cfwd.z);
				glUniform2f(scr, Display::width, Display::height);
				glBindVertexArray(gro->_vao);
				glDrawArrays(GL_POINTS, 0, 10000000);
				glBindVertexArray(0);
				glUseProgram(0);
			}
		}
	}
}

uint fpsc, fps;
float told;
void paintfunc() {
	if (++fpsc == 100) {
		fps = (uint)roundf(100.0f / (Time::time - told));
		told = Time::time;
		fpsc = 0;
	}
	UI::Label(10, Display::height - 20, 12, "fps: " + std::to_string(fps), font, white());
	if (Engine::Button(10, 10, 120, 18, white(1, 0.5f), drawMesh? "Spheres" : "Lines", 12, font, white(), true) == MOUSE_RELEASE) {
		drawMesh = !drawMesh;
	}
	if (Engine::Button(10, 30, 120, 18, white(1, 0.5f), "Reload colors", 12, font, white(), true) == MOUSE_RELEASE) {
		Gromacs::LoadFiles();
		gro->ReloadCols();
	}
	if (Engine::Button(10, 50, 120, 18, white(1, 0.5f), "Reset view", 12, font, white(), true) == MOUSE_RELEASE) {
		rw = rz = 0;
		center = Vec3();
	}

	UI::Label(10, 80, 12, "sky str", font, white());
	auto& set = Scene::active->settings;
	set.skyStrength = Engine::DrawSliderFill(80, 80, 100, 14, 0, 2, set.skyStrength, white(1, 0.3f), white());
	UI::Label(10, 200, 12, "rep x", font, white());
	repx = Engine::DrawSliderFill(80, 200, 100, 14, 0, 5, repx, white(1, 0.3f), white());
	UI::Label(10, 220, 12, "rep y", font, white());
	repy = Engine::DrawSliderFill(80, 220, 100, 14, 0, 5, repy, white(1, 0.3f), white());
	UI::Label(10, 240, 12, "rep z", font, white());
	repz = Engine::DrawSliderFill(80, 240, 100, 14, 0, 5, repz, white(1, 0.3f), white());
}

#include "py/pyreader.h"
#include "py/pynode.h"

PyScript *scr, *scr2;
PyNode *node, *node2;
void paintfunc2() {
	Engine::DrawQuad(0, 0, Display::width, Display::height, black());

	node->Draw(Vec2(100, 100));
}

int main(int argc, char **argv)
{
	ChokoLait::Init(800, 800);
	bg = new Background(IO::path + "/refl.hdr");
	font = new Font(IO::path + "/arimo.ttf", ALIGN_TOPLEFT);
	PyReader::Init();
	PyNode::Init();
	PyNode::font = font;
	if (!PyReader::Read(IO::path + "/py/foo.py", &scr))
		abort();
	if (!PyReader::Read(IO::path + "/py/boo.py", &scr2))
		abort();
	node = new PyNode(scr);
	scr->SetVal(0, 1.0f);
	scr->SetVal(1, 1);
	node2 = new PyNode(scr2);

	string op = scr->Exec();
	scr2->invars[0].value = scr->pRets[0];
	op = scr2->Exec();
	auto f = (float*)scr2->Get(0);

	while (ChokoLait::alive()) {
		ChokoLait::Update();
		ChokoLait::Paint(nullptr, paintfunc2);
	}

	/*
	auto& set = Scene::active->settings;
	set.sky = bg;
	set.skyStrength = 1.5f;
	set.skyBrightness = 0;
	shad = new Shader(IO::GetText(IO::path + "/parV.txt"), IO::GetText(IO::path + "/parF.txt"));
	//Gromacs::LoadFiles();
	gro = new Gromacs(IO::path + "/md.gro");
	glEnable(GL_PROGRAM_POINT_SIZE);
	//glPointSize(20);

	while (ChokoLait::alive()) {
		ChokoLait::Update(updateFunc);
		//ChokoLait::mainCamera->object->transform.localPosition(Vec3(0, 0, -camz));

		ChokoLait::Paint(rendFunc, paintfunc);
	}
	*/
}