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

#include <json/value.h>

#include <functional>
#include <iosfwd>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace lsp {

// NOTE: https://microsoft.github.io/language-server-protocol/specifications/specification-3-14/
//
// This file contains the high level definitions of the underlying transport protocol.
// It is not meant to include all but only what is necessary for solls.
using MessageId = std::variant<int, std::string>;

enum class ErrorCode
{
	// Defined by JSON RPC
	ParseError = -32700,
	InvalidRequest = -32600,
	MethodNotFound = -32601,
	InvalidParams = -32602,
	InternalError = -32603,
	serverErrorStart = -32099,
	serverErrorEnd = -32000,
	ServerNotInitialized = -32002,
	UnknownErrorCode = -32001,

	// Defined by the protocol.
	RequestCancelled = -32800,
	ContentModified = -32801,
};

/// Transport layer API
///
/// The transport layer API is abstracted so it users become more testable as well as
/// this way it could be possible to support other transports (HTTP for example) easily.
class Transport
{
public:
	virtual ~Transport() = default;

	/// @returns boolean indicating whether or not the underlying (input) stream is closed.
	virtual bool closed() const noexcept = 0;

	/// Reveives a message
	virtual std::optional<Json::Value> receive() = 0;
	// TODO: ^^ think about variant<Json::Value, Timeout, Closed, error_code> as return type instead

	/// Sends a notification message to the other end (client).
	virtual void notify(std::string const& _method, Json::Value const& _params) = 0;

	/// Sends a reply message, optionally with a given ID to correlate this message to another from the other end.
	virtual void reply(MessageId const& _id, Json::Value const& _result) = 0;

	/// Sends an error reply with regards to the given request ID.
	virtual void error(MessageId const& _id, ErrorCode _code, std::string const& _message) = 0;
};

/// Standard JSON-RPC stream transport over standard iostream.
class JSONTransport: public Transport
{
public:
	/// Constructs a standard stream transport layer.
	///
	/// @param _in for example std::cin (stdin)
	/// @param _out for example std::cout (stdout)
	/// @param _trace special logger used for debugging the LSP messages.
	JSONTransport(std::istream& _in, std::ostream& _out, std::function<void(std::string_view)> _trace);

	// Constructs a JSON transport using standard I/O streams.
	explicit JSONTransport(std::function<void(std::string_view)> _trace);

	bool closed() const noexcept override;
	std::optional<Json::Value> receive() override;
	void notify(std::string const& _method, Json::Value const& _params) override;
	void reply(MessageId const& _id, Json::Value const& _result) override;
	void error(MessageId const& _id, ErrorCode _code, std::string const& _message) override;

protected:
	using HeaderMap = std::map<std::string, std::string>;

	/// Reads given number of bytes from the client.
	virtual std::string readBytes(int _n);

	/// Sends an arbitrary raw message to the client.
	///
	/// Used by the notify/reply/error function family.
	virtual void send(Json::Value const& _message);

	/// Parses a single text line from the client ending with CRLF (or just LF).
	std::string readLine();

	/// Parses header section from the client including message-delimiting empty line.
	std::optional<HeaderMap> parseHeaders();

	/// Appends given JSON message to the trace log.
	void traceMessage(Json::Value const& _message, std::string_view _title);

private:
	std::istream& m_input;
	std::ostream& m_output;
	std::function<void(std::string_view)> m_trace;
};

} // end namespace
