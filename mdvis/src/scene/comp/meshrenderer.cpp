#include "Engine.h"

MeshRenderer::MeshRenderer() : Component("Mesh Renderer", COMP_MRD, DRAWORDER_SOLID | DRAWORDER_TRANSPARENT, nullptr, { COMP_MFT }) {
	_materials.push_back(-1);
}

void MeshRenderer::DrawDeferred(GLuint shader) {
	MeshFilter* mf = (MeshFilter*)dependacyPointers[0].raw();
	if (!mf->mesh || !mf->mesh->loaded)
		return;
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	Mat4x4 m1 = MVP::modelview();
	Mat4x4 m2 = MVP::projection();

	glBindVertexArray(mf->mesh->vao);

	for (uint m = 0; m < mf->mesh->materialCount; m++) {
		if (!materials[m])
			continue;
		if (shader == 0) materials[m]->ApplyGL(m1, m2);
		else glUseProgram(shader);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mf->mesh->_matIndicesBuffers[m]);
		glDrawElements(GL_TRIANGLES, mf->mesh->_matTriangles[m].size(), GL_UNSIGNED_INT, 0);
		//glDrawElements(GL_TRIANGLES, mf->mesh->_matTriangles[m].size(), GL_UNSIGNED_INT, &mf->mesh->_matTriangles[m][0]);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glUseProgram(0);
	glBindVertexArray(0);
	glDisable(GL_CULL_FACE);
}

void MeshRenderer::_UpdateMat(void* i) {
	MeshRenderer* mf = (MeshRenderer*)i;
	for (int q = mf->_materials.size() - 1; q >= 0; q--) {
		mf->materials[q](_GetCache<Material>(ASSETTYPE_MATERIAL, mf->_materials[q]));
	}
}

void MeshRenderer::_UpdateTex(void* i) {
	MatVal_Tex* v = (MatVal_Tex*)i;
	v->tex = _GetCache<Texture>(ASSETTYPE_TEXTURE, v->id);
}

void MeshRenderer::Refresh() {
	MeshFilter* mf = (MeshFilter*)dependacyPointers[0].raw();
	if (!mf || !mf->mesh || !mf->mesh->loaded) {
		materials.clear();
		_materials.clear();
	}
	else {
		//Ref<Material> emt;
		materials.resize(mf->mesh->materialCount);// , emt);
		_materials.resize(mf->mesh->materialCount, -1);
	}
}