#include "web/anscript.h"
#include "utils/dylib.h"

class FScript_I;
typedef std::shared_ptr<FScript_I> pCScript_I;

class FScript : public AnScript {
public:
	typedef void(*emptyFunc)();
	typedef char* (*wrapFunc)();

	FScript();

	DyLib lib;
	wrapFunc funcLoc;

	std::vector<CVar> _inputs, _outputs;
	std::vector<emptyFunc> _inarr_pre, _outarr_post;
	int pre, post;

	int32_t** arr_in_shapeloc;
	void** arr_in_dataloc;
	int32_t** arr_out_shapeloc;
	void** arr_out_dataloc;

	void Clear() override;
	pAnScript_I CreateInstance() override;

	static std::unordered_map<std::string, std::weak_ptr<FScript>> allScrs;
};

class FScript_I : public AnScript_I {
public:
	~FScript_I();

	void* Resolve(uintptr_t offset) override;
	int* GetDimValue(const CVar::szItem& i) override;

	void SetInput(int i, short val) override;
	void SetInput(int i, int val) override;
	void SetInput(int i, double val) override;
	void SetInput(int i, void* val, char tp, std::vector<int> szs) override;
	void GetOutput(int i, int* val) override;

	void GetOutputArrs();

	struct OutVal {
		VarVal val;
		std::vector<int> dims;
	};

	std::vector<OutVal> outputArrs;

	float GetProgress() override;
};