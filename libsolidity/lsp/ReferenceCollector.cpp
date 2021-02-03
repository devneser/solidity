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
#include <libsolidity/lsp/ReferenceCollector.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/lsp/LanguageServer.h>

using std::vector;
using std::string;

using namespace std::string_literals;
using namespace solidity::frontend;

namespace solidity::lsp
{

ReferenceCollector::ReferenceCollector(
	frontend::Declaration const& _declaration,
	std::string const& _sourceIdentifierName
):
	m_declaration{_declaration},
	m_sourceIdentifierName{_sourceIdentifierName.empty() ? _declaration.name() : _sourceIdentifierName}
{
	fprintf(stderr, "ReferenceCollector.ctor: decl.name='%s', source='%s'\n",
			_declaration.name().c_str(),
			_declaration.location().text().c_str());
}

std::vector<::lsp::DocumentHighlight> ReferenceCollector::collect(
	frontend::Declaration const& _declaration,
	frontend::ASTNode const& _ast,
	std::string const& _sourceIdentifierName
)
{
	auto collector = ReferenceCollector(_declaration, _sourceIdentifierName);
	_ast.accept(collector);
	return collector.take();
}

bool ReferenceCollector::visit(frontend::ImportDirective const& _import)
{
	for (auto const& symbolAlias: _import.symbolAliases())
	{
		fprintf(stderr, "ReferenceCollector.ImportDirective.UnitAlias: name=%s\n",
				symbolAlias.alias->c_str());
		if (m_sourceIdentifierName == *symbolAlias.alias)
		{
			addReference(symbolAlias.location);
			return true;
		}
	}
	return visitNode(_import);
}

bool ReferenceCollector::visit(frontend::Identifier const& _identifier)
{
	// TODO: also check for candidateDeclarations and overloadedDeclarations.
	auto ref = _identifier.annotation().referencedDeclaration;
	fprintf(stderr, "ReferenceCollector.visit(Identifier): %s (%s)\n",
			_identifier.name().c_str(),
			_identifier.annotation().referencedDeclaration
				? typeid(*ref).name()
				: "no type");

	if (auto const declaration = _identifier.annotation().referencedDeclaration; declaration)
		if (declaration == &m_declaration)
			addReference(_identifier.location(), "(identifier)");

	return visitNode(_identifier);
}

bool ReferenceCollector::visit(frontend::MemberAccess const& _memberAccess)
{
	// TODO: MemberAccess.annotation.referencedDeclaration is always NULL, why?
	// It should be EnumValue for an enum value.
	fprintf(stderr, "ReferenceCollector.MemberAccess(%s): %s\n",
			_memberAccess.annotation().referencedDeclaration
				? _memberAccess.annotation().referencedDeclaration->name().c_str()
				: "null",
			_memberAccess.memberName().c_str());

	if (_memberAccess.annotation().referencedDeclaration == &m_declaration)
		addReference(_memberAccess.location(), "memberAccess("s + _memberAccess.memberName() + ")"s);

	return visitNode(_memberAccess);
}

bool ReferenceCollector::visitNode(frontend::ASTNode const& _node)
{
	fprintf(stderr, "ReferenceCollector.visitNode: %s\n", typeid(_node).name());
	if (&_node == &m_declaration)
	{
		if (auto const* decl = dynamic_cast<Declaration const*>(&_node))
			addReference(decl->nameLocation(), "(visitNode)");
		else
			addReference(_node.location(), "(visitNode)");
	}

	return true;
}

void ReferenceCollector::addReference(solidity::langutil::SourceLocation const& _location, std::string msg)
{
	auto const [startLine, startColumn] = _location.source->translatePositionToLineColumn(_location.start);
	auto const [endLine, endColumn] = _location.source->translatePositionToLineColumn(_location.end);
	auto const locationRange = ::lsp::Range{
		{startLine, startColumn},
		{endLine, endColumn}
	};

	fprintf(stderr, " -> found reference %s at %d:%d .. %d:%d\n",
		msg.c_str(),
		startLine, startColumn,
		endLine, endColumn
	);

	auto highlight = ::lsp::DocumentHighlight{};
	highlight.range = locationRange;
	highlight.kind = ::lsp::DocumentHighlightKind::Text; // TODO: are you being read or written to?

	m_result.emplace_back(highlight);
}

}
