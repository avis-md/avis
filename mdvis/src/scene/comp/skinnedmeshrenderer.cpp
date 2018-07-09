#include "Engine.h"

ComputeShader* SkinnedMeshRenderer::skinningProg = 0;

SkinnedMeshRenderer::SkinnedMeshRenderer(SceneObject* o) : Component("Skinned Mesh Renderer", COMP_SRD, DRAWORDER_SOLID, o) {
	if (!o) {
		Debug::Error("SMR", "SceneObject cannot be null!");
	}
	rSceneObject& par = object->parent;
	while (par) {
		armature = par->GetComponent<Armature>().get();
		if (armature) break;
		else par = par->parent;
	}
	if (!armature) {
		Debug::Error("SkinnedMeshRenderer", "Cannot add Skin to object without armature!");
		dead = true;
	}
}

SkinnedMeshRenderer::SkinnedMeshRenderer(std::ifstream& stream, SceneObject* o, long pos) : Component("Skinned Mesh Renderer", COMP_SRD, DRAWORDER_SOLID, o) {

}

void SkinnedMeshRenderer::mesh(Mesh* m) {
	_mesh = m;
	InitWeights();
}

void SkinnedMeshRenderer::InitWeights() {
	std::vector<uint> noweights;
	//weights.clear();
	auto bsz = armature->_allbones.size();
	weights = std::vector<std::array<std::pair<ArmatureBone*, float>, 4>>(_mesh->vertexCount);
	std::vector<SkinDats> dats(_mesh->vertexCount);
	for (uint i = 0; i < _mesh->vertexCount; i++) {
		byte a = 0;
		float tot = 0;
		for (auto& g : _mesh->vertexGroupWeights[i]) {
			auto bn = armature->MapBone(_mesh->vertexGroups[g.first]);
			if (!bn) continue;
			weights[i][a].first = bn;
			weights[i][a].second = g.second;
			dats[i].mats[a] = bn->id;
			tot += g.second;
			if (++a == 4) break;
		}
		for (byte b = a; b < 4; b++) {
			weights[i][b].first = armature->_bones[0];
		}
		if (a == 0) {
			noweights.push_back(i);
			weights[i][0].second = 1;
			dats[i].weights[0] = 1;
		}
		else {
			while (a > 0) {
				weights[i][a - 1].second /= tot;
				dats[i].weights[a - 1] = weights[i][a - 1].second;
				a--;
			}
		}
	}

	if (skinBufPoss) {
		delete(skinBufPoss);
		delete(skinBufNrms);
		delete(skinBufPossO);
		delete(skinBufNrmsO);
		delete(skinBufDats);
		delete(skinBufMats);
		if (skinBufShps) {
			delete(skinBufShps);
			delete(skinBufShpWs);
		}
	}
	skinBufPoss = new ComputeBuffer<Vec4>(_mesh->vertexCount);
	skinBufNrms = new ComputeBuffer<Vec4>(_mesh->vertexCount);
	skinBufPossO = new ComputeBuffer<Vec4>(_mesh->vertexCount);
	skinBufNrmsO = new ComputeBuffer<Vec4>(_mesh->vertexCount);
	skinBufDats = new ComputeBuffer<SkinDats>(_mesh->vertexCount);
	skinBufMats = new ComputeBuffer<Mat4x4>(ARMATURE_MAX_BONES);
	skinBufPoss->Set(&_mesh->vertices[0].x, sizeof(Vec4), sizeof(Vec3));
	skinBufNrms->Set(&_mesh->normals[0].x, sizeof(Vec4), sizeof(Vec3));
	skinBufDats->Set(&dats[0]);
	if (!!_mesh->shapekeyCount) {
		skinBufShps = new ComputeBuffer<Vec4>(_mesh->vertexCount * _mesh->shapekeyCount);
		Vec3* keys = new Vec3[_mesh->vertexCount * _mesh->shapekeyCount];
		for (byte b = 0; b < _mesh->shapekeyCount; b++) {
			memcpy(keys + (_mesh->vertexCount * b), &_mesh->shapekeys[b].second[0], _mesh->vertexCount * sizeof(Vec3));
		}
		skinBufShps->Set(keys, sizeof(Vec4), sizeof(Vec3));
		memset(shapekeyWeights, 0, 64 * 4);
		skinBufShpWs = new ComputeBuffer<float>(64, shapekeyWeights);
	}
	skinDispatchGroups = (uint)ceilf(((float)_mesh->vertexCount) / SKINNED_THREADS_PER_GROUP);

	if (!!noweights.size())
		Debug::Warning("SMR", std::to_string(noweights.size()) + " vertices in \"" + _mesh->name + "\" have no weights assigned!");
}

