//#include "SceneObjects.h"
#include "Engine.h"
#include "Editor.h"

Object::Object(string nm) : id(Engine::GetNewId()), name(nm) {}

bool DrawComponentHeader(Editor* e, Vec4 v, uint pos, Component* c) {
	Engine::DrawQuad(v.r, v.g + pos, v.b - 17, 16, grey2());
	//bool hi = expand;
	if (Engine::EButton((e->editorLayer == 0), v.r, v.g + pos, 16, 16, c->_expanded ? e->collapse : e->expand, white()) == MOUSE_RELEASE) {
		c->_expanded = !c->_expanded;//hi = !expand;
	}
	//Engine::DrawTexture(v.r, v.g + pos, 16, 16, c->_expanded ? e->collapse : e->expand);
	if (Engine::EButton(e->editorLayer == 0, v.r + v.b - 16, v.g + pos, 16, 16, e->buttonX, white(1, 0.7f)) == MOUSE_RELEASE) {
		//delete
		c->object->RemoveComponent(c);
		if (c == nullptr)
			e->WAITINGREFRESHFLAG = true;
		return false;
	}
	Engine::Label(v.r + 20, v.g + pos + 3, 12, c->name, e->font, white());
	return c->_expanded;
}

Component::Component(string name, COMPONENT_TYPE t, DRAWORDER drawOrder, SceneObject* o, vector<COMPONENT_TYPE> dep) : Object(name), componentType(t), active(true), drawOrder(drawOrder), _expanded(true), dependancies(dep), object(o) {
	for (COMPONENT_TYPE t : dependancies) {
		dependacyPointers.push_back(nullptr);
	}
}

int Camera::camVertsIds[19] = { 0, 1, 0, 2, 0, 3, 0, 4, 1, 2, 2, 4, 4, 3, 3, 1, 1, 2, 5 };

Camera::Camera() : Component("Camera", COMP_CAM, DRAWORDER_NOT), ortographic(false), fov(60), orthoSize(10), screenPos(0.3f, 0.1f, 0.6f, 0.4f), clearType(CAM_CLEAR_COLOR), clearColor(black(1)), _tarRT(-1) {
	UpdateCamVerts();
	InitGBuffer();
}

Camera::Camera(ifstream& stream, SceneObject* o, long pos) : Camera() {
	if (pos >= 0)
		stream.seekg(pos);
	_Strm2Val(stream, fov);
	_Strm2Val(stream, screenPos.x);
	_Strm2Val(stream, screenPos.y);
	_Strm2Val(stream, screenPos.w);
	_Strm2Val(stream, screenPos.h);
}

/// <summary>Clear, Reset Projection Matrix</summary>
void Camera::ApplyGL() {
	switch (clearType) {
	case CAM_CLEAR_COLOR:
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT);
		glClearDepth(1);
		glClear(GL_DEPTH_BUFFER_BIT);
		break;
	case CAM_CLEAR_DEPTH:
		glClearDepth(1);
		glClear(GL_DEPTH_BUFFER_BIT);
		break;
	}
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(1.2f, -1.2f, 1);
	glMultMatrixf(glm::value_ptr(glm::perspective(fov*0.5f, Display::width*1.0f / Display::height, nearClip, farClip)));
	glScalef(-1, 1, -1);
	//rotation matrix here

	Vec3 pos = -object->transform.worldPosition();
	glTranslatef(pos.x, pos.y, pos.z);
}

void Camera::UpdateCamVerts() {
#ifdef IS_EDITOR
	Vec3 cst = Vec3(cos(fov*0.5f * 3.14159265f / 180), sin(fov*0.5f * 3.14159265f / 180), tan(fov*0.5f * 3.14159265f / 180))*cos(fov*0.618f * 3.14159265f / 180);
	camVerts[1] = Vec3(cst.x, cst.y, 1 - cst.z) * 2.0f;
	camVerts[2] = Vec3(-cst.x, cst.y, 1 - cst.z) * 2.0f;
	camVerts[3] = Vec3(cst.x, -cst.y, 1 - cst.z) * 2.0f;
	camVerts[4] = Vec3(-cst.x, -cst.y, 1 - cst.z) * 2.0f;
	camVerts[5] = Vec3(0, cst.y * 1.5f, 1 - cst.z) * 2.0f;
#endif
}

