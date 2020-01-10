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

#include <liblsp/Range.h>
#include <liblsp/TextBuffer.h>

#include <deque>
#include <map>
#include <string>
#include <ostream>
#include <vector>

namespace lsp::vfs
{

using TextLines = std::deque<std::string>;

class File
{
public:
	File(std::string _uri, std::string _languageId, int _version, std::string _text);

	// readers
	std::string const& uri() const noexcept { return m_uri; }
	std::string const& languageId() const noexcept { return m_languageId; }
	constexpr int version() const noexcept { return m_version; }
	std::string const& contentString() const { return m_buffer.data(); }
	TextBuffer const& buffer() const noexcept { return m_buffer; }

	// modifiers
	constexpr void setVersion(int _version) noexcept { m_version = _version; }
	void erase(Range const& _range);
	void modify(Range const& _range, std::string const& _replacementText);
	void replace(std::string const& _replacementText);

	static TextLines splitLines(std::string const& _text);

private:
	std::string m_uri;
	std::string m_languageId;
	int m_version;
	TextBuffer m_buffer;
};

class VFS
{
public:
	// accessors
	//
	size_t size() const noexcept { return m_files.size(); }
	File const* find(std::string const& _uri) const noexcept;
	File* find(std::string const& _uri) noexcept;

	// modifiers
	//
	File& insert(std::string _uri, std::string _languageId, int _version, TextLines _text);
	File& insert(std::string _uri, std::string _languageId, int _version, std::string _text);
	void remove(std::string const& _uri);

	/// Modifies given VFS file by deleting the @p _range and replace it with the @p _replacementText.
	void modify(std::string const& _uri, Range const& _range, std::string const& _replacementText);

	/// Retrieves a read-only list of all files available in this VFS.
	std::vector<std::reference_wrapper<File const>> files() const
	{
		std::vector<std::reference_wrapper<File const>> result;
		result.reserve(m_files.size());
		for (auto const& file: m_files)
			result.emplace_back(file.second);
		return result;
	}

private:
	std::map<std::string, File> m_files = {};
};

} // end namespace

namespace std
{

ostream& operator<<(ostream& _os, lsp::vfs::File const& _file);
ostream& operator<<(ostream& _os, lsp::vfs::VFS const& _vfs);

} // end namespace std