void SkinnedMeshRenderer::DrawDeferred(GLuint shader) {
	if (!_mesh || !_mesh->loaded)
		return;
	Skin();

	//glEnableClientState(GL_VERTEX_ARRAY);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glEnable(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);
	//glVertexPointer(3, GL_FLOAT, 0, &(_mesh->vertices[0]));
	//glLineWidth(1);
	Mat4x4 m1 = MVP::modelview();
	Mat4x4 m2 = MVP::projection();

	/*
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, skinBufPossO->pointer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, sizeof(Vec4), 0);
	glBindBuffer(GL_ARRAY_BUFFER, skinBufNrmsO->pointer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, sizeof(Vec4), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE, 0, &(_mesh->tangents[0]));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, 0, &(_mesh->uv0[0]));
	*/
	glBindVertexArray(vao);

	for (uint m = 0; m < _mesh->materialCount; m++) {
		if (!materials[m])
			continue;
		if (shader == 0) materials[m]->ApplyGL(m1, m2);
		else glUseProgram(shader);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _mesh->_matIndicesBuffers[m]);
		glDrawElements(GL_TRIANGLES, _mesh->_matTriangles[m].size(), GL_UNSIGNED_INT, 0);
		//glDrawElements(GL_TRIANGLES, _mesh->_matTriangles[m].size(), GL_UNSIGNED_INT, &(_mesh->_matTriangles[m][0]));
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	/*
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	*/

	glUseProgram(0);
	//glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_CULL_FACE);
}

void SkinnedMeshRenderer::InitSkinning() {
	skinningProg = new ComputeShader(DefaultResources::GetStr("gpuskin.txt"));
}

void SkinnedMeshRenderer::Skin() {
	skinBufMats->Set(&armature->_animMatrices[0][0].x);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, skinBufPoss->pointer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, skinBufNrms->pointer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, skinBufDats->pointer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, skinBufMats->pointer);
	if (!!_mesh->shapekeyCount) {
		skinBufShpWs->Set(shapekeyWeights);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, skinBufShps->pointer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, skinBufShpWs->pointer);
	}
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, skinBufPossO->pointer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, skinBufNrmsO->pointer);
	//skinningProg->Dispatch(skinDispatchGroups, 1, 1);
	glUseProgram(skinningProg->pointer);
	//auto l = glGetUniformLocation(skinningProg->pointer, "params");
	glUniform4i(1, _mesh->vertexCount, _mesh->shapekeyCount, 0, 0);
	glDispatchCompute(skinDispatchGroups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glUseProgram(0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, 0);
}

void SkinnedMeshRenderer::InitVao() {
	if (vao) {
		glDeleteVertexArrays(1, &vao);
	}
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, skinBufPossO->pointer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec4), 0);
	glBindBuffer(GL_ARRAY_BUFFER, _mesh->vbos[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, skinBufNrmsO->pointer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vec4), NULL);
	glBindBuffer(GL_ARRAY_BUFFER, _mesh->vbos[3]);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	/*
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, skinBufPossO->pointer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, skinBufPossO->pointer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, _mesh->vbos[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, _mesh->vbos[3]);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	*/
}

void SkinnedMeshRenderer::SetMesh(int i) {
	_meshId = i;
	if (i >= 0) {
		_mesh = _GetCache<Mesh>(ASSETTYPE_MESH, i);
		InitWeights();
	}
	else
		_mesh = nullptr;
	if (!_mesh || !_mesh->loaded) {
		materials.clear();
		_materials.clear();
	}
	else {
		materials.resize(_mesh->materialCount, nullptr);
		_materials.resize(_mesh->materialCount, -1);
	}
	InitVao();
}

void SkinnedMeshRenderer::_UpdateMesh(void* i) {
	SkinnedMeshRenderer* mf = (SkinnedMeshRenderer*)i;
	if (mf->_meshId != -1) {
		mf->_mesh = _GetCache<Mesh>(ASSETTYPE_MESH, mf->_meshId);
	}
	else
		mf->_mesh = nullptr;
}
void SkinnedMeshRenderer::_UpdateMat(void* i) {
	SkinnedMeshRenderer* mf = (SkinnedMeshRenderer*)i;
	for (int q = mf->_materials.size() - 1; q >= 0; q--) {
		mf->materials[q] = _GetCache<Material>(ASSETTYPE_MATERIAL, mf->_materials[q]);
	}
}
void SkinnedMeshRenderer::_UpdateTex(void* i) {
	MatVal_Tex* v = (MatVal_Tex*)i;
	v->tex = _GetCache<Texture>(ASSETTYPE_TEXTURE, v->id);
}