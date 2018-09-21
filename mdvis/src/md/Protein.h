#include "Engine.h"
#include "Particles.h"

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
    uint* chain; //N[]C[]C[], where [] is any non-chain bond
    pMesh mesh;
    GLuint idBuf, idBufTex;
    byte chainReso, loopReso;
    float smoothness;
	bool expanded, visible;
	bool drawGrad;
	Vec4 tint;

	void ApplyChain();

    static byte proCnt;
    static Protein* pros;

    static void Init(), Clear();
    static bool Refresh();
    static void Draw(), Recolor(), DrawMenu(float off);

    static GLuint shad, colShad;
    static GLint shadLocs[10], colShadLocs[6];
};