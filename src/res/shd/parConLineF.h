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

#pragma once
namespace glsl {
    const char* parConLineF = R"(
flat in uint v2f_id1;
flat in uint v2f_id2;
in float v2f_u;

uniform uint id2;
uniform usamplerBuffer id2col;
uniform sampler2D colList;
uniform vec4 gradCols[3];
uniform int colUseGrad;
uniform int usegrad;
uniform vec4 onecol;

layout (location=0) out vec4 outColor;
layout (location=1) out uvec4 outId;
layout (location=2) out vec4 outNormal;

vec3 gradfill(float f) {
	return (gradCols[0].rgb * clamp(2 - 4 * f, 0, 1)
		+ gradCols[1].rgb * clamp(2 - abs(2 - 4 * f), 0, 1)
		+ gradCols[2].rgb * clamp(4 * f - 2, 0, 1));
}

void SetColor(int id1, int id2, float f) {
    int cd1 = int(texelFetch(id2col, id1-1).r);
    int cd2 = int(texelFetch(id2col, id2-1).r);
    vec3 cl1 = (colUseGrad == 1)? gradfill(cd1 / 65535.0f)
        : texture(colList, vec2((mod(cd1, 16) + 0.5) / 16.0, ((cd1 / 16) + 0.5) / 16.0)).rgb;
    vec3 cl2 = (colUseGrad == 1)? gradfill(cd2 / 65535.0f)
        : texture(colList, vec2((mod(cd2, 16) + 0.5) / 16.0, ((cd2 / 16) + 0.5) / 16.0)).rgb;
    outColor.rgb = mix(mix(cl1, cl2, f), onecol.rgb, onecol.a);
    outColor.a = 1;
}

void main()
{
    SetColor(int(v2f_id1), int(v2f_id2), mix(round(v2f_u), v2f_u, usegrad));
    outNormal = vec4(0, 0, 0, 0);
}
)";
}