void Camera::InitGBuffer() {
#ifndef IS_EDITOR
	glGenFramebuffers(1, &d_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, d_fbo);

	// Create the gbuffer textures
	glGenTextures(3, d_texs);
	glGenTextures(1, &d_depthTex);

	for (uint i = 0; i < 3; i++) {
		glBindTexture(GL_TEXTURE_2D, d_texs[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, Display::width, Display::height, 0, GL_RGB, GL_FLOAT, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, d_texs[i], 0);
	}

	// depth
	glBindTexture(GL_TEXTURE_2D, d_depthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, Display::width, Display::height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, d_depthTex, 0);

	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, DrawBuffers);

	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (Status != GL_FRAMEBUFFER_COMPLETE) {
		Debug::Error("Camera", "FB error:" + Status);
	}

	// restore default FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
#endif
}

void Camera::DrawEditor(EB_Viewer* ebv) {
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, &camVerts[0]);
	glLineWidth(1);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor4f(0, 0, 0, 1);
	glDrawElements(GL_LINES, 16, GL_UNSIGNED_INT, &camVertsIds[0]);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, &camVertsIds[16]);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void Camera::DrawInspector(Editor* e, Component*& c, Vec4 v, uint& pos) {
	Camera* cam = (Camera*)c;
	if (DrawComponentHeader(e, v, pos, this)) {
		Engine::Label(v.r + 2, v.g + pos + 20, 12, "Field of view", e->font, white());
		Engine::DrawQuad(v.r + v.b * 0.3f, v.g + pos + 17, v.b * 0.3f - 1, 16, grey1());
		Engine::Label(v.r + v.b * 0.3f + 2, v.g + pos + 20, 12, to_string(cam->fov), e->font, white());
		cam->fov = Engine::DrawSliderFill(v.r + v.b*0.6f, v.g + pos + 17, v.b * 0.4f-1, 16, 0.1f, 179.9f, cam->fov, grey1(), white());
		Engine::Label(v.r + 2, v.g + pos + 35, 12, "Frustrum", e->font, white());
		Engine::Label(v.r + 4, v.g + pos + 50, 12, "X", e->font, white());
		Engine::DrawQuad(v.r + 20, v.g + pos + 47, v.b*0.3f - 20, 16, grey1());
		Engine::Label(v.r + v.b*0.3f + 4, v.g + pos + 50, 12, "Y", e->font, white());
		Engine::DrawQuad(v.r + v.b*0.3f + 20, v.g + pos + 47, v.b*0.3f - 20, 16, grey1());
		Engine::Label(v.r + 4, v.g + pos + 67, 12, "W", e->font, white());
		Engine::DrawQuad(v.r + 20, v.g + pos + 64, v.b*0.3f - 20, 16, grey1());
		Engine::Label(v.r + v.b*0.3f + 4, v.g + pos + 67, 12, "H", e->font, white());
		Engine::DrawQuad(v.r + v.b*0.3f + 20, v.g + pos + 64, v.b*0.3f - 20, 16, grey1());
		float dh = ((v.b*0.35f - 1)*Display::height / Display::width) - 1;
		Engine::DrawQuad(v.r + v.b*0.65f, v.g + pos + 35, v.b*0.35f - 1, dh, grey1());
		Engine::DrawQuad(v.r + v.b*0.65f + ((v.b*0.35f - 1)*screenPos.x), v.g + pos + 35 + dh*screenPos.y, (v.b*0.35f - 1)*screenPos.w, dh*screenPos.h, grey2());
		pos += (uint)max(37 + dh, 87);
		Engine::Label(v.r + 2, v.g + pos + 4, 12, "Filtering", e->font, white());
		vector<string> clearNames = { "None", "Color and Depth", "Depth only", "Skybox" };
		if (Engine::EButton(e->editorLayer == 0, v.r + v.b * 0.3f, v.g + pos + 1, v.b * 0.7f - 1, 14, grey2(), clearNames[clearType], 12, e->font, white()) == MOUSE_PRESS) {
			e->RegisterMenu(nullptr, "", clearNames, { &_SetClear0, &_SetClear1, &_SetClear2, &_SetClear3 }, 0, Vec2(v.r + v.b * 0.3f, v.g + pos));
		}
		Engine::Label(v.r + 2, v.g + pos + 20, 12, "Target", e->font, white());
		e->DrawAssetSelector(v.r + v.b * 0.3f, v.g + pos + 17, v.b*0.7f, 16, grey1(), ASSETTYPE_RENDERTEXTURE, 12, e->font, &_tarRT, nullptr, this);
		pos += 34;
	}
	else pos += 17;
}

