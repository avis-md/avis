#pragma once

/*
Format: signature, name, type[B / I / F / S], {args},
		description,

  signature must start with [S(ystem) / A(nalysis) / V(isualization)]

Args:
	Bool: value[0 / 1]
	Int: value, min, max, use slider
	Float: value, min, max, use slider
	String: value
	Color: value.r, value.g, value.b, value.a

  if min == max then no range is imposed
*/

namespace config {
	const char* prefs[] = {
		"SDPI", "DPI Scale Factor", "F", "1", "0.2", "5", "N",
		"Scale Factor of the User Interface. Large numbers make the icons larger and vice versa.",

		"SHQUI", "High Quality UI", "B", "1",
		"Blurring of user interface background",

		"SUIBL", "UI Blur Amount", "F", "10", "0.1", "10", "Y",
		"Blur size of user interface background",

		"SOPUI", "UI Opacity", "F", "0.7", "0", "1", "Y",
		"Non-see-through-ness of the user interface",

		"SBKCL", "UI Background Color", "C", "0.1", "0.1", "0.1", "1",
		"Tint color of the user interface",

#ifdef PLATFORM_WIN
		"ACLW", "Additional Compiler Flags", "S", "",
		"Additional flags to pass to the compiler. Include libraries here if necessary.",

		"ALNKW", "Additional Linker Flags", "S", "",
		"Additional flags to pass to the linker. Include libraries here if necessary.",
#else
		"ACL", "Additional Compiler Flags", "S", "",
		"Additional flags to pass to the compiler. Include libraries here if necessary.",
#endif
		"AOMP", "Enable OpenMP", "B", "0",
		"Enables OpenMP when compiling. OpenMP libraries must be separately installed.",

		"VSS", "Scroll Speed", "F", "1", "0.2", "5", "Y",
		"Mouse ScrollWheel Sensitivity",


		nullptr
	};
}