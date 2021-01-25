/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0
#pragma once

#include <boost/filesystem.hpp>
#include <map>
#include <vector>

#include <libsolidity/interface/ReadFile.h>

namespace solidity {

class FileReader
{
public:
	using StringMap = std::map<std::string, std::string>;

	FileReader(
		boost::filesystem::path _basePath,
		std::vector<boost::filesystem::path> _allowedDirectories
	):
		m_basePath(std::move(_basePath)),
		m_allowedDirectories(std::move(_allowedDirectories)),
		m_sourceCodes(),
		m_fullPathMapping()
	{}

	FileReader(): FileReader({}, {}) {}

	boost::filesystem::path const& basePath() const noexcept { return m_basePath; }
	void setBasePath(boost::filesystem::path const& _path);

	void allowDirectory(boost::filesystem::path _path);

	std::vector<boost::filesystem::path> const& allowedDirectories() const noexcept { return m_allowedDirectories; }

	StringMap const& sourceCodes() const noexcept { return m_sourceCodes; }
	StringMap& sourceCodes() noexcept { return m_sourceCodes; }

	std::vector<std::string> sourceNames() const;
	std::string const& sourceCode(const std::string& _name) const { return m_sourceCodes.at(_name); }

	StringMap const& fullPathMapping() const noexcept { return m_fullPathMapping; }

	/// Adds given source, imported as @p _path with filesystem path @p _fspath.
	void setSource(std::string _path, std::optional<boost::filesystem::path> _fspath, std::string _source);

	/// Resets all sources to the given ones.
	void setSources(StringMap _sources);

	frontend::ReadCallback::Result readFile(std::string const& _kind, std::string const& _path);

	frontend::ReadCallback::Result operator()(std::string const& _kind, std::string const& _path)
	{
		return readFile(_kind, _path);
	}

private:
	/// Base path, used for resolving relative paths in imports.
	boost::filesystem::path m_basePath;

	/// list of allowed directories to read files from
	std::vector<boost::filesystem::path> m_allowedDirectories;

	/// map of input files to source code strings
	StringMap m_sourceCodes;

	/// map of input file names to full path names suitable for file://-URIs
	StringMap m_fullPathMapping;
};

}
