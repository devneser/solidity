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
#include <libsolidity/interface/FileReader.h>

#include <liblangutil/Exceptions.h>

#include <libsolutil/Exceptions.h>
#include <libsolutil/CommonIO.h>
#include <libsolutil/CommonData.h>

using solidity::frontend::ReadCallback;
using solidity::langutil::InternalCompilerError;
using solidity::util::errinfo_comment;
using solidity::util::readFileAsString;
using std::optional;
using std::string;
using std::vector;

namespace solidity {

vector<string> FileReader::sourceNames() const
{
	vector<string> names;
	for (auto const& source: m_sourceCodes)
		names.emplace_back(source.first);
	return names;
}

void FileReader::setBasePath(boost::filesystem::path const& _path)
{
	m_basePath = _path;
	allowDirectory(_path);
}

void FileReader::allowDirectory(boost::filesystem::path _path)
{
	if (!util::contains(m_allowedDirectories, _path))
		m_allowedDirectories.emplace_back(std::move(_path));
}

void FileReader::setSource(string _path, optional<boost::filesystem::path> _fspath, string _source)
{
	if (_fspath.has_value())
		m_fullPathMapping[_path] = _fspath.value().generic_string();

	m_sourceCodes[std::move(_path)] = std::move(_source);
}

void FileReader::setSources(StringMap  _sources)
{
	m_sourceCodes = std::move(_sources);
}

// TODO: move to <solidity/interface/ReadFile.h>?
ReadCallback::Result FileReader::readFile(string const& _kind, string const& _path)
{
	try
	{
		if (_kind != ReadCallback::kindString(ReadCallback::Kind::ReadFile))
			BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment(
				"ReadFile callback used as callback kind " +
				_kind
			));
		string validPath = _path;
		if (validPath.find("file://") == 0)
			validPath.erase(0, 7);

		auto const path = m_basePath / validPath;
		auto canonicalPath = boost::filesystem::weakly_canonical(path);
		bool isAllowed = false;
		for (auto const& allowedDir: m_allowedDirectories)
		{
			// If dir is a prefix of boostPath, we are fine.
			if (
				std::distance(allowedDir.begin(), allowedDir.end()) <= std::distance(canonicalPath.begin(), canonicalPath.end()) &&
				std::equal(allowedDir.begin(), allowedDir.end(), canonicalPath.begin())
			)
			{
				isAllowed = true;
				break;
			}
		}
		if (!isAllowed)
			return ReadCallback::Result{false, "File outside of allowed directories."};

		if (!boost::filesystem::exists(canonicalPath))
			return ReadCallback::Result{false, "File not found."};

		if (!boost::filesystem::is_regular_file(canonicalPath))
			return ReadCallback::Result{false, "Not a valid file."};

		// NOTE: we ignore the FileNotFound exception as we manually check above
		auto contents = readFileAsString(canonicalPath.string());
		m_sourceCodes[path.generic_string()] = contents;
		m_fullPathMapping[_path] = path.generic_string();
		return ReadCallback::Result{true, contents};
	}
	catch (util::Exception const& _exception)
	{
		return ReadCallback::Result{false, "Exception in read callback: " + boost::diagnostic_information(_exception)};
	}
	catch (...)
	{
		return ReadCallback::Result{false, "Unknown exception in read callback."};
	}
}

}

