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