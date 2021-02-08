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

#include <liblsp/Server.h>
#include <liblsp/VFS.h>

#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/FileReader.h>
#include <libsolidity/ast/AST.h>

#include <json/value.h>

#include <boost/filesystem/path.hpp>

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace solidity::frontend {
	class Declaration;
}

namespace solidity::lsp {

/// Solidity Language Server, managing one LSP client.
class LanguageServer: public ::lsp::Server
{
public:
	using Logger = std::function<void(std::string_view)>;

	/// @param _logger special logger used for debugging the LSP.
	explicit LanguageServer(::lsp::Transport& _client, Logger _logger);

	//std::vector<boost::filesystem::path>& allowedDirectories() noexcept { return m_allowedDirectories; }

	// Client-to-Server messages
	::lsp::ServerId initialize(std::string _rootUri, std::vector<::lsp::WorkspaceFolder> _workspaceFolders) override;
	void initialized() override;
	void shutdown() override;
	void changeConfiguration(SettingsMaps const&) override;
	void documentOpened(std::string const& _uri, std::string _languageId, int _documentVersion, std::string _contents) override;
	void documentContentUpdated(std::string const& _uri, std::optional<int> _documentVersion, std::string const& _fullContentChange) override;
	void documentContentUpdated(std::string const& _uri, std::optional<int> _version, ::lsp::Range _range, std::string const& _text) override;
	void documentContentUpdated(std::string const& _uri) override;
	void documentClosed(std::string const& _uri) override;
	std::vector<::lsp::Location> gotoDefinition(::lsp::DocumentPosition _position) override;
	std::vector<::lsp::DocumentHighlight> semanticHighlight(::lsp::DocumentPosition _documentPosition) override;
	std::vector<::lsp::Location> references(::lsp::DocumentPosition _documentPosition) override;

	/// performs a validation run.
	///
	/// update diagnostics and also pushes any updates to the client.
	void validateAll();
	void validate(::lsp::vfs::File const& _file, std::vector<::lsp::PublishDiagnostics>& _result);
	void validate(::lsp::vfs::File const& _file);

private:
	frontend::ReadCallback::Result readFile(std::string const&, std::string const&);

	void compile(::lsp::vfs::File const& _file);

	frontend::ASTNode const* findASTNode(::lsp::Position const& _position, std::string const& _fileName);

	std::optional<::lsp::Location> declarationPosition(frontend::Declaration const* _declaration);

	std::vector<::lsp::DocumentHighlight> findAllReferences(
		frontend::Declaration const* _declaration,
		std::string const& _sourceIdentifierNam,
		frontend::SourceUnit const& _sourceUnit
	);

	std::vector<::lsp::DocumentHighlight> findAllReferences(
		frontend::Declaration const* _declaration,
		frontend::SourceUnit const& _sourceUnit
	)
	{
		if (_declaration)
			return findAllReferences(_declaration, _declaration->name(), _sourceUnit);
		else
			return {};
	}

	void findAllReferences(
		frontend::Declaration const* _declaration,
		std::string const& _sourceIdentifierName,
		frontend::SourceUnit const& _sourceUnit,
		std::string const& _sourceUnitUri,
		std::vector<::lsp::Location>& _output
	);

	void findAllReferences(
		frontend::Declaration const* _declaration,
		frontend::SourceUnit const& _sourceUnit,
		std::string const& _sourceUnitUri,
		std::vector<::lsp::Location>& _output
	)
	{
		if (!_declaration)
			return;

		findAllReferences(_declaration, _declaration->name(), _sourceUnit, _sourceUnitUri, _output);
	}

private:
	/// In-memory filesystem for each opened file.
	///
	/// Closed files will not be removed as they may be needed for compiling.
	::lsp::vfs::VFS m_vfs;

	std::unique_ptr<FileReader> m_fileReader;

	/// map of input files to source code strings
	std::map<std::string, std::string> m_sourceCodes;

	/// Mapping between VFS file and its diagnostics.
	std::map<std::string /*URI*/, std::vector<::lsp::PublishDiagnostics>> m_diagnostics;

	std::unique_ptr<frontend::CompilerStack> m_compilerStack;

	/// Allowed directories
	std::vector<boost::filesystem::path> m_allowedDirectories;

	/// Workspace root directory
	boost::filesystem::path m_basePath;

	/// Configured EVM version that is being used in compilations.
	langutil::EVMVersion m_evmVersion = langutil::EVMVersion::berlin();
};

} // namespace solidity

