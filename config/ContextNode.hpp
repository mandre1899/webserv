#ifndef CONTEXTNODE_HPP
#define CONTEXTNODE_HPP

#include <string>
#include <vector>
#include <iostream>
#include "config/TokenStream.hpp"
#include "utils/Logger.hpp"

enum class ctxNodeIdent
{
	http,
	server,
	location
};

struct ctxNode
{
	ctxNodeIdent conIdent;
	TokenStream param;
	std::vector<ctxNode> child;
};

class ContextNode {
private:
	ctxNode _rootNode;

	/* Member Functions */
	void fillCtx(TokenStream& tokens, ctxNode& ctx);
	void createCtx(TokenStream& tokens);

public:
	/* Constructors & Destructors */
	ContextNode() = delete;
	ContextNode(TokenStream& tokens);
	ContextNode(const ContextNode& src);
	ContextNode(ContextNode&& src);
	~ContextNode();

	/* Member Functions */

	/* Basic Operators */
	ContextNode& operator=(const ContextNode& src);
	ContextNode& operator=(ContextNode&& src);

	/* Getters & Setters */
	const ctxNode& getRootNode() const;
};

std::ostream& operator<<(std::ostream& os, const ctxNodeIdent& ident);
std::ostream& operator<<(std::ostream& os, const ctxNode& ctx);

#endif // CONTEXTNODE_HPP