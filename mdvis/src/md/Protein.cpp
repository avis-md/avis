#include "Protein.h"
#include "md/parmenu.h"
#include "md/parloader.h"
#include "utils/spline.h"
#include "utils/solidify.h"
#include "vis/pargraphics.h"
#include "ui/icons.h"
#include "ui/ui_ext.h"
#include "res/shddata.h"
#include "utils/glext.h"

//const byte signature[] = { 2, 'H', 0, 'C', 2, 'H', 0, 'C', 1, 'O', 0 };

byte Protein::proCnt = 0;
std::vector<Protein> Protein::pros;

Shader Protein::shad;

Protein::Protein() : cnt(0), chainReso(8), loopReso(20), expanded(false), visible(true), drawGrad(false) {}

byte AminoAcidType (const char* nm) {
	uint32_t i = *(uint32_t*)nm;
	const uint32_t mask = 0x00ffffff;
	for (byte b = 0; b < AMINO_ACID_LEN; ++b) {
		uint32_t j = *(uint32_t*)(&AMINO_ACIDS[0] + 3*b);
		if ((i&mask)==(j&mask)) return b;
	}
	return 255;
}

/*
N
|
C-H
|
C=O
*/

uint _Has(const std::vector<uint>& c, char _c) {
	uint i = 0;
	for (auto& a : c) {
		if (Particles::names[a * PAR_MAX_NAME_LEN] == _c)
			return i;
		i++;
	}
	return ~0U;
}

#define _FOR(conn, c, i) for (auto& i : conn) { \
	if (Particles::names[i * PAR_MAX_NAME_LEN] == c)

byte _CntOf(const std::vector<uint>& c, char _c) {
	byte i = 0;
	_FOR(c, _c, a) {
		i++;
	}}
	return i;
}

void Protein::Init() {
	shad = Shader::FromVF(IO::GetText(IO::path + "prochainV.txt"), IO::GetText(IO::path + "prochainF.txt"));
#define LC(nm) shad.AddUniform(#nm)
	int i = 0;
	LC(_MV);
	LC(_P);
	LC(poss);
	LC(ids);
	LC(chainSz);
	LC(chainReso);
	LC(loopReso);
	LC(proId);
	LC(beziery);
	LC(col);
	LC(usegrad);
	LC(gradcols);
#undef LC
}

void Protein::Clear() {
	pros.clear();
	proCnt = 0;
}

