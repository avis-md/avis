#include "Protein.h"
#include "md/ParMenu.h"
#include "utils/rawvector.h"
#include "utils/spline.h"
#include "utils/solidify.h"
#include "vis/pargraphics.h"
#include "utils/solidify.h"

//const byte signature[] = { 2, 'H', 0, 'C', 2, 'H', 0, 'C', 1, 'O', 0 };

byte Protein::proCnt = 0;
Protein* Protein::pros;

Shader* Protein::shad;
GLint Protein::shadLocs[];

byte AminoAcidType (const char* nm) {
	uint32_t i = *(uint32_t*)nm;
	const uint32_t mask = 0x00ffffff;
	for (byte b = 0; b < AMINO_ACID_LEN; b++) {
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
        if (Particles::particles_Name[a * PAR_MAX_NAME_LEN] == _c)
            return i;
        i++;
    }
    return ~0U;
}

#define _FOR(conn, c, i) for (auto& i : conn) { \
    if (Particles::particles_Name[i * PAR_MAX_NAME_LEN] == c)

byte _CntOf(const std::vector<uint>& c, char _c) {
    byte i = 0;
    _FOR(c, _c, a) {
        i++;
    }}
    return i;
}

void Protein::Init() {
	shad = new Shader(IO::GetText(IO::path + "/prochainV.txt"), IO::GetText(IO::path + "/prochainF.txt"));
    shadLocs[0] = glGetUniformLocation(shad->pointer, "_MV");
    shadLocs[1] = glGetUniformLocation(shad->pointer, "_P");
    shadLocs[2] = glGetUniformLocation(shad->pointer, "poss");
    shadLocs[3] = glGetUniformLocation(shad->pointer, "ids");
    shadLocs[4] = glGetUniformLocation(shad->pointer, "chainSz");
    shadLocs[5] = glGetUniformLocation(shad->pointer, "chainReso");
    shadLocs[6] = glGetUniformLocation(shad->pointer, "loopReso");
}

void Protein::Refresh() {
    if (pros) std::free(pros);
    auto proVec = rawvector<Protein, byte>(pros, proCnt);
    Protein* p = 0;
    uint* ch;
    for (uint i = 0; i < Particles::residueListSz; i++) {
        auto& rl = Particles::residueLists[i];
        for (uint j = 0; j < rl.residueSz; j++) {
            auto& rs = rl.residues[j];
            if (rs.type != 255) {
                if (!p) {
                    proVec.push(Protein());
                    p = pros + (proCnt-1);
                    p->first = Int2(i, j);
                    p->chain = (uint*)std::malloc(sizeof(uint) * 6);
                    ch = p->chain;
                    p->cnt = 1;
                }
                else {
                    p->cnt++;
                    p->chain = (uint*)std::realloc(p->chain, sizeof(uint) * 6 * p->cnt);
                    ch = p->chain + (6 * (p->cnt-1));
                }

                auto conns = std::vector<std::vector<uint>>(rs.cnt);

                for (uint a = 0; a < rs.cnt; a++) conns[a] = std::vector<uint>();
                
                for (uint k = 0; k < rs.cnt_b; k++) {
                    auto& cn = Particles::particles_Conn[rs.offset_b + k];
                    if (cn[1] >= rs.offset) {
                        conns[cn[0] - rs.offset].push_back(cn[1]);
                        conns[cn[1] - rs.offset].push_back(cn[0]);
                    }
                }
                
                for (uint a = 0; a < rs.cnt; a++) {
                    if (Particles::particles_Name[(a + rs.offset) * PAR_MAX_NAME_LEN] == 'N') {
                        //if (_Has(conns[a], 'H')) {
                            _FOR(conns[a], 'C', b) {
                                auto& cb = conns[b - rs.offset];
                                uint b2 = _Has(cb, 'H');
                                if (b2 < ~0U) {
                                    _FOR(cb, 'C', c) {
                                        auto& cc = conns[c - rs.offset];
                                        uint c2 = _Has(cc, 'O');
                                        if (c2 < ~0U) {
                                            if (_CntOf(cc, 'C') == 1) {
                                                ch[0] = a + rs.offset;
                                                for (auto ca : conns[a]) {
                                                    if (ca >= rs.offset && ca != b) {
                                                        ch[1] = ca;
                                                        break;
                                                    }
                                                }
                                                ch[2] = b;
                                                ch[3] = b2;
                                                ch[4] = c;
                                                ch[5] = c2;
                                                goto found;
                                            }
                                        }
                                    }}
                                }
                            }}
                        //}
                    }
                }
                Debug::Error("Protein", "Cannot find amino chain!");
                found:;
            }
            else {
                if (p) {
                    /*
                    std::vector<Vec3> pts(p->cnt * 3);
                    for (uint i = 0; i < p->cnt * 3; i++)
                        pts[i] = Particles::particles_Pos[p->chain[i]];
                    std::vector<Vec3> res((p->cnt * 3 - 1) * 12 + 1);
                    Spline::ToSpline(&pts[0], p->cnt * 3, 12, &res[0]);
                    p->mesh = Solidify::Do(&res[0], (p->cnt * 3 - 1) * 12 + 1, 0.02f, 12);

                    auto obj = SceneObject::New("Protein");
                    Scene::active->AddObject(obj);
                    obj->AddComponent<MeshFilter>()->mesh(p->mesh);
                    obj->AddComponent<MeshRenderer>()->materials[0](mat);
                    */

                    //p->chainTangents = new Vec3[p->cnt * 3];
                    //Solidify::GetTangents(p->chain, p->cnt, p->chainTangents);
                    
                    glGenBuffers(1, &p->idBuf);
                    glBindBuffer(GL_ARRAY_BUFFER, p->idBuf);
                    glBufferData(GL_ARRAY_BUFFER, 6 * p->cnt * sizeof(uint), p->chain, GL_STATIC_DRAW);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);

                    glGenTextures(1, &p->idBufTex);
                    glBindTexture(GL_TEXTURE_BUFFER, p->idBufTex);
                    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, p->idBuf);
                    glBindTexture(GL_TEXTURE_BUFFER, 0);

                    p = 0;
                }
            }
        }
    }
}

