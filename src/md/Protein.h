// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

#include "Engine.h"
#include "particles.h"

const byte AMINO_ACID_LEN = 20;
const std::string AMINO_ACIDS =
"ALA\
ARG\
ASN\
ASP\
CYS\
GLN\
GLU\
GLY\
HIS\
ILE\
LEU\
LYS\
MET\
PHE\
PRO\
SER\
THR\
TRP\
TYR\
VAL";

byte AminoAcidType(const char* nm);

class Protein {
public:
    Protein();

    uint cnt;
    Int2 first;
    //uint* chain; //N[]C[]C[], where [] is any non-chain bond
    std::vector<uint> chain;
    pMesh mesh;
    GLuint idBuf, idBufTex;
    byte chainReso, loopReso;
    float smoothness;
	bool expanded, visible;
	bool drawGrad;
	Vec4 tint;

	void ApplyChain();

    static byte proCnt;
    static std::vector<Protein> pros;

    static void Init(), Clear();
    static bool Refresh();
    static void Draw(), DrawMenu(float off);

    static Shader shad;
};