void Camera::Serialize(Editor* e, ofstream* stream) {
	_StreamWrite(&fov, stream, 4);
	_StreamWrite(&screenPos.x, stream, 4);
	_StreamWrite(&screenPos.y, stream, 4);
	_StreamWrite(&screenPos.w, stream, 4);
	_StreamWrite(&screenPos.h, stream, 4);
}

void Camera::_SetClear0(EditorBlock* b) {
	Editor::instance->selected->GetComponent<Camera>()->clearType = CAM_CLEAR_NONE;
}
void Camera::_SetClear1(EditorBlock* b) {
	Editor::instance->selected->GetComponent<Camera>()->clearType = CAM_CLEAR_COLOR;
}
void Camera::_SetClear2(EditorBlock* b) {
	Editor::instance->selected->GetComponent<Camera>()->clearType = CAM_CLEAR_DEPTH;
}
void Camera::_SetClear3(EditorBlock* b) {
	Editor::instance->selected->GetComponent<Camera>()->clearType = CAM_CLEAR_SKY;
}

MeshFilter::MeshFilter() : Component("Mesh Filter", COMP_MFT, DRAWORDER_NOT), _mesh(-1) {

}

void MeshFilter::DrawInspector(Editor* e, Component*& c, Vec4 v, uint& pos) {
	//MeshFilter* mft = (MeshFilter*)c;
	if (DrawComponentHeader(e, v, pos, this)) {
		Engine::Label(v.r + 2, v.g + pos + 20, 12, "Mesh", e->font, white());
		e->DrawAssetSelector(v.r + v.b * 0.3f, v.g + pos + 17, v.b*0.7f, 16, grey1(), ASSETTYPE_MESH, 12, e->font, &_mesh, &_UpdateMesh, this);
		pos += 34;
	}
	else pos += 17;
}

void MeshFilter::SetMesh(int i) {
	_mesh = i;
	if (i >= 0)
		mesh = _GetCache<Mesh>(ASSETTYPE_MESH, i);
	else
		mesh = nullptr;
	object->Refresh();
}

void MeshFilter::_UpdateMesh(void* i) {
	MeshFilter* mf = (MeshFilter*)i;
	if (mf->_mesh >= 0) {
		mf->mesh = _GetCache<Mesh>(ASSETTYPE_MESH, mf->_mesh);
	}
	else
		mf->mesh = nullptr;
	mf->object->Refresh();
}

void MeshFilter::Serialize(Editor* e, ofstream* stream) {
	_StreamWriteAsset(e, stream, ASSETTYPE_MESH, _mesh);
}

MeshFilter::MeshFilter(ifstream& stream, SceneObject* o, long pos) : Component("Mesh Filter", COMP_MFT, DRAWORDER_NOT, o), _mesh(-1) {
	if (pos >= 0)
		stream.seekg(pos);
	ASSETTYPE t;
	_Strm2Asset(stream, Editor::instance, t, _mesh);
	if (_mesh >= 0)
		mesh = _GetCache<Mesh>(ASSETTYPE_MESH, _mesh);
	object->Refresh();
}

MeshRenderer::MeshRenderer() : Component("Mesh Renderer", COMP_MRD, DRAWORDER_SOLID | DRAWORDER_TRANSPARENT, nullptr, {COMP_MFT}) {
	_materials.push_back(-1);
}

MeshRenderer::MeshRenderer(ifstream& stream, SceneObject* o, long pos) : Component("Mesh Renderer", COMP_MRD, DRAWORDER_SOLID | DRAWORDER_TRANSPARENT, o, { COMP_MFT }) {
	_UpdateMat(this);
	if (pos >= 0)
		stream.seekg(pos);
	ASSETTYPE t;
	int s;
	_Strm2Val(stream, s);
	for (int q = 0; q < s; q++) {
		Material* m;
		ASSETID i;
		_Strm2Asset(stream, Editor::instance, t, i);
		m = _GetCache<Material>(ASSETTYPE_MATERIAL, i);
		materials.push_back(m);
		_materials.push_back(i);
	}
	//_Strm2Asset(stream, Editor::instance, t, _mat);
}

