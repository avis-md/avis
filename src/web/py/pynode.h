#include "web/anweb.h"

class PyNode : public AnNode {
public:
	PyNode(pPyScript_I);

	void Update() override;

	void PreExecute() override;
	void Execute() override;
	void WriteFrame(int f) override;
	void RemoveFrames() override;

	void Reconn() override;
	void CatchExp(char* c) override;
};