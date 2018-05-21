#include "Protein.h"
#include "utils/rawvector.h"

byte Protein::proCnt = 0;
Protein* Protein::pros;

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
N-HA
|
CA-HB
|
CB=O

pattern = id, number[pattern]

chain: HAHBOCBCAN
*/

bool DoSearch (const std::vector<uint>& cs, byte* pattern, uint*& ids, const std::vector<std::vector<uint>>& conns) {
	for (byte b = 0; b < *(pattern++); b++) {
		char n2 = *(pattern++);
		for (uint j = 0; j < cs.size(); j++) {
			if (Particles::particles_Name[cs[j] * PAR_MAX_NAME_LEN] == n2) {
				if (!DoSearch(conns[j], pattern, ids, conns)) return false;
				else *(ids++) = j;
			}
		}
		return false;
	}
	return true;
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
                }
                else {
                    p->cnt++;
                    p->chain = (uint*)std::realloc(p->chain, sizeof(uint) * 6 * p->cnt);

                }

                std::vector<std::vector<uint>> conns(rs.cnt);
                for (uint k = 0; k < rs.cnt_b; k++) {
                    auto& cn = Particles::particles_Conn[rs.offset_b + k];
                    if (cn[1] >= rs.cnt) {
                        conns[cn[0] - rs.cnt].push_back(cn[1]);
                        conns[cn[1] - rs.cnt].push_back(cn[0]);
                    }
                }

                for (uint i = 0; i < rs.cnt; i++) {
                    if (Particles::particles_Name[(i + rs.offset) * PAR_MAX_NAME_LEN])
                        if (DoSearch(conns[i], (byte*)signature, ch, conns)) break;
                }
            }
        }
    }
}