#include "node_plot.h"
#ifndef IS_ANSERVER
#include "utils/plot.h"
#include "ui/ui_ext.h"
#include "ui/icons.h"
#include "md/particles.h"
#endif

INODE_DEF(__("Plot Data"), Plot, MISC)

Node_Plot::Node_Plot() : INODE_INITF(AN_FLAG_RUNONSEEK | AN_FLAG_RUNONVALCHG), type(TYPE::LINES), tex(0) {
	INODE_TITLE(NODE_COL_SPC);

	inputR.resize(3, nodecon(0, 0, false));
	inputVDef.resize(3);
	script->invars.push_back(std::pair<std::string, std::string>("array", "list(**)"));
	script->invars.push_back(std::pair<std::string, std::string>("X ID", "int"));
	script->invars.push_back(std::pair<std::string, std::string>("Y ID", "int"));
	script->invaropts.resize(3);
	inputVDef[1].i = inputVDef[2].i = -1;
	glGenTextures(1, &tex);
}

Node_Plot::~Node_Plot() {
	glDeleteTextures(1, &tex);
}

void Node_Plot::DrawHeader(float& off) {
	AnNode::DrawHeader(off);

	UI::Quad(pos.x, off, width, 17, bgCol);
	static std::string ss[] = {
		"Lines",
		"Lines (Accumulate)",
		"Scatter",
		"Density",
		"Contour",
		""
	};
	_di.target = (uint*)&type;
	_di.list = &ss[0];
	UI2::Dropdown(pos.x + 5, off, width - 10, "Type", _di);
	static auto _type = type;
	if (_type != type) {
		_type = type;
		script->invars[0].second = (type == TYPE::ALINES)? "*" : "list(**)";
		if (inputR[0].first) inputR[0].first->ConnectTo(inputR[0].second, this, 0);
	}
	off += 17;
}

void Node_Plot::DrawFooter(float& y) {
	//
	static bool drawFull;
	UI::Quad(pos.x, y, width, width, bgCol);
	if (drawFull) {
		switch (type) {
		case TYPE::LINES:
		case TYPE::ALINES:
			if (valXs.size()) {
				plt::plot(Display::width*0.5f - Display::height*0.45f, Display::height*0.05f, Display::height*0.9f, Display::height*0.9f, &valXs[0], &_valYs[0], valXs.size(), _valYs.size(), UI::font, 10, white(1, 0.8f));
			}
			break;
		case TYPE::DENSITY:
			if (texDirty) SetTex();
			UI::Quad(Display::width*0.5f - Display::height*0.45f, Display::height*0.05f, Display::height*0.9f, Display::height*0.9f, tex);
			break;
		default:
			break;
		}
		if (Engine::Button(2, 2, 16, 16, blue()) == MOUSE_RELEASE) {
			drawFull = false;
		}
	}
	else {
		switch (type) {
		case TYPE::LINES:
		case TYPE::ALINES:
			if (valXs.size()) {
				plt::plot(pos.x + 12, y + 2, width - 14, width - 14, &valXs[0], &_valYs[0], valXs.size(), _valYs.size(), UI::font, 10, white(1, 0.8f));
			}
			break;
		case TYPE::DENSITY:
			if (texDirty) SetTex();
			UI::Quad(pos.x + 2, y + 2, width - 4, width - 4, tex);
			break;
		default:
			break;
		}
		if (Engine::Button(pos.x + width - 20, y + 2, 16, 16, white(0)) == MOUSE_RELEASE) {
			drawFull = true;
		}
	}
	y += width;
}

#define RETERR(msg) std::cerr << msg << std::endl; return

