#include "anops.h"
#include "ui/ui_ext.h"
#include "ui/icons.h"
#include "libssh2_sftp.h"

bool AnOps::expanded = true;
byte AnOps::connectStatus;
float AnOps::expandPos;

bool AnOps::remote = false;
std::string AnOps::user = "username", AnOps::ip = "host", AnOps::pw;
ushort AnOps::port = 22;
std::string AnOps::path = "path";
std::string AnOps::message = "disconnected";

std::thread* AnOps::conThread;

SSH AnOps::ssh;

void AnOps::Draw() {
	const float expos = Display::width - expandPos;
	const float w = 180;
	UI2::BackQuad(expos, 0, expandPos, Display::height - 18.f);
	if (expanded) {
		UI::Label(expos + 1, 1, 12, "Options", white());
		
		float off = 20;
		
		UI2::Toggle(expos + 2, off, w - 4, "Invert Run All", AnWeb::invertRun); off += 17;
		UI2::Toggle(expos + 2, off, w - 4, "Run On Seek", AnWeb::runOnFrame);

		off = std::max(off, Display::height - 68.f - 18.f);
		UI::Label(expos + 2, off, 12, "Status", white());
		UI::Label(expos + 4, off + 17, 12, "C++", white());
		UI::Quad(expos + 164, off + 18, 14, 14, (remote? AnWeb::hasC_s : AnWeb::hasC) ? green() : red());
		UI::Label(expos + 4, off + 34, 12, "Python", white());
		UI::Quad(expos + 164, off + 34, 14, 14, (remote ? AnWeb::hasPy_s : AnWeb::hasPy) ? green() : red());
		UI::Label(expos + 4, off + 51, 12, "Fortran", white());
		UI::Quad(expos + 164, off + 51, 14, 14, (remote ? AnWeb::hasFt_s : AnWeb::hasFt) ? green() : red());
		
		UI2::BackQuad(expos - 16.f, Display::height - 34.f, 16.f, 16.f);
		if ((!UI::editingText && (Input::KeyUp(Key_O))) || Engine::Button(expos - 16.f, Display::height - 34.f, 16.f, 16.f, Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE)
			expanded = false;
		expandPos = std::min(expandPos + 1500 * Time::delta, w);
	}
	else {
		if ((!UI::editingText && (Input::KeyUp(Key_O))) || Engine::Button(expos - 110.f, Display::height - 34.f, 110.f, 16.f, white(0.9f, 0.15f), white(1, 0.15f), white(1, 0.05f)) == MOUSE_RELEASE)
			expanded = true;
		UI::Texture(expos - 110.f, Display::height - 34.f, 16.f, 16.f, Icons::expand);
		UI::Label(expos - 92.f, Display::height - 33.f, 12.f, "Options (O)", white());
		expandPos = std::max(expandPos - 1500 * Time::delta, 2.f);
	}
}

void AnOps::Connect() {
	message = "connecting";
	connectStatus = 1;
	SSHConfig config;
	config.ip = ip;
	config.port = port;
	config.user = user;
	config.auth = SSH_Auth::PASSWORD;
	config.pw = pw;
	ssh = SSH::Connect(config);
	connectStatus = ssh.ok? 2 : 0;
	if (!connectStatus) {
		message = "failed to connect";
		return;
	}
	ssh.EnableSFTP();

	message = "initializing files";
	if (!ssh.HasFile(path + "/mdvis_ansrv")) {
		ssh.MkDir(path);
		ssh.SendFile(IO::path + "bin/mdvis_ansrv", path + "/mdvis_ansrv");
		ssh.Write("chmod +rx " + path + "/mdvis_ansrv");
	}
	ssh.Write("cd " + path);
	ssh.Write("mkdir nodes; rm -f nodes/*; mkdir -p ser/in; mkdir ser/out");

	message = "scanning runtime";

	
	std::string pth;
	AnWeb::hasC_s = ssh.HasCmd("g++", pth);
	Debug::Message("AnOps", "g++ location: " + pth);
	AnWeb::hasPy_s = ssh.HasCmd("python", pth);
	Debug::Message("AnOps", "python location: " + pth);
	AnWeb::hasFt_s = ssh.HasCmd("gfortran", pth);
	Debug::Message("AnOps", "gfortran location: " + pth);

	SendNodes(true);
	
	connectStatus = 255;
	message = "connected";
}

void AnOps::Disconnect() {
	ssh.Disconnect();
	connectStatus = 0;
	AnWeb::hasC_s = false;
	AnWeb::hasPy_s = false;
	AnWeb::hasFt_s = false;

	message = "disconnected";
}

void AnOps::SendNodes(bool cp) {
	message = "syncing nodes";
	DoSendNodes(IO::path + "nodes/", "nodes/");
	message = "compiling";
	ssh.Write("./mdvis_ansrv -p -c 1; echo '<''>'");
	ssh.WaitFor("<>", 500);
}

void AnOps::DoSendNodes(std::string p, std::string rp) {
	auto fs = IO::GetFiles(p, ".cpp");
	auto fs2 = IO::GetFiles(p, ".py");
	fs.insert(fs.end(), fs2.begin(), fs2.end());
	if (!!fs.size()) {
		ssh.Write("mkdir -p " + rp);
		for (auto& f : fs) {
			ssh.SendFile(p + f, path + "/" + rp + f);
			ssh.Write("chmod +r " + path + "/" + rp + f);
		}
	}
	IO::GetFolders(p, &fs);
	for (auto& f : fs) {
		DoSendNodes(p + f + "/", rp + f + "/");
	}
}

void AnOps::SendIn() {
	message = "syncing input";
	auto ins = IO::GetFiles(IO::path + "nodes/__tmp__/in/");
	for (auto& i : ins) {
		ssh.SendFile(IO::path + "nodes/__tmp__/in/" + i, path + "/ser/in/" + i);
	}
	for (auto& i : ins) {
		ssh.Write("chmod +r ser/in/" + i);
	}
}

void AnOps::RecvOut() {
	message = "syncing output";
	if (!IO::HasDirectory(IO::path + "nodes/__tmp__/out/")) IO::MakeDirectory(IO::path + "nodes/__tmp__/out/");
	auto fls = ssh.ListFiles(path + "/ser/out/");
	for (auto& f : fls) {
		if (f[0] != '.')
			ssh.GetFile(path + "/ser/out/" + f, IO::path + "nodes/__tmp__/out/" + f);
	}
}