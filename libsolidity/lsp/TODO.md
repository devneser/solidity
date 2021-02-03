# MEETING Questions

- ImportDirective does not traverse into SymbolAlias.
	- change that to identify symbol aliases.
	- to find all refs of that symbol alias
	- to get direct correlation between alias use and alias def.

# Milestone Before Merge

- [ ] remapping
- [ ] findAllReferences: include *TYPE* names (not just vars / members)
- [ ] findAllReferences: marks the whole `Enum.Value` instead of just `Value` keep or change?
- [ ] AST: add location for the actual type names? (so that every type X can be highlighted, if X is non-trivial)

- [ ] Q: the documents opened via didOpen should probably also added to fullPathMapping?
- [ ] make sure remappings are working
- [ ] REVIEW: test windows resolving file urls in client
- [ ] QA: file changes: move into lsp::Server out of the solidity lsp
- [ ] LSP seems to support Location[] as response. Try that!
- [ ] cursor on a function def name -> find all refs -> yields no calls to this function
- [ ] Hover support (showing underlying type + formatted Natspec documentation)
- [ ] configurable compiler settings
- [x] compiler diagnostics
- [x] goto definition
- [x] add proper name locations in AST (in separate pr)
- [x] finished semantic highlighting
- [x] find all references
- [x] extracting FileReader into own pr for shared use in solc
- [x] migrating lsp into solc
- [x] early return in fn bodies

# Postponed for after merge (separate PR)

- [ ] CODE ACTION: rename symbol
- [ ] CODE ACTION: quickfix (to upgrade language syntax rewrite deprecated syntax)
- [ ] auto completion (on at least `.`) - more complex, due to parser errors bailing out early
- [ ] VScode plugin (separate git repo)

# Motivations

The following features could be implemented (incomplete list of ideas):

- realtime compilation validation (error, warning, formal verification)
- auto completion
- go to definition
- find all references
- auto correction on typical programming issues
- semantic highlying of symbols
- symbol rename
- automatic adding of imports whenever a symbol is referenced that is not yet imported in the current unit.
- signature and natspec documentation information upon symbol hover
  - show generated EVM code for hovered scope
  - show estimated gas cost for hovered expression
- get realtime suggestions to improve code quality, or other reasons
- the solidity upgrade tool could potentially be integrated into the language server
- also support Yul

## Questions:

- what clients would you guys like us to support? Remix, VS, VIM, ...?
- could this be wanted by by Remix?
- what would be most/least important to you to have in general (and in the initial release)
  - compiler diagnostics
  - auto completion (requires some compiler stack refactoring to allow invalid ASTs)
  - goto definition / implementation
  - find all references
  - text folding
  - symbol rename
  - semantic highlight
  - (on-type) text formatting
  - ... fast responding high performant LS
- how do clients want to push their build configuration to the server?
  - C++ LS uses a `compile_commands.json` for the flags
- Since the LS is internally compiling the code, it might as well extend the API by
  also responding to compilation results (on successful builds).
- LSIF: is there interest in "Language Server Index Format"?
  - pronounced "else-if"
  - Ref: https://code.visualstudio.com/blogs/2019/02/19/lsif
  - this is an indexing file format that the LS can produce for offline usage
  - example use: platforms (such as GitHub!) could provide goto definition (etc) features

# What an initial release could look like

## Features:

- [x] basic compilation checking with reporting errors to the client
- [x] goto definition
- [x] semantic highlighting of the currently selected word
- [ ] hover support (showing signature, maybe natspec docs already?)
- [ ] auto completion upon writing `.` or `(` or `,` (NO. after initial release?)

## Internal Requirements

- [ ] hover-information (signature + natspec documentation)
- [ ] perform validation not after every change but
  - either if there was no text change notification for a given timeout, or
  - if a bigger timeout has been exceeded and no revalidation has been taken place since that period of time yet.

## methods implementation status

- [ ] client/registerCapability
- [ ] client/unregisterCapability
- [ ] codeLens/resolve
- [ ] completionItem/resolve (-> method)
- [ ] documentLink/resolve
- [x] exit (-> notification)
- [x] initialize (-> method)
- [x] initialized (-> notification)
- [x] shutdown (-> notification)
- [ ] telemetry/event
- [ ] textDocument/codeAction
- [ ] textDocument/codeLens
- [ ] textDocument/colorPresentation
- [ ] textDocument/completion (-> method)
- [ ] textDocument/declaration (-> method) XXX
- [x] textDocument/definition (-> method) XXX
- [x] textDocument/didChange (-> notification)
- [x] textDocument/didClose (-> notification)
- [x] textDocument/didOpen (-> notification)
- [ ] textDocument/didSave (-> notification)
- [ ] textDocument/documentColor
- [ ] textDocument/documentHighlight (-> method) XXX
- [ ] textDocument/documentLink
- [ ] textDocument/documentSymbol (-> method)
- [ ] textDocument/foldingRange
- [ ] textDocument/formatting
- [ ] textDocument/hover (-> method) XXX
- [ ] textDocument/implementation (-> method) XXX
- [ ] textDocument/onTypeFormatting
- [ ] textDocument/prepareRename
- [ ] textDocument/publishDiagnostics (<- notification) XXX
- [ ] textDocument/rangeFormatting
- [x] textDocument/references (-> method)
- [ ] textDocument/rename
- [ ] textDocument/selectionRange
- [ ] textDocument/signatureHelp (-> method) XXX
- [ ] textDocument/typeDefinition (-> method) XXX
- [ ] textDocument/willSave (-> notification)
- [ ] textDocument/willSaveWaitUntil (-> method)
- [x] window/logMessage (<- notification)
- [ ] window/showMessage
- [ ] window/showMessageRequest
- [ ] window/workDoneProgress/cancel
- [ ] window/workDoneProgress/create
- [ ] workspace/applyEdit
- [ ] workspace/configuration
- [ ] workspace/didChangeConfiguration (-> method)
- [ ] workspace/didChangeWatchedFiles
- [ ] workspace/didChangeWorkspaceFolders (-> notification)
- [ ] workspace/executeCommand
- [ ] workspace/symbol

