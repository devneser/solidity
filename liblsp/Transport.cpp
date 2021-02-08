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
#include <liblsp/Transport.h>

#include <libsolutil/JSON.h>
#include <libsolutil/Visitor.h>

#include <boost/algorithm/string.hpp>

#include <iostream>
#include <sstream>

using std::cin;
using std::cout;
using std::istream;
using std::monostate;
using std::nullopt;
using std::optional;
using std::ostream;
using std::string;
using std::stringstream;

namespace lsp {

JSONTransport::JSONTransport(istream& _in, ostream& _out, std::function<void(std::string_view)> _trace):
	m_input{_in},
	m_output{_out},
	m_trace{std::move(_trace)}
{
}

JSONTransport::JSONTransport(std::function<void(std::string_view)> _trace):
	JSONTransport(cin, cout, std::move(_trace))
{
}

bool JSONTransport::closed() const noexcept
{
	return m_input.eof();
}

optional<Json::Value> JSONTransport::receive()
{
	auto const headers = parseHeaders();
	if (!headers)
		return nullopt;

	if (!headers->count("content-length"))
		return nullopt;

	string const data = readBytes(stoi(headers->at("content-length")));

	Json::Value jsonMessage;
	string errs;
	solidity::util::jsonParseStrict(data, jsonMessage, &errs);
	if (!errs.empty())
		return nullopt; // JsonParseError

	traceMessage(jsonMessage, "Request");

	return {jsonMessage};
}

void JSONTransport::notify(string const& _method, Json::Value const& _message)
{
	Json::Value json;
	json["jsonrpc"] = "2.0";
	json["method"] = _method;
	json["params"] = _message;
	send(json);
}

void JSONTransport::reply(MessageId const& _id, Json::Value const& _message)
{
	Json::Value json;
	json["jsonrpc"] = "2.0";
	json["result"] = _message;
	visit(solidity::util::GenericVisitor{
		[&](int _id) { json["id"] = _id; },
		[&](string const& _id) { json["id"] = _id; },
		[&](monostate) {}
	}, _id);
	send(json);
}

void JSONTransport::error(MessageId const& _id, ErrorCode _code, string const& _message)
{
	Json::Value json;
	json["jsonrpc"] = "2.0";
	visit(solidity::util::GenericVisitor{
		[&](int _id) { json["id"] = _id; },
		[&](string const& _id) { json["id"] = _id; },
		[&](monostate) {}
	}, _id);
	json["error"]["code"] = static_cast<int>(_code);
	json["error"]["message"] = _message;
	send(json);
}

void JSONTransport::send(Json::Value const& _json)
{
	string const jsonString = solidity::util::jsonCompactPrint(_json);

	m_output << "Content-Length: " << jsonString.size() << "\r\n";
	m_output << "\r\n";
	m_output << jsonString;

	m_output.flush();
	//traceMessage(_json, "Response");
}

void JSONTransport::traceMessage(Json::Value const& _message, std::string_view _title)
{
	if (m_trace)
	{
		auto const jsonString = solidity::util::jsonPrettyPrint(_message);

		stringstream sstr;
		sstr << _title << ": " << jsonString;

		m_trace(sstr.str());
	}
}

string JSONTransport::readLine()
{
	string line;

	getline(m_input, line);
	if (!line.empty() && line.back() == '\r')
		line.resize(line.size() - 1);

	return line;
}

optional<JSONTransport::HeaderMap> JSONTransport::parseHeaders()
{
	HeaderMap headers;

	for (string line = readLine(); !line.empty(); line = readLine())
	{
		auto const delimiterPos = line.find(':');
		if (delimiterPos == string::npos)
			return nullopt;

		auto const name = boost::to_lower_copy(line.substr(0, delimiterPos));
		auto const value = boost::trim_copy(line.substr(delimiterPos + 1));

		headers[name] = value;
	}
	return {headers};
}

string JSONTransport::readBytes(int _n)
{
	if (_n < 0)
		return {};

	string data;
	data.resize(static_cast<string::size_type>(_n));
	m_input.read(data.data(), _n);
	return data;
}

} // end namespace