void MeshRenderer::DrawEditor(EB_Viewer* ebv) {
	MeshFilter* mf = (MeshFilter*)dependacyPointers[0];
	if (mf == nullptr || mf->mesh == nullptr || !mf->mesh->loaded)
		return;
	glEnableClientState(GL_VERTEX_ARRAY);
	glPolygonMode(GL_FRONT_AND_BACK, (ebv->selectedShading == 0) ? GL_FILL : GL_LINE);
	glEnable(GL_CULL_FACE);
	glVertexPointer(3, GL_FLOAT, 0, &(mf->mesh->vertices[0]));
	glLineWidth(1);
	GLfloat matrix[16], matrix2[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	glGetFloatv(GL_PROJECTION_MATRIX, matrix2);
	glm::mat4 m1(matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6], matrix[7], matrix[8], matrix[9], matrix[10], matrix[11], matrix[12], matrix[13], matrix[14], matrix[15]);
	glm::mat4 m2(matrix2[0], matrix2[1], matrix2[2], matrix2[3], matrix2[4], matrix2[5], matrix2[6], matrix2[7], matrix2[8], matrix2[9], matrix2[10], matrix2[11], matrix2[12], matrix2[13], matrix2[14], matrix2[15]);
	glm::mat4 mat(m2*m1);
	//glm::mat4()
	for (uint m = 0; m < mf->mesh->materialCount; m++) {
		if (materials[m] == nullptr)
			continue;
		materials[m]->ApplyGL(mat);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 0, &(mf->mesh->vertices[0]));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, 0, &(mf->mesh->uv0[0]));
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, 0, &(mf->mesh->normals[0]));
		glDrawElements(GL_TRIANGLES, mf->mesh->_matTriangles[m].size(), GL_UNSIGNED_INT, &(mf->mesh->_matTriangles[m][0]));
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
	}
	glUseProgram(0);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_CULL_FACE);
}