void Node_Plot::Execute() {
#ifndef IS_ANSERVER
	if (!inputR[0].first) return;
	auto xid = getval_i(1);
	auto yid = getval_i(2);
	CVar& cv = inputR[0].getconv();
	if (!cv.value) {
		RETERR("Value pointer is empty!");
	}
	auto ds = cv.dimVals.size();
	if (ds > 2) {
		RETERR("Data of 3+ dimensions cannot be plotted!");
	}
	else if (ds == 2 && type == TYPE::ALINES) {
		RETERR("Data of 3+ dimensions cannot be accumulated!");
	}
	else if (ds > 0 && !*(void**)cv.value) return;
	int sz;
	int sz2;
	if (type == TYPE::ALINES) {
		sz = (int)Particles::anim.frameCount;
		sz2 = (cv.type == AN_VARTYPE::LIST)? *cv.dimVals[0] : 1;
		if (sz <= 1) {
			RETERR("Accumulate can only be used when animation data is loaded!");
		}
	}
	else {
		sz = *cv.dimVals[0];
		sz2 = (cv.dimVals.size() > 1)? *cv.dimVals[1] : 1;
	}
	if (!sz || !sz2) {
		RETERR("Size is empty!");
	}
	if (sz > 1) {
		if (type == TYPE::ALINES) {
			xid = -1;
			yid = (cv.type == AN_VARTYPE::LIST)? Clamp(yid, -1, *cv.dimVals[0] - 1) : -1;
		}
		else {
			xid = Clamp(xid, -1, sz2 - 1);
			yid = Clamp(yid, -1, sz2 - 1);
		}
	}
	valXs.resize(sz);
	valYs.resize(sz2);
	_valYs.resize(sz2);
	for (int a = 0; a < sz2; ++a) {
		valYs[a].resize(sz);
		_valYs[a] = &valYs[a][0];
	}
	if (ds == 1 || xid == -1 || type == TYPE::ALINES) {
		if (type < TYPE::DENSITY) {
			for (int i = 0; i < sz; ++i) {
				valXs[i] = (float)i;
			}
		}
	}
	else {
		switch (cv.typeName[6]) {
		case 's':
			for (int i = 0; i < sz; ++i) {
				valXs[i] = (float)(*(short**)cv.value)[i * 2 + xid];
			}
			break;
		case 'i':
			for (int i = 0; i < sz; ++i) {
				valXs[i] = (float)(*(int**)cv.value)[i * 2 + xid];
			}
			break;
		case 'd':
			for (int i = 0; i < sz; ++i) {
				valXs[i] = (float)(*(double**)cv.value)[i * 2 + xid];
			}
			break;
		default:
			valXs.clear();
			RETERR("Unexpected data type " + cv.typeName + "!");
		}
	}
	if (type == TYPE::ALINES) {
		if (ds == 1 && yid == -1) {
			switch (cv.typeName[6]) {
#define cs(_c, _t) \
			case _c:\
				for (int i = 0; i < sz2; ++i) {\
					valYs[i][Particles::anim.currentFrame] = (float)(*(_t**)cv.value)[i];\
				} break
				cs('s', short);
				cs('i', int);
				cs('d', double);
			default:
				valXs.clear();
				RETERR("Unexpected data type " + cv.typeName + "!");
#undef cs
			}
		}
		else {
			valYs.resize(1);
			_valYs.resize(1);
			if (!ds) {
				switch (cv.typeName[0]) {
#define cs(_c, _t) \
				case _c:\
					valYs[0][Particles::anim.currentFrame] = (float)(*(_t*)cv.value);\
					break
					cs('s', short);
					cs('i', int);
					cs('d', double);
				default:
					valXs.clear();
					RETERR("Unexpected data type " + cv.typeName + "!");
#undef cs
				}
			}
			else {
				switch (cv.typeName[6]) {
#define cs(_c, _t) \
				case _c:\
					valYs[0][Particles::anim.currentFrame] = (float)(*(_t**)cv.value)[yid];\
					break
					cs('s', short);
					cs('i', int);
					cs('d', double);
				default:
					valXs.clear();
					RETERR("Unexpected data type " + cv.typeName + "!");
#undef cs
				}
			}
		}
	}
	else {
		if (ds == 2 && yid == -1) {
			for (int j = 0; j < sz2; ++j) {
				switch (cv.typeName[6]) {
#define cs(_c, _t) \
				case _c:\
					for (int i = 0; i < sz; ++i) {\
						valYs[j][i] = (float)(*(_t**)cv.value)[i*sz2 + j];\
					} break
					cs('s', short);
					cs('i', int);
					cs('d', double);
				default:
					valXs.clear();
					RETERR("Unexpected data type " + cv.typeName + "!");
#undef cs
				}
			}
		}
		else {
			valYs.resize(1);
			_valYs.resize(1);
			int j = (ds == 1) ? 0 : yid;
			switch (cv.typeName[6]) {
#define cs(_c, _t) \
			case _c:\
				for (int i = 0; i < sz; ++i) {\
					valYs[0][i] = (float)(*(_t**)cv.value)[i*sz2 + j];\
				} break
				cs('s', short);
				cs('i', int);
				cs('d', double);
			default:
				valXs.clear();
				RETERR("Unexpected data type " + cv.typeName + "!");
#undef cs
			}
		}
		texDirty = true;
	}
#endif
}

void Node_Plot::SetTex() {
	_w = valYs.size();
	_h = valYs[0].size();
	float mn;
	float mx = mn = valYs[0][0];
	for (auto& ys : valYs) {
		for (auto& y : ys) {
			mn = std::min(mn, y);
			mx = std::max(mx, y);
		}
	}
	std::vector<float> vals(_w * _h * 3);
#pragma omp parallel for
	for (int a = 0; a < _w; ++a) {
		for (int b = 0; b < _h; ++b) {
			const int i = a*_h*3 + b*3;
			const float v = valYs[a][b];
			if (v != 0) {
				const float hue = InverseLerp(mn, mx, v) * 4;
				vals[i] = 1 - Clamp(std::abs(hue - 4) - 1, 0.f, 1.f);
				vals[i + 1] = 1 - Clamp(std::abs(hue - 2) - 1, 0.f, 1.f);
				vals[i + 2] = Clamp(std::abs(hue - 3) - 1, 0.f, 1.f);
			}
			else {
				vals[i] = vals[i + 1] = vals[i + 2] = 1;
			}
		}
	}
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _w, _h, 0, GL_RGB, GL_FLOAT, vals.data());
	SetTexParams<>(0, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	texDirty = false;
}

void Node_Plot::LoadOut(const std::string& path) {
	Execute();
}

void Node_Plot::OnConn(bool o, int i) {
	if (i == 0) {
		auto& cv = inputR[0].getconv();
		auto sz = cv.dimVals.size();
		if (type == TYPE::ALINES && sz > 1) {
			Debug::Warning("Node::Plot", "Data of 2+ dimensions cannot be accumulated!");
		}
		else if (sz == 1) useids = false;
		else if (sz == 2) {
			auto v = *cv.dimVals[1];
			useids = (v > 1);
		}
		else {
			Debug::Warning("Node::Plot", "Data of 3+ dimensions cannot be plotted!");
		}
		//inputR[1].use = inputR[2].use = useids;
	}
}

void Node_Plot::Save(XmlNode* n) {
	n->addchild("type", std::to_string((int)type));
}

void Node_Plot::Load(XmlNode* n2) {
	for (auto& n : n2->children) {
		if (n.name == "type") {
			type = (TYPE)TryParse(n.value, 0);
			script->invars[0].second = (type == TYPE::ALINES)? "*" : "list(**)";
		}
	}
}