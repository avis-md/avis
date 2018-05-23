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
    Protein() : cnt(0) {}

    uint cnt;
    Int2 first;
    uint* chain;
    pMesh mesh;

    static byte proCnt;
    static Protein* pros;

    static void Init();
    static void Refresh();

    static Shader* shad;
    static pMaterial mat;
};