void MeshRenderer::DrawInspector(Editor* e, Component*& c, Vec4 v, uint& pos) {
	//MeshRenderer* mrd = (MeshRenderer*)c;
	if (DrawComponentHeader(e, v, pos, this)) {
		MeshFilter* mft = (MeshFilter*)dependacyPointers[0];
		if (mft->mesh == nullptr) {
			Engine::Label(v.r + 2, v.g + pos + 20, 12, "No Mesh Assigned!", e->font, white());
			pos += 34;
		}
		else {
			Engine::Label(v.r + 2, v.g + pos + 20, 12, "Materials: " + to_string(mft->mesh->materialCount), e->font, white());
			for (uint a = 0; a < mft->mesh->materialCount; a++) {
				Engine::Label(v.r + 2, v.g + pos + 37, 12, "Material " + to_string(a), e->font, white());
				e->DrawAssetSelector(v.r + v.b * 0.3f, v.g + pos + 34, v.b*0.7f, 16, grey1(), ASSETTYPE_MATERIAL, 12, e->font, &_materials[a], & _UpdateMat, this);
				pos += 17;
				if (materials[a] == nullptr)
					continue;
				for (uint q = 0; q < materials[a]->valOrders.size(); q++) {
					Engine::Label(v.r + 20, v.g + pos + 38, 12, materials[a]->valNames[materials[a]->valOrders[q]][materials[a]->valOrderIds[q]], e->font, white());
					Engine::DrawTexture(v.r + 3, v.g + pos + 35, 16, 16, e->matVarTexs[materials[a]->valOrders[q]]);
					void* bbs = materials[a]->vals[materials[a]->valOrders[q]][materials[a]->valOrderGLIds[q]];
					switch (materials[a]->valOrders[q]) {
					case SHADER_INT:
						Engine::Button(v.r + v.b * 0.3f + 17, v.g + pos + 35, v.b*0.7f - 17, 16, grey1(), to_string(*(int*)bbs), 12, e->font, white());
						break;
					case SHADER_FLOAT:
						Engine::Button(v.r + v.b * 0.3f + 17, v.g + pos + 35, v.b*0.7f - 17, 16, grey1(), to_string(*(float*)bbs), 12, e->font, white());
						break;
					case SHADER_SAMPLER:
						e->DrawAssetSelector(v.r + v.b * 0.3f + 17, v.g + pos + 35, v.b*0.7f - 17, 16, grey1(), ASSETTYPE_TEXTURE, 12, e->font, &((MatVal_Tex*)bbs)->id, _UpdateTex, bbs);
						break;
					}
					pos += 17;
				}
				pos++;
				/*
				for (auto aa : materials[a]->vals) {
					int r = 0;
					for (auto bb : aa.second) {
						Engine::Label(v.r + 27, v.g + pos + 38, 12, materials[a]->valNames[aa.first][r], e->font, white());

						Engine::DrawTexture(v.r + v.b * 0.3f, v.g + pos + 35, 16, 16, e->matVarTexs[aa.first]);
						switch (aa.first) {
						case SHADER_INT:
							Engine::Button(v.r + v.b * 0.3f + 17, v.g + pos + 35, v.b*0.7f - 17, 16, grey1(), to_string(*(int*)bb.second), 12, e->font, white());
							break;
						case SHADER_FLOAT:
							Engine::Button(v.r + v.b * 0.3f + 17, v.g + pos + 35, v.b*0.7f - 17, 16, grey1(), to_string(*(float*)bb.second), 12, e->font, white());
							break;
						case SHADER_SAMPLER:
							e->DrawAssetSelector(v.r + v.b * 0.3f + 17, v.g + pos + 35, v.b*0.7f - 17, 16, grey1(), ASSETTYPE_TEXTURE, 12, e->font, &((MatVal_Tex*)bb.second)->id, _UpdateTex, bb.second);
							break;
						}
						r++;
						pos += 17;
					}
				}
				pos += 1;
				*/
			}
			pos += 34;
		}
	}
	else pos += 17;
}

void MeshRenderer::_UpdateMat(void* i) {
	MeshRenderer* mf = (MeshRenderer*)i;
	for (int q = mf->_materials.size() - 1; q >= 0; q--) {
		mf->materials[q] = _GetCache<Material>(ASSETTYPE_MATERIAL, mf->_materials[q]);
	}
}

void MeshRenderer::_UpdateTex(void* i) {
	MatVal_Tex* v = (MatVal_Tex*)i;
	v->tex = _GetCache<Texture>(ASSETTYPE_TEXTURE, v->id);
}

void MeshRenderer::Serialize(Editor* e, ofstream* stream) {
	int s = _materials.size();
	_StreamWrite(&s, stream, 4);
	for (ASSETID i : _materials)
		_StreamWriteAsset(e, stream, ASSETTYPE_MATERIAL, i);
}

void MeshRenderer::Refresh() {
	MeshFilter* mf = (MeshFilter*)dependacyPointers[0];
	if (mf == nullptr || mf->mesh == nullptr || !mf->mesh->loaded) {
		materials.clear();
		_materials.clear();
	}
	else {
		materials.resize(mf->mesh->materialCount, nullptr);
		_materials.resize(mf->mesh->materialCount, -1);
	}
}

void TextureRenderer::DrawInspector(Editor* e, Component*& c, Vec4 v, uint& pos) {
	//MeshRenderer* mrd = (MeshRenderer*)c;
	if (DrawComponentHeader(e, v, pos, this)) {
		Engine::Label(v.r + 2, v.g + pos + 20, 12, "Texture", e->font, white());
		e->DrawAssetSelector(v.r + v.b * 0.3f, v.g + pos + 17, v.b*0.7f, 16, grey1(), ASSETTYPE_TEXTURE, 12, e->font, &_texture);
		pos += 34;
	}
	else pos += 17;
}

void TextureRenderer::Serialize(Editor* e, ofstream* stream) {
	_StreamWriteAsset(e, stream, ASSETTYPE_TEXTURE, _texture);
}

