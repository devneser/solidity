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

#include <liblangutil/Exceptions.h>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

using namespace solidity;
namespace solidity::langutil
{

SourceLocation const parseSourceLocation(std::string const& _input, std::string const& _sourceName, size_t _maxIndex)
{
	// Expected input: "start:length:sourceindex"
	enum SrcElem : size_t { Start, Length, Index };

	std::vector<std::string> pos;

	boost::algorithm::split(pos, _input, boost::is_any_of(":"));

	// Index-value must be a valid index or the special value (size_t::max) to indicate that there
	// is no source, which is done in at least ASTJsonConverter::sourceIndexFromLocation().
	char* eptr = nullptr;
	auto const indexValue = size_t{std::strtoul(pos[Index].c_str(), &eptr, 10)};
	astAssert(
		pos.size() == 3 &&
		eptr && *eptr == '\0' &&
		(_maxIndex >= indexValue || indexValue == std::numeric_limits<size_t>::max()),
		"'src'-field ill-formatted or src-index too high: " + std::to_string(indexValue)
	);

	int start = stoi(pos[Start]);
	int end = start + stoi(pos[Length]);

	// ASSUMPTION: only the name of source is used from here on, the m_source of the CharStream-Object can be empty
	std::shared_ptr<langutil::CharStream> source = std::make_shared<langutil::CharStream>("", _sourceName);

	return SourceLocation{start, end, source};
}

}
