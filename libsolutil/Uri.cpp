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

#include <libsolutil/Uri.h>

#include <array>
#include <string>
#include <string_view>

using std::nullopt;
using std::optional;
using std::string;
using std::string_view;

namespace solidity::util
{

optional<Uri> Uri::parse(std::string_view _uri)
{
	// We don't care about complex URIs that contain IPv6 addresses, that's not out use-case here.
	enum class State {
		Error,
		Scheme,       // NAME
		SchemeColon,  // ':'
		SchemeSlash1, // '/'
		UserOrHost,   // NAME
		At,           // '@'
		Host,         // NAME
		PortStart,    // ':'
		Port,         // NUMBER
		PathStart,    // '/'
		Path,         // TEXT
		QueryStart,   // '?'
		Query,        // TEXT
		FragmentStart,// '#'
		Fragment      // TEXT
	};

	Uri uri;
	State state = State::Scheme;
	string text;

	for (auto i = begin(_uri); i != end(_uri) && state != State::Error; ++i)
	{
		auto const ch = char{*i};
		switch (state)
		{
			case State::Error:
				break;
			case State::Scheme:
				if (std::isalpha(ch))
					uri.scheme += ch;
				else if (ch == ':')
					state = State::SchemeColon;
				else
					state = State::Error;
				break;
			case State::SchemeColon:
				if (ch == '/')
					state = State::SchemeSlash1;
				else
					state = State::Error;
				break;
			case State::SchemeSlash1:
				if (ch == '/')
					// "scheme://" parsed"
					state = State::UserOrHost;
				else
					state = State::Error;
				break;
			case State::UserOrHost:
				if (ch == '@')
				{
					uri.user = move(text);
					text.clear();
					state = State::At;
				}
				else if (ch == ':')
				{
					uri.host = move(text);
					text.clear();
					state = State::PortStart;
				}
				else if (ch == '/')
				{
					uri.host = move(text);
					text.clear();
					uri.path += ch;
					state = State::Path;
				}
				else
				{
					text += ch;
				}
				break;
			case State::At:
				if (std::isalnum(ch) || ch == '.')
					text += ch;
				state = State::Host;
				break;
			case State::Host:
				if (ch == ':')
					state = State::PortStart;
				else if (ch == '/')
					state = State::PathStart;
				else if (std::isalnum(ch) || ch == '.')
					text += ch;
				else
					state = State::Error;
				break;
			case State::PortStart:
				if (std::isdigit(ch))
				{
					uri.port = ch - '0';
					state = State::Port;
				}
				else
					state = State::Error;
				break;
			case State::Port:
				if (std::isdigit(ch))
					uri.port = uri.port * 10 + (ch - '0');
				else if (ch == '/')
					state = State::PathStart;
				else
					state = State::Error;
				break;
			case State::PathStart:
				uri.path += '/';
				state = State::Path;
				break;
			case State::Path:
				if (ch == '?')
					state = State::Query;
				else if (ch == '#')
					state = State::Fragment;
				else
					uri.path += ch;
				break;
			case State::QueryStart:
				if (ch == '#')
					state = State::Fragment;
				else
					state = State::Query;
				break;
			case State::Query:
				if (ch == '#')
					state = State::Fragment;
				else
					uri.query += ch;
				break;
			case State::FragmentStart:
			case State::Fragment:
				uri.fragment += ch;
				break;
		}
	}

	if (state != State::Error)
		return uri;
	else
		return nullopt;
}

}
