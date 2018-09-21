#include "anops.h"
#include "ui/icons.h"
#include "libssh2_sftp.h"

bool AnOps::expanded = true;
byte AnOps::connectStatus;
float AnOps::expandPos;

bool AnOps::remote = false;
std::string AnOps::user = "chokopan", AnOps::ip = "192.168.0.180", AnOps::pw;
ushort AnOps::port = 22;
std::string AnOps::path = "/Users/chokopan/mdvis";
std::string AnOps::message = "disconnected";

std::thread* AnOps::conThread;

SSH AnOps::ssh;

void AnOps::Draw() {
	UI::Quad(Display::width - expandPos, 0.0f, 180.0f, Display::height - 18.0f, white(0.9f, 0.15f));
	if (expanded) {
		float w = 180;
		UI::Label(Display::width - expandPos + 5, 1, 12, "Options", white());
		
		UI::Label(Display::width - expandPos + 2, 18, 12, "Host", white());
		if (Engine::Button(Display::width - expandPos + 1, 35, expandPos*0.5f - 1, 16, remote ? white(1, 0.2f) : white(1, 0.1f), "Local", 12, white(), true) == MOUSE_RELEASE) {
			if (connectStatus == 255)
				Disconnect();
			remote = false;
		}
		if (Engine::Button(Display::width - expandPos*0.5f, 35, expandPos*0.5f - 1, 16, remote ? white(1, 0.1f) : white(1, 0.2f), "Remote", 12, white(), true) == MOUSE_RELEASE) {
			remote = true;
		}

		float off = 53;

		if (remote) {
			UI::Label(Display::width - expandPos + 4, 52, 12, "Domain", white());
			ip = UI::EditText(Display::width - expandPos + 80, 52, 99, 16, 12, white(1, 0.5f), ip, true, white());
			UI::Label(Display::width - expandPos + 4, 69, 12, "Port", white());
			port = TryParse(UI::EditText(Display::width - expandPos + 80, 69, 99, 16, 12, white(1, 0.5f), std::to_string(port), true, white()), 22);
			UI::Label(Display::width - expandPos + 4, 86, 12, "Username", white());
			user = UI::EditText(Display::width - expandPos + 80, 86, 99, 16, 12, white(1, 0.5f), user, true, white());
			UI::Label(Display::width - expandPos + 4, 103, 12, "Password", white());
			pw = UI::EditTextPass(Display::width - expandPos + 80, 103, 99, 16, 12, white(1, 0.5f), pw, '*', true, white());
			
			if (!connectStatus) {
				if (Engine::Button(Display::width - expandPos + 100, 120, 79, 16, white(1, 0.2f), "Connect", 12, white(), true) == MOUSE_RELEASE) {
					connectStatus = 1;
					conThread = new std::thread(Connect);
				}
			}
			else if (connectStatus == 255) {
				if (Engine::Button(Display::width - expandPos + 100, 120, 79, 16, white(1, 0.2f), "Disconnect", 12, white(), true) == MOUSE_RELEASE) {
					Disconnect();
				}
			}
			UI::Label(Display::width - expandPos + 2, 138, 12, message, Vec4(0.1f, 1.0f, 0.1f, 1.0f));

			UI::Label(Display::width - expandPos + 4, 157, 12, "Working directory", white());
			path = UI::EditText(Display::width - expandPos + 1, 174, 178, 16, 12, white(1, 0.5f), path, true, white());

			off = 177;
		}

		if (conThread && (!connectStatus || (connectStatus == 255))) {
			conThread->join();
			delete(conThread);
			conThread = 0;
		}


		off = max(off, Display::height - 68.0f - 18.f);
		UI::Label(Display::width - expandPos + 2, off, 12, "Status", white());
		UI::Label(Display::width - expandPos + 4, off + 17, 12, "C++", white());
		UI::Quad(Display::width - expandPos + 164, off + 18, 14, 14, (remote? AnWeb::hasC_s : AnWeb::hasC) ? green() : red());
		UI::Label(Display::width - expandPos + 4, off + 34, 12, "Python", white());
		UI::Quad(Display::width - expandPos + 164, off + 34, 14, 14, (remote ? AnWeb::hasPy_s : AnWeb::hasPy) ? green() : red());
		UI::Label(Display::width - expandPos + 4, off + 51, 12, "Fortran", white());
		UI::Quad(Display::width - expandPos + 164, off + 51, 14, 14, (remote ? AnWeb::hasFt_s : AnWeb::hasFt) ? green() : red());
		
		UI::Quad(Display::width - expandPos - 16.0f, Display::height - 34.0f, 16.0f, 16.0f, white(0.9f, 0.15f));
		if ((!UI::editingText && (Input::KeyUp(Key_O))) || Engine::Button(Display::width - expandPos - 16.0f, Display::height - 34.0f, 16.0f, 16.0f, Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE)
			expanded = false;
		expandPos = Clamp(expandPos + 1500 * Time::delta, 0.0f, 180.0f);
	}
	else {
		UI::Quad(Display::width - expandPos, 0.0f, expandPos, Display::height - 18.0f, white(0.9f, 0.15f));
		if ((!UI::editingText && (Input::KeyUp(Key_O))) || Engine::Button(Display::width - expandPos - 110.0f, Display::height - 34.0f, 110.0f, 16.0f, white(0.9f, 0.15f), white(1, 0.15f), white(1, 0.05f)) == MOUSE_RELEASE)
			expanded = true;
		UI::Texture(Display::width - expandPos - 110.0f, Display::height - 34.0f, 16.0f, 16.0f, Icons::expand);
		UI::Label(Display::width - expandPos - 92.0f, Display::height - 33.0f, 12.0f, "Options (O)", white());
		expandPos = Clamp(expandPos - 1500 * Time::delta, 2.0f, 180.0f);
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

	ssh.Flush();
	ssh.EnableSFTP();

	message = "initializing files";
	if (!ssh.HasFile(path + "/mdvis_ansrv")) {
		ssh.MkDir(path);
		ssh.SendFile(IO::path + "bin/mdvis_ansrv", path + "/mdvis_ansrv");
		ssh.Write("chmod +rx " + path + "/mdvis_ansrv");
	}
	ssh.Write("cd " + path);
	ssh.Write("mkdir nodes; rm -f nodes/*; mkdir -p ser/in; mkdir ser/out");
	ssh.Flush();

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
	ssh.Flush();
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