void Protein::Draw() {
    for (byte b = 0; b < proCnt; b++) {
        auto& p = pros[b];

        auto _mv = MVP::modelview();
        auto _p = MVP::projection();
        
        glUseProgram(shad->pointer);
        glUniformMatrix4fv(shadLocs[0], 1, GL_FALSE, glm::value_ptr(_mv));
        glUniformMatrix4fv(shadLocs[1], 1, GL_FALSE, glm::value_ptr(_p));
        glUniform1i(shadLocs[2], 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_BUFFER, Particles::posTexBuffer);
        glUniform1i(shadLocs[3], 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_BUFFER, p.idBufTex);
        glUniform1i(shadLocs[4], p.cnt * 3);
        glUniform1i(shadLocs[5], p.chainReso);
        glUniform1i(shadLocs[6], p.loopReso);

        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glBindVertexArray(ParGraphics::emptyVao);
        glDrawArrays(GL_TRIANGLES, 0, 6 * (p.cnt * 3 - 1) * p.chainReso * p.loopReso);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        //*/
        glBindVertexArray(0);
        glUseProgram(0);
    }
}

void Protein::DrawMenu() {
    //
    auto& expandPos = ParMenu::expandPos;
	auto& font = ParMenu::font;
	
    auto cr = pros->chainReso;
    auto lr = pros->loopReso;
    
    UI::Label(expandPos - 148, 3, 12, "Curve Resolution", font, white());
	cr = (byte)Engine::DrawSliderFill(expandPos - 80, 2, 78, 16, 4, 20, cr, white(1, 0.5f), white());
    UI::Label(expandPos - 147, 20, 12, "Bevel Resolution", font, white());
	lr = (byte)Engine::DrawSliderFill(expandPos - 80, 19, 78, 16, 4, 20, lr, white(1, 0.5f), white());

    if (cr != pros->chainReso || lr != pros->loopReso) {
        Scene::dirty = true;

        pros->chainReso = cr;
        pros->loopReso = lr;
    }
}