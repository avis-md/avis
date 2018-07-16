#include "Engine.h"
#include "Particles.h"

const byte AMINO_ACID_LEN = 20;
const string AMINO_ACIDS =
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
    Protein() : cnt(0), chainReso(5), loopReso(12), expanded(false), visible(true), drawGrad(false) {}

    uint cnt;
    Int2 first;
    uint* chain; //N[]C[]C[], where [] is any non-chain bond
    pMesh mesh;
    GLuint idBuf, idBufTex;
    byte chainReso, loopReso;
	bool expanded, visible;
	bool drawGrad;
	Vec4 tint;

	void ApplyChain();

    static byte proCnt;
    static Protein* pros;

    static void Init(), Clear(), Refresh();
    static void Draw(), Recolor(), DrawMenu(float off);

    static GLuint shad, colShad;
    static GLint shadLocs[10], colShadLocs[6];
};