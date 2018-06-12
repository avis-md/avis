#include "anops.h"
#include "ui/icons.h"
#include "libssh2_sftp.h"

bool AnOps::expanded = true;
byte AnOps::connectStatus;
float AnOps::expandPos;

bool AnOps::remote = true;
string AnOps::user = "chokopan", AnOps::ip = "192.168.0.180", AnOps::pw;
ushort AnOps::port = 22;
string AnOps::path = "/Users/chokopan/mdvis";
string AnOps::message = "disconnected";

std::thread* AnOps::conThread;

SSH AnOps::ssh;

void AnOps::Draw() {
	Engine::DrawQuad(Display::width - expandPos, 0.0f, 180.0f, Display::height - 18.0f, white(0.9f, 0.15f));
	if (expanded) {
		float w = 180;
		UI::Label(Display::width - expandPos + 5, 1, 12, "Options", AnNode::font, white());
		
		UI::Label(Display::width - expandPos + 2, 18, 12, "Host", AnNode::font, white());
		if (Engine::Button(Display::width - expandPos + 1, 35, expandPos*0.5f - 1, 16, remote ? white(1, 0.2f) : white(1, 0.1f), "Local", 12, AnNode::font, white(), true) == MOUSE_RELEASE) {
			if (connectStatus == 255)
				Disconnect();
			remote = false;
		}
		if (Engine::Button(Display::width - expandPos*0.5f, 35, expandPos*0.5f - 1, 16, remote ? white(1, 0.1f) : white(1, 0.2f), "Remote", 12, AnNode::font, white(), true) == MOUSE_RELEASE) {
			remote = true;
		}

		float off = 53;

		if (remote) {
			UI::Label(Display::width - expandPos + 4, 52, 12, "Domain", AnNode::font, white());
			ip = UI::EditText(Display::width - expandPos + 80, 52, 99, 16, 12, white(1, 0.5f), ip, AnNode::font, true, nullptr, white());
			UI::Label(Display::width - expandPos + 4, 69, 12, "Port", AnNode::font, white());
			port = TryParse(UI::EditText(Display::width - expandPos + 80, 69, 99, 16, 12, white(1, 0.5f), std::to_string(port), AnNode::font, true, nullptr, white()), 22);
			UI::Label(Display::width - expandPos + 4, 86, 12, "Username", AnNode::font, white());
			user = UI::EditText(Display::width - expandPos + 80, 86, 99, 16, 12, white(1, 0.5f), user, AnNode::font, true, nullptr, white());
			UI::Label(Display::width - expandPos + 4, 103, 12, "Password", AnNode::font, white());
			pw = UI::EditTextPass(Display::width - expandPos + 80, 103, 99, 16, 12, white(1, 0.5f), pw, '*', AnNode::font, true, nullptr, white());
			
			if (!connectStatus) {
				if (Engine::Button(Display::width - expandPos + 100, 120, 79, 16, white(1, 0.2f), "Connect", 12, AnNode::font, white(), true) == MOUSE_RELEASE) {
					connectStatus = 1;
					conThread = new std::thread(Connect);
				}
			}
			else if (connectStatus == 255) {
				if (Engine::Button(Display::width - expandPos + 100, 120, 79, 16, white(1, 0.2f), "Disconnect", 12, AnNode::font, white(), true) == MOUSE_RELEASE) {
					Disconnect();
				}
			}
			UI::Label(Display::width - expandPos + 2, 138, 12, message, AnNode::font, Vec4(0.1f, 1.0f, 0.1f, 1.0f));

			UI::Label(Display::width - expandPos + 4, 157, 12, "Working directory", AnNode::font, white());
			path = UI::EditText(Display::width - expandPos + 1, 174, 178, 16, 12, white(1, 0.5f), path, AnNode::font, true, nullptr, white());

			off = 177;
		}

		if (conThread && (!connectStatus || (connectStatus == 255))) {
			conThread->join();
			delete(conThread);
			conThread = 0;
		}


		off = max(off, Display::height - 68.0f - 18.f);
		UI::Label(Display::width - expandPos + 2, off, 12, "Status", AnNode::font, white());
		UI::Label(Display::width - expandPos + 4, off + 17, 12, "C++", AnNode::font, white());
		Engine::DrawQuad(Display::width - expandPos + 164, off + 18, 14, 14, (remote? AnWeb::hasC_s : AnWeb::hasC) ? green() : red());
		UI::Label(Display::width - expandPos + 4, off + 34, 12, "Python", AnNode::font, white());
		Engine::DrawQuad(Display::width - expandPos + 164, off + 34, 14, 14, (remote ? AnWeb::hasPy_s : AnWeb::hasPy) ? green() : red());
		UI::Label(Display::width - expandPos + 4, off + 51, 12, "Fortran", AnNode::font, white());
		Engine::DrawQuad(Display::width - expandPos + 164, off + 51, 14, 14, (remote ? AnWeb::hasFt_s : AnWeb::hasFt) ? green() : red());
		
		Engine::DrawQuad(Display::width - expandPos - 16.0f, Display::height - 34.0f, 16.0f, 16.0f, white(0.9f, 0.15f));
		if ((!UI::editingText && (Input::KeyUp(Key_O)) || Engine::Button(Display::width - expandPos - 16.0f, Display::height - 34.0f, 16.0f, 16.0f, Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE))
			expanded = false;
		expandPos = Clamp(expandPos + 1500 * Time::delta, 0.0f, 180.0f);
	}
	else {
		Engine::DrawQuad(Display::width - expandPos, 0.0f, expandPos, Display::height - 18.0f, white(0.9f, 0.15f));
		if ((!UI::editingText && (Input::KeyUp(Key_O)) || Engine::Button(Display::width - expandPos - 110.0f, Display::height - 34.0f, 110.0f, 16.0f, white(0.9f, 0.15f), white(1, 0.15f), white(1, 0.05f)) == MOUSE_RELEASE))
			expanded = true;
		UI::Texture(Display::width - expandPos - 110.0f, Display::height - 34.0f, 16.0f, 16.0f, Icons::expand);
		UI::Label(Display::width - expandPos - 92.0f, Display::height - 33.0f, 12.0f, "Options (O)", AnNode::font, white());
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

	ssh.EnableSFTP();
	message = "initializing files";
	if (!ssh.HasFile(path + "/mdvis_ansrv")) {
		ssh.MkDir(path);
		ssh.SendFile(IO::path + "/bin/mdvis_ansrv", path + "/mdvis_ansrv");
		ssh.Write("chmod +rx " + path + "/mdvis_ansrv");
	}
	AnOps::ssh.Write("mkdir nodes; mkdir -p ser/in; mkdir ser/out");

	message = "scanning runtime";

	ssh.Write("cd " + path);

	string pth;
	AnWeb::hasC_s = ssh.HasCmd("g++", pth);
	Debug::Message("AnOps", "g++ location: " + pth);
	AnWeb::hasPy_s = ssh.HasCmd("python", pth);
	Debug::Message("AnOps", "python location: " + pth);
	AnWeb::hasFt_s = ssh.HasCmd("gfortran", pth);
	Debug::Message("AnOps", "gfortran location: " + pth);
	
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

void AnOps::SendIn() {
	message = "syncing";
	auto ins = IO::GetFiles(IO::path + "/nodes/__tmp__/in/");
	for (auto& i : ins) {
		ssh.SendFile(IO::path + "/nodes/__tmp__/in/" + i, path + "/ser/in/" + i);
		ssh.Write("chmod +r ser/in/" + i);
	}
}