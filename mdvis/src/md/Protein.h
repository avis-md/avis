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
    Protein() : cnt(0), chainReso(5), loopReso(12) {}

    uint cnt;
    Int2 first;
    uint* chain;
    pMesh mesh;
    GLuint idBuf, idBufTex;
    byte chainReso, loopReso;
    
    static byte proCnt;
    static Protein* pros;

    static void Init();
    static void Refresh();
    static void Draw();

    static Shader* shad;
    static GLint shadLocs[7];
};