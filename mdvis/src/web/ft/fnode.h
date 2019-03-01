#include "web/anweb.h"

class FNode : public AnNode {
public:
	FNode(pFScript_I);

	void Update() override;

	void PreExecute() override;
	void Execute() override;
	void WriteFrame(int f) override;
	void RemoveFrames() override;

	void Reconn() override;
	void CatchExp(char* c) override;
};