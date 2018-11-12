#include "Engine.h"

class BrowseTarget {
public:
	virtual ~BrowseTarget() {}

	std::string path;
	std::vector<std::string> files, fds;
	virtual void Seek(std::string fd, bool isfull) = 0;
	virtual void Home() = 0;
};

class LocalBrowseTarget : public BrowseTarget {
public:
	LocalBrowseTarget(std::string path = "");
	void Seek(std::string fd, bool isfull) override;
	void Home() override;
};

class RemoteBrowseTarget : public BrowseTarget {
public:
	RemoteBrowseTarget(std::string path = "");
	void Seek(std::string fd, bool isfull) override;
	void Home() override;
};