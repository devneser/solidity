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
/**
 * Unit tests for UTF-8 validation.
 */

#include <libsolutil/CommonData.h>
#include <libsolutil/Uri.h>

#include <test/Common.h>

#include <boost/test/unit_test.hpp>

using namespace std::string_view_literals;

namespace solidity::util::test
{

BOOST_AUTO_TEST_SUITE(UriTest, *boost::unit_test::label("nooptions"))

BOOST_AUTO_TEST_CASE(valid_full)
{
	auto const uri = Uri::parse("http://github.com/some/path?query#fragment"sv).value();
	BOOST_REQUIRE(uri.scheme == "http");
	BOOST_REQUIRE(uri.host == "github.com");
	BOOST_REQUIRE(uri.path == "/some/path");
	BOOST_REQUIRE(uri.query == "query");
	BOOST_REQUIRE(uri.fragment == "fragment");
}

BOOST_AUTO_TEST_CASE(valid_fragment)
{
	auto const uri = Uri::parse("http://github.com/some/path#fragment"sv).value();
	BOOST_REQUIRE(uri.scheme == "http");
	BOOST_REQUIRE(uri.host == "github.com");
	BOOST_REQUIRE(uri.path == "/some/path");
	BOOST_REQUIRE(uri.query == "");
	BOOST_REQUIRE(uri.fragment == "fragment");
}

BOOST_AUTO_TEST_CASE(file_uri)
{
	auto const uri = Uri::parse("file:///path/to/file.sol"sv).value();
	BOOST_REQUIRE(uri.scheme == "file");
	BOOST_REQUIRE(uri.host == "");
	BOOST_REQUIRE(uri.path == "/path/to/file.sol");
	BOOST_REQUIRE(uri.query == "");
	BOOST_REQUIRE(uri.fragment == "");
}

BOOST_AUTO_TEST_SUITE_END()

}

