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
#include <liblsp/VFS.h>

#include <algorithm>
#include <deque>
#include <sstream>

using namespace std;

namespace std
{
	ostream& operator<<(ostream& _os, lsp::vfs::File const& _file)
	{
		_os << '"' << _file.uri() << "\": {languageId: " << _file.languageId();
		_os << ", version: " << _file.version();
		_os << ", text: \"";
		for (auto const ch: _file.contentString())
		{
			if (ch == '\n')
				_os << "\\n";
			else if (ch == '\t')
				_os << "\\t";
			else if (ch == '\r')
				_os << "\\r";
			else if (ch >= 0x20 && std::isprint(ch))
				_os << ch;
			else
			{
				char buf[5];
				snprintf(buf, sizeof(buf), "\\x%02x", ch);
				_os << buf;
			}
		}
		_os << "\"}";
		return _os;
	}

	ostream& operator<<(ostream& _os, lsp::vfs::VFS const& _vfs)
	{
		_os << "{size: " << _vfs.size() << "}";
		return _os;
	}
}

namespace lsp::vfs
{

File::File(string _uri, string _languageId, int _version, string _text):
	m_uri{ move(_uri) },
	m_languageId{ move(_languageId) },
	m_version{ _version },
	m_buffer{ move(_text) }
{
}

TextLines File::splitLines(string const& _text)
{
	TextLines result;

	size_t last = 0;
	size_t next = _text.find('\n');

	while (next != _text.npos) // string::npos
	{
		result.push_back(_text.substr(last, next - last));
		last = next + 1;
		next = _text.find('\n', last);
	}

	if (last != 0)
		result.push_back(_text.substr(last));
	else
		result.push_back(_text);

	return result;
}

void File::erase(Range const& _range)
{
	m_buffer.replace(_range, "");
}

void File::modify(Range const& _range, std::string const& _replacementText)
{
	m_buffer.replace(_range, _replacementText);
}

void File::replace(std::string const& _replacementText)
{
	m_buffer.assign(_replacementText);
}

File& VFS::insert(std::string _uri, std::string _languageId, int _version, string _text)
{
	if (auto i = m_files.find(_uri); i != end(m_files))
		return i->second = vfs::File(move(_uri), move(_languageId), _version, move(_text));
	else
		return m_files.emplace(pair{_uri, File{_uri, move(_languageId), _version, move(_text)}}).first->second;
}

File const* VFS::find(std::string const& _uri) const noexcept
{
	if (auto i = m_files.find(_uri); i != end(m_files))
		return &i->second;
	else
		return nullptr;
}

File* VFS::find(std::string const& _uri) noexcept
{
	if (auto i = m_files.find(_uri); i != end(m_files))
		return &i->second;
	else
		return nullptr;
}

void VFS::remove(std::string const& _uri)
{
	if (auto i = m_files.find(_uri); i != end(m_files))
		m_files.erase(i);
}

} // end namespace
