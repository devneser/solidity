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
/**
 * Unit tests for the address checksum.
 */

#include <liblsp/VFS.h>
#include <liblsp/TextBuffer.h>
#include <libsolutil/Exceptions.h>

#include <test/Common.h>

#include <boost/test/unit_test.hpp>

// namespace std {
// 	ostream& operator<<(ostream& _os, lsp::vfs::TextLines const& _lines)
// 	{
// 		_os << '{';
// 		for (size_t i = 0; i < _lines.size(); ++i)
// 			_os << (i ? ", " : "") << '"' << _lines[i] << '"';
// 		_os << '}';
// 		return _os;
// 	}
// }

using namespace std;

namespace lsp::test
{

BOOST_AUTO_TEST_SUITE(LSP)

BOOST_AUTO_TEST_CASE(vfs_File_splitLines)
{
	string const multiLine1 = "Hello";
	vfs::TextLines lines1 = vfs::File::splitLines(multiLine1);
	BOOST_CHECK_EQUAL(lines1.size(), 1);
	BOOST_CHECK_EQUAL(lines1.front(), "Hello");

	string const multiLine2 = "Hello\nWorld";
	vfs::TextLines const lines2 = vfs::File::splitLines(multiLine2);
	BOOST_CHECK_EQUAL(lines2.size(), 2);
	BOOST_CHECK_EQUAL(lines2.front(), "Hello"sv);
	BOOST_CHECK_EQUAL(lines2.back(), "World"sv);

	string const multiLine3 = "Hello\nWorld\n";
	vfs::TextLines lines3 = vfs::File::splitLines(multiLine3);
	BOOST_CHECK_EQUAL(lines3.size(), 3);
	BOOST_CHECK_EQUAL(lines3[0], "Hello");
	BOOST_CHECK_EQUAL(lines3[1], "World");
	BOOST_CHECK_EQUAL(lines3[2], "");

	string const emptyLinesStr = "\n";
	vfs::TextLines emptyLines = vfs::File::splitLines(emptyLinesStr);
	BOOST_CHECK_EQUAL(emptyLines.size(), 2);
	BOOST_CHECK_EQUAL(emptyLines[0], "");
	BOOST_CHECK_EQUAL(emptyLines[1], "");
}

BOOST_AUTO_TEST_CASE(VFS_create)
{
	vfs::VFS vfs;
	vfs.insert("file:///project/test.txt", "text", 1, "Hello, World\n");

	auto const file = vfs.find("file:///project/test.txt");
	BOOST_CHECK_NE(file, nullptr);

	BOOST_CHECK_EQUAL(file->uri(), "file:///project/test.txt");
	BOOST_CHECK_EQUAL(file->languageId(), "text");
	BOOST_CHECK_EQUAL(file->version(), 1);

	BOOST_CHECK_EQUAL(file->contentString(), "Hello, World\n");
}

BOOST_AUTO_TEST_CASE(VFS_modify_erase)
{
	vfs::VFS vfs;
	vfs::File& file = vfs.insert("file:///project/test.txt", "text", 1, "Hello, World\n");
	file.modify(Range{{0, 0}, {0, 1}}, "");

	BOOST_CHECK_EQUAL(file.contentString(), "ello, World\n");

	file.modify(Range{{0, 4}, {0, 5}}, "");
	BOOST_CHECK_EQUAL(file.contentString(), "ello World\n");

	file.modify(Range{{0, 4}, {0, 10}}, "");
	BOOST_CHECK_EQUAL(file.contentString(), "ello\n");
}

BOOST_AUTO_TEST_CASE(VFS_modify_erase_multiline)
{
	vfs::VFS vfs;
	vfs::File& file = vfs.insert("file:///project/test.txt", "text", 1, "Hello,\nWorld\nCrew\n");
	file.modify(Range{{0, 1}, {2, 2}}, "");

	BOOST_CHECK_EQUAL(file.contentString(), "Hew\n");
}

BOOST_AUTO_TEST_CASE(VFS_modify_change)
{
	vfs::VFS vfs;
	vfs::File& file = vfs.insert("file:///project/test.txt", "text", 1, "Hello, World\n");
	file.modify(Range{{0, 5}, {0, 6}}, ";");

	BOOST_CHECK_EQUAL(file.contentString(), "Hello; World\n");
}

BOOST_AUTO_TEST_CASE(VFS_modify_change_single_to_multi_line2)
{
	// replace fragment of a single line with 2 lines
	vfs::VFS vfs;
	vfs::File& file = vfs.insert("file:///project/test.txt", "text", 1, "Hello\nWorld\n");
	file.modify(Range{{0, 1}, {0, 2}}, "{foo\nbar}");

	BOOST_CHECK_EQUAL(file.contentString(), "H{foo\nbar}llo\nWorld\n");
}

BOOST_AUTO_TEST_CASE(VFS_modify_change_single_to_multi_line3)
{
	// replace fragment of a single line with 3 lines
	vfs::VFS vfs;
	vfs::File& file = vfs.insert("file:///project/test.txt", "text", 1, "Hello\nWorld\n");
	file.modify(Range{{0, 1}, {0, 2}}, "{foo\nbar\ncom}");

	BOOST_CHECK_EQUAL(file.contentString(), "H{foo\nbar\ncom}llo\nWorld\n");
}

BOOST_AUTO_TEST_CASE(VFS_modify_change_single_to_multi_line3_last_empty)
{
	// replace fragment of a single line with 3 lines
	vfs::VFS vfs;
	vfs::File& file = vfs.insert("file:///project/test.txt", "text", 1, "Hello\nWorld\n");
	file.modify(Range{{0, 1}, {0, 2}}, "{foo\nbar}\n");

	BOOST_CHECK_EQUAL(file.contentString(), "H{foo\nbar}\nllo\nWorld\n");
}

BOOST_AUTO_TEST_CASE(VFS_modify_insert_at_the_beginning)
{
	TextBuffer text{"Hello, World\n"};

	text.replace(Range{{0, 0}, {0, 0}}, "[");
	BOOST_CHECK_EQUAL(text.data(), "[Hello, World\n");

	TextBuffer text2{"Hello,\nWorld\n"};
	text2.replace(Range{{1, 0}, {1, 0}}, "[");
	BOOST_CHECK_EQUAL(text2.data(), "Hello,\n[World\n");

	text2[Range{{2, 0}, {2, 0}}] = "[";
	BOOST_CHECK_EQUAL(text2.data(), "Hello,\n[World\n[");
}

BOOST_AUTO_TEST_CASE(VFS_modify_insert)
{
	vfs::VFS vfs;
	vfs::File& file = vfs.insert("file:///project/test.txt", "text", 1, "Hello, World\n");
	file.modify(Range{{0, 5}, {0, 5}}, ";");

	BOOST_CHECK_EQUAL(file.contentString(), "Hello;, World\n");
}

BOOST_AUTO_TEST_SUITE_END()

} // end namespace
