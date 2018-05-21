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
    const char* signature = "\x1N\x2H\x0C\x2H\x0C\x1O\x0\x0";

    Protein() : cnt(0) {}

    uint cnt;
    Int2 first;
    uint* chain;

    static byte proCnt;
    static Protein* pros;

    void Refresh();
};