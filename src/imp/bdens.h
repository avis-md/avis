#pragma once
#include "importer_info.h"
#include <vector>
#include <string>

//minimal binary density data
/*
 byte  data     type       desc
 0~1   sizex    ushort     x dim length
 2~3   sizey    ushort     y dim length
 4~5   sizez    ushort     z dim length
 6     datatype char       D:d64, F:f32, S:i16, I:i32, L:i64
 7~?   density  datatype[] actual data
*/
class BDens {
public:
	static bool Read(ParInfo* info);
};