TextureRenderer::TextureRenderer(ifstream& stream, SceneObject* o, long pos) : Component("Texture Renderer", COMP_TRD, DRAWORDER_OVERLAY, o) {
	if (pos >= 0)
		stream.seekg(pos);
	ASSETTYPE t;
	_Strm2Asset(stream, Editor::instance, t, _texture);
}

SceneScript::SceneScript(Editor* e, ASSETID id) : Component(e->headerAssets[id] + " (Script)", COMP_SCR, DRAWORDER_NOT), _script(id) {
	if (id < 0) {
		name = "missing script!";
		return;
	}
	ifstream strm(e->projectFolder + "Assets\\" + e->headerAssets[id] + ".meta", ios::in | ios::binary);
	if (!strm.is_open()) {
		e->_Error("Script Component", "Cannot read meta file!");
		_script = -1;
		return;
	}
	ushort sz;
	_Strm2Val<ushort>(strm, sz);
	_vals.resize(sz);
	SCR_VARTYPE t;
	for (ushort x = 0; x < sz; x++) {
		_Strm2Val<SCR_VARTYPE>(strm, t);
		_vals[x].second.first = t;
		char c[100];
		strm.getline(&c[0], 100, 0);
		_vals[x].first += string(c);
		switch (t) {
		case SCR_VAR_INT:
			_vals[x].second.second = new int(0);
			break;
		case SCR_VAR_FLOAT:
			_vals[x].second.second = new float(0);
			break;
		case SCR_VAR_STRING:
			_vals[x].second.second = new string("");
			break;
		}
	}
}

SceneScript::SceneScript(ifstream& strm, SceneObject* o) : Component("(Script)", COMP_SCR, DRAWORDER_NOT), _script(-1) {
	char* c = new char[100];
	strm.getline(c, 100, 0);
	string s(c);
	int i = 0;
	for (auto a : Editor::instance->headerAssets) {
		if (a == s) {
			_script = i;
			name = a + " (Script)";
			break;
		}
		i++;
	}
	delete[](c);
}

SceneScript::~SceneScript() {
#ifdef IS_EDITOR
	for (auto a : _vals) {
		switch (a.second.first) {
		case SCR_VAR_INT:
		case SCR_VAR_FLOAT:
			delete(a.second.second);
		}
	}
#endif
}

void SceneScript::Serialize(Editor* e, ofstream* stream) {
	_StreamWriteAsset(e, stream, ASSETTYPE_SCRIPT_H, _script);
}

void SceneScript::DrawInspector(Editor* e, Component*& c, Vec4 v, uint& pos) {
	SceneScript* scr = (SceneScript*)c;
	if (DrawComponentHeader(e, v, pos, this)) {
		pos += 100;
	}
	else pos += 17;
}

void SceneScript::Parse(string s, Editor* e) {
	string p = e->projectFolder + "Assets\\" + s;
	ifstream strm(p.c_str(), ios::in);
	char* c = new char[100];
	int flo = s.find_last_of('\\') + 1;
	if (flo == string::npos) flo = 0;
	string sc = ("class" + s.substr(flo, s.size() - 2 - flo) + ":publicSceneScript{");
	bool hasClass = false, isPublic = false;
	ushort count = 0;
	while (!strm.eof()) {
		strm.getline(c, 100);
		string ss;
		for (uint a = 0; a < 100; a++) {
			if (c[a] != ' ' && c[a] != '\t' && c[a] != '\r') {
				ss += c[a];
			}
		}
		if (strcmp(sc.c_str(), ss.c_str()) == 0) {
			hasClass = true;
			string op = p + ".meta";
			ofstream oStrm(op.c_str(), ios::out | ios::binary | ios::trunc);
			oStrm << "XX";
			//read variables
			while (!strm.eof()) {
				strm.getline(c, 100);
				string sss(c);
				while (sss != "" && sss[0] == ' ' || sss[0] == '\t')
					sss = sss.substr(1);
				if (sss == "" || sss.find('(') != string::npos)
					continue;
				if (!isPublic && sss == "public:") {
					isPublic = true;
					continue;
				}
				else if (isPublic && (sss == "protected:" || sss == "private:" || sss == "};"))
					break;
				if (!isPublic)
					continue;
				int spos = sss.find_first_of(" ");
				if (spos == string::npos)
					continue;
				string tp = sss.substr(0, spos);
				if (tp == "int") {
					oStrm << (char)SCR_VAR_INT;
				}
				else if (tp == "float") {
					oStrm << (char)SCR_VAR_FLOAT;
				}
				else if (tp == "string" || tp == "std::string") {
					oStrm << (char)SCR_VAR_STRING;
				}
				else if (tp == "Texture*") {
					oStrm << (char)SCR_VAR_TEXTURE;
				}
				else continue;
				int spos2 = min(min(sss.find_first_of(" ", spos + 1), sss.find_first_of(";", spos + 1)), sss.find_first_of("=", spos + 1));
				if (spos2 == string::npos) {
					long l = (long)oStrm.tellp();
					oStrm.seekp(l - 1);
					continue;
				}
				string s2 = sss.substr(spos + 1, spos2 - spos - 1);
				while (sss != "" && sss[0] == ' ' || sss[0] == '\t')
					sss = sss.substr(1);
				if (sss == ""){
					long l = (long)oStrm.tellp();
					oStrm.seekp(l - 1);
					continue;
				}
				oStrm << s2 << (char)0;
				count++;
			}
			oStrm.seekp(0);
			_StreamWrite(&count, &oStrm, 2);
			oStrm.close();
		}
	}
	if (!hasClass)
		e->_Error("Script Importer", "SceneScript class for " + s + " not found!");
}

