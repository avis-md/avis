#include "web/anweb.h"

class CNode : public AnNode {
public:
	CNode(pCScript_I);

	std::vector<void*> inputV, outputV;

	void Update() override;

	void PreExecute() override;
	void Execute() override;
	void WriteFrame(int f) override;
	void RemoveFrames() override;

	void Reconn() override;
	void CatchExp(char* c) override;
};