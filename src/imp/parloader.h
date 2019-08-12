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
#include "ChokoLait.h"
#include "utils/dylib.h"
#include "utils/ssh.h"
#define NO_EXPORT_IMP
#include "importer_info.h"

#define SRV_APP 2
#define SRV_APP_STR #SRV_APP

struct ParImporter {
	typedef bool(*loadsig)(ParInfo*);
	typedef bool(*loadfrmsig)(FrmInfo*);
	typedef bool(*loadtrjsig)(TrjInfo*);
	struct Func {
		std::string name;
		enum class FUNC_TYPE {
			CONFIG,
			FRAME,
			TRAJ
		} type;
		std::vector<std::string> exts;
		loadsig func;
		loadfrmsig frmFunc;
		loadtrjsig trjFunc;
	};

	std::string name, sig;
	DyLib* lib;

	std::vector<Func> funcs;
};

class ParLoader {
public:
	static void Init(), Deinit(), Scan();

	static int impId, funcId;
	static ParImporter* customImp;
	static bool loadAsTrj, additive;
	static uint frameskip;
	static int maxframes;
	static bool useConn, useConnPeriodic;
	static bool useConnCache, hasConnCache, oldConnCache, ovwConnCache;
	static std::string connCachePath;

	static bool isSrv, srvusepass;
	static std::string srvuser, srvhost;
	static int srvport;
	static std::string srvkey, srvpass;
	static SSH srv;

	static std::vector<ParImporter> importers;
	static std::vector<std::string> exts;
	
	static bool showDialog, busy, fault, directLoad;
	static std::string requestSig;
	static bool parDirty, trjDirty;
	static float* loadProgress, *loadProgress2;
	static uint16_t* loadFrames;
	static std::string loadName;
	static std::vector<std::string> droppedFiles;

	static bool _showImp;
	static float _impPos, _impScr;

	static void SrvConnect(), SrvDisconnect();
	static void SaveSrvInfo();

	static void ScanFrames(const std::string& first);

	static void DoOpen();
	static void DoOpenAnim();

	static void OpenFrame(uint f, const std::string& path);
	static void OpenFrameNow(uint f, std::string path);

	static void DrawOpenDialog();

	static bool OnDropFile(int i, const char** c);
	static void OnOpenFile(const std::vector<std::string>& files);
	static void FindImpId(bool force = false);
	static uint FindNextOff(std::string path);
};