bool Protein::Refresh() {
	Clear();
	Protein* p = 0;
	uint* ch = 0;
	bool isn = false;
	for (uint i = 0; i < Particles::residueListSz; ++i) {
		auto& rl = Particles::residueLists[i];
		for (uint j = 0; j < rl.residueSz; ++j) {
			auto& rs = rl.residues[j];
			if (rs.type != 255) {
				if (!p) {
					Debug::Message("Protein", "Amino chain start " + std::to_string(i)
						+ "(" + rl.name + ")");
					pros.push_back(Protein());
					p = &pros.back();
					p->first = Int2(i, j);
					//p->chain = (uint*)std::malloc(sizeof(uint) * 6);
					p->chain.resize(6);
					ch = p->chain.data();
					p->cnt = 1;
					isn = true;
				}
				else
					isn = false;

				auto conns = std::vector<std::vector<uint>>(rs.cnt);

				for (uint a = 0; a < rs.cnt; a++) conns[a] = std::vector<uint>();

				bool hascon = isn;
				uint ls = ch[4];
				for (uint k = 0; k < rs.cnt_b; ++k) {
					auto& cn = Particles::conns.ids[rs.offset_b + k];
					if (cn[1] >= (int)rs.offset) {
						conns[cn[0] - rs.offset].push_back(cn[1]);
						conns[cn[1] - rs.offset].push_back(cn[0]);
					}
					else if (cn[1] == ls)
						hascon = true;
				}

				if (!isn) {
					if (hascon) {
						p->cnt++;
						//p->chain = (uint*)std::realloc(p->chain, sizeof(uint) * 6 * p->cnt);
						p->chain.resize(6 * p->cnt);
						ch = &p->chain[(6 * (p->cnt - 1))];
					}
					else {
						p->ApplyChain();
						pros.push_back(Protein());
						p = &pros.back();
						p->first = Int2(i, j);
						//p->chain = (uint*)std::malloc(sizeof(uint) * 6);
						p->chain.resize(6);
						ch = p->chain.data();
						p->cnt = 1;
						isn = true;
					}
				}
				
				int mxf = 0;
				std::string msg2 = "Amino chain not found for residue ";
				msg2 += std::string(&rl.name[0]);
				msg2 += " (" + rs.name + ")\n";
				for (uint a = 0; a < rs.cnt; ++a) {
					if (Particles::names[(a + rs.offset) * PAR_MAX_NAME_LEN] == 'N') {
						mxf = 1;
						//if (_Has(conns[a], 'H')) {
							_FOR(conns[a], 'C', b) {
								mxf = 2;
								auto& cb = conns[b - rs.offset];
								//uint b2 = _Has(cb, 'H');
								//if (b2 < ~0U) {
									_FOR(cb, 'C', c) {
										mxf = 3;
										auto& cc = conns[c - rs.offset];
										uint c2 = _Has(cc, 'O');
										if (c2 < ~0U) {
											mxf = 4;
											if (_CntOf(cc, 'C') == 1) {
												ch[0] = a + rs.offset;
												ch[2] = b;
												//ch[3] = b2;
												ch[4] = c;
												//ch[5] = c2;
												ch[1] = ch[3] = ch[5] = 0;
												goto found;
											}
										}
									}}
								//}
							}}
						//}
					}
				}
				//Debug::Error("Protein", "Cannot find amino chain!");
				//ParLoader::fault = true;
				if (mxf == 0)
					msg2 += "Cannot find N atom";
				else if (mxf == 1)
					msg2 += "Cannot find 1st C atom";
				else if (mxf == 2)
					msg2 += "Cannot find 2nd C atom";
				else if (mxf == 3)
					msg2 += "Cannot find O atom attached to 2nd C atom";
				else if (mxf == 4)
					msg2 += "2nd C atom has more than 1 C atom attached";
				msg2 += "\nChain signature is N-C1-C2=O";
				VisSystem::SetMsg("Protein amino chain error!", 2, msg2);
				Debug::Warning("Protein", msg2);
				Debug::Warning("Protein", "Aborting from error");
				//Particles::Clear();
				return false;
				found:;
			}
			else {
				if (p) {
					p->ApplyChain();
					p = nullptr;
					Debug::Message("Protein", "Amino chain end "+ std::to_string(i-1)
						+ "(" + Particles::residueLists[i-1].name + ")");
				}
			}
		}
	}
	if (p) {
		p->ApplyChain();
		p = nullptr;
		Debug::Message("Protein", "Amino chain end "+ std::to_string(Particles::residueLists.size()-1)
			+ "(" + Particles::residueLists.back().name + ")");
	}

	if (!!(proCnt = (byte)pros.size())) {
		for (byte b = 0; b < proCnt; ++b) {
			pros[b].tint = Color::HueBaseCol(1 - (float(b) / proCnt));
		}
	}
	return true;
}

void Protein::ApplyChain() {
	for (uint i = 0; i < cnt * 3; ++i) {
		auto& p1 = Particles::poss[chain[i * 2]];
		for (uint j = 0; j < cnt * 3; ++j) {
			if (j < (i - 3) || j >(i + 3)) {
				auto& p2 = Particles::poss[chain[j * 2]];
				if (glm::length2(p1 - p2) < 33) {
					chain[i * 2 + 1] = j;
					break;
				}
			}
		}
	}

	glGenBuffers(1, &idBuf);
	SetGLBuf(idBuf, chain.data(), 6 * cnt);

	glGenTextures(1, &idBufTex);
	glBindTexture(GL_TEXTURE_BUFFER, idBufTex);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, idBuf);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
}

