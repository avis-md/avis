#include "Protein.h"
#include "utils/rawvector.h"

//const byte signature[] = { 2, 'H', 0, 'C', 2, 'H', 0, 'C', 1, 'O', 0 };

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

//pattern = id, number[pattern]

//chain: HAHBOCBCAN
*/
/*
bool DoSearch (const std::vector<uint>& cs, byte** pattern, uint*& ids, const std::vector<std::vector<uint>>& conns) {
    auto s = cs.size();
    uint* ido = ids;
    for (byte b = 0, bc = *((*pattern)++); b < bc; b++) {
		char n2 = *((*pattern)++);
        bool ok = false;
		for (uint j = 0; j < s; j++) {
			if (Particles::particles_Name[cs[j] * PAR_MAX_NAME_LEN] == n2) {
				if (DoSearch(conns[j], pattern, ids, conns)) {
                    *(ids++) = j;
                    j = s;
                    ok = true;
                }
            }
		}
		if (!ok) {
            ids = ido;
            return false;
	    }
    }
    return true;
}
*/
bool _Has(const std::vector<uint>& c, char _c) {
    for (auto& a : c) {
        if (Particles::particles_Name[a * PAR_MAX_NAME_LEN] == _c)
            return true;
    }
    return false;
}

#define _FOR(conn, c, i) for (auto& i : conn) { \
    if (Particles::particles_Name[i * PAR_MAX_NAME_LEN] == c)


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
                    p->chain = (uint*)std::malloc(sizeof(uint) * 3);
                    ch = p->chain;
                    p->cnt = 1;
                }
                else {
                    p->cnt++;
                    p->chain = (uint*)std::realloc(p->chain, sizeof(uint) * 3 * p->cnt);
                    ch = p->chain + (3 * (p->cnt-1));
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
                                if (_Has(conns[b - rs.offset], 'H')) {
                                    _FOR(conns[b - rs.offset], 'C', c) {
                                        if (_Has(conns[c - rs.offset], 'O')) {
                                            ch[0] = a + rs.offset;
                                            ch[1] = b;
                                            ch[2] = c;
                                            goto found;
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
                if (p) p = 0;
            }
        }
    }
}