SceneObject::SceneObject() : SceneObject(Vec3(), Quat(), Vec3(1, 1, 1)) {}
SceneObject::SceneObject(string s) : SceneObject(s, Vec3(), Quat(), Vec3(1, 1, 1)) {}
SceneObject::SceneObject(Vec3 pos, Quat rot, Vec3 scale) : SceneObject("New Object", Vec3(), Quat(), Vec3(1, 1, 1)) {}
SceneObject::SceneObject(string s, Vec3 pos, Quat rot, Vec3 scale) : active(true), transform(this, pos, rot, scale), childCount(0), _expanded(true), Object(s) {
	id = Engine::GetNewId();
}

void SceneObject::Enable() {
	active = true;
}

void SceneObject::Enable(bool enableAll) {
	active = false;
}

Component* ComponentFromType (COMPONENT_TYPE t){
	switch (t) {
	case COMP_CAM:
		return new Camera();
	case COMP_MFT:
		return new MeshFilter();
	default:
		return nullptr;
	}
}

SceneObject* SceneObject::AddChild(SceneObject* child) { 
	childCount++; 
	children.push_back(child); 
	child->parent = this;
	return this;
}

Component* SceneObject::AddComponent(Component* c) {
	c->object = this;
	int i = 0;
	for (COMPONENT_TYPE t : c->dependancies) {
		c->dependacyPointers[i] = GetComponent(t);
		if (c->dependacyPointers[i] == nullptr) {
			c->dependacyPointers[i] = AddComponent(ComponentFromType(t));
		}
		i++;
	}
	for (Component* cc : _components)
	{
		if ((cc->componentType == c->componentType) && cc->componentType != COMP_SCR) {
			Debug::Message("Add Component", "Same component already exists!");
			delete(c);
			return cc;
		}
	}
	_components.push_back(c);
	return c;
}

/// <summary>you should probably use GetComponent&lt;T&gt;() instead.</summary>
Component* SceneObject::GetComponent(COMPONENT_TYPE type) {
	for (Component* cc : _components)
	{
		if (cc->componentType == type) {
			return cc;
		}
	}
	return nullptr;
}

void SceneObject::RemoveComponent(Component*& c) {
	for (int a = _components.size()-1; a >= 0; a--) {
		if (_components[a] == c) {
			for (int aa = _components.size()-1; aa >= 0; aa--) {
				for (COMPONENT_TYPE t : _components[aa]->dependancies) {
					if (t == c->componentType) {
						Editor::instance->_Warning("Component Deleter", "Cannot delete " + c->name + " because other components depend on it!");
						return;
					}
				}
			}
			_components.erase(_components.begin() + a);
			c = nullptr;
			return;
		}
	}
	Debug::Warning("SceneObject", "component to delete is not found");
}

void SceneObject::Refresh() {
	for (Component* c : _components) {
		c->Refresh();
	}
}