void Protein::Draw() {
	auto _mv = MVP::modelview();
	auto _p = MVP::projection();
	glEnable(GL_CULL_FACE);
	for (byte b = 0; b < proCnt; ++b) {
		auto& p = pros[b];
		if (!p.visible) continue;
		
		glUseProgram(shad);
		glUniformMatrix4fv(shad.Loc(0), 1, GL_FALSE, glm::value_ptr(_mv));
		glUniformMatrix4fv(shad.Loc(1), 1, GL_FALSE, glm::value_ptr(_p));
		glUniform1i(shad.Loc(2), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER, Particles::posTexBuffer);
		glUniform1i(shad.Loc(3), 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_BUFFER, p.idBufTex);
		glUniform1i(shad.Loc(4), p.cnt * 3);
		glUniform1i(shad.Loc(5), p.chainReso);
		glUniform1i(shad.Loc(6), p.loopReso);
		glUniform1i(shad.Loc(7), b);
		glUniform1f(shad.Loc(8), p.smoothness);
		glUniform3f(shad.Loc(9), p.tint.r, p.tint.g, p.tint.b);
		glUniform1i(shad.Loc(10), p.drawGrad);
		glUniform4fv(shad.Loc(11), 3, &ParGraphics::gradCols[0][0]);

		glBindVertexArray(Camera::emptyVao);
		glDrawArrays(GL_TRIANGLES, 0, 6 * (p.cnt * 3 - 1) * p.chainReso * p.loopReso);
		glBindVertexArray(0);
		glUseProgram(0);
	}
	glDisable(GL_CULL_FACE);
}

void Protein::DrawMenu(float off) {
	auto exp = ParMenu::expandPos;
	if (Engine::Button(2, off, 16, 16, Icons::select, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) {
	}
	if (Engine::Button(19, off, 16, 16, Icons::deselect, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) {
	}
	if (Engine::Button(36, off, 16, 16, Icons::flipselect, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) {
	}
	off += 17;
	for (uint i = 0; i < proCnt; ++i) {
		Protein& p = pros[i];
		UI::Quad(exp - 148, off, 146, 16, white(1, 0.3f));
		if (Engine::Button(exp - 148, off, 16, 16, p.expanded ? Icons::expand : Icons::collapse) == MOUSE_RELEASE) {
			p.expanded = !p.expanded;
		}
		UI::Label(exp - 132, off, 12, std::to_string(i + 1), white(p.visible ? 1 : 0.5f));
		if (Engine::Button(exp - 130, off, 96, 16) == MOUSE_RELEASE) {
			
		}
		if (Engine::Button(exp - 50, off, 16, 16, Icons::pro_col, p.tint * (p.drawGrad ? 0.4f : 0.8f)) == MOUSE_RELEASE) {
			if (p.drawGrad) p.drawGrad = false;
			else {
				Popups::type = POPUP_TYPE::COLORPICK;
				Popups::pos = Vec2(exp - 50, off + 17);
				Popups::data = &p.tint;
			}
		}
		if (Engine::Button(exp - 34, off, 16, 16, Icons::pro_grad, white(p.drawGrad ? 0.8f : 0.4f)) == MOUSE_RELEASE) {
			p.drawGrad = true;
		}
		if (Engine::Button(exp - 18, off, 16, 16, p.visible ? Icons::visible : Icons::hidden) == MOUSE_RELEASE) {
			p.visible = !p.visible;
			Scene::dirty = true;
		}
		static bool _drawGrad;
		if (_drawGrad != p.drawGrad) {
			_drawGrad = p.drawGrad;
			Scene::dirty = true;
		}
		static Vec4 _tint;
		if (_tint != p.tint) {
			_tint = p.tint;
			Scene::dirty = true;
		}
		off += 17;
		if (p.expanded) {
			uint f2 = (uint)p.first.y;
			uint a = 0;
			for (uint f1 = (uint)p.first.x; f1 < Particles::residueListSz; ++f1) {
				auto& rli = Particles::residueLists[f1];
				while (f2 < rli.residueSz) {
					UI::Quad(exp - 143, off, 141, 16, white(1, 0.4f));
					UI::Label(exp - 141, off, 12, rli.name, white());
					f2++;
					off += 17;
					a++;
					if (a == p.cnt) goto pend;
				}
				f2 = 0;
			}
		pend:;
		}
	}
}
