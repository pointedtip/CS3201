#include "ast.h"

/*
//stub main
int main()
{
	cout << "initilizing..." << endl;
	vector<string> tokenized_program = //example
	{ "Procedure", "myProc", "{",
		"if", "x", "then", "{",
		"while", "k", "{",
		"k", "=", "y", "-", "(", "a", "-", "x", ")", ";",
		"}",
		"}",
		"else", "{",
		"call", "sun", ";",
		"f", "=", "(","(","(", "x", ")",")",")", ";",
		"}",
		"}",
		"Procedure", "2ndproc", "{",
		"call", "me", ";",
		"call", "you", ";",
		"}"};
	// Create AST
	ast *a = new ast;
	cout << "building ast..." << endl;
	(*a).buildAST(tokenized_program);
	cout << "ast generated." << endl;
	cout << "printing ast..." << endl;
	(*a).printAST(); ///prints the ast
	cout << "ast printed." << endl;
	(*a).printProcTable();
	(*a).printVarTable();
	return 0;
}
*/

ast::ast()
{
	this -> root = NULL;
	this -> procTable = new vector<pair<string, Tnode*>>;
	this -> varTable = new vector<pair<string, vector<Tnode*>>>;
}


ast::~ast()
{
}

//assume input is of valid SIMPLE language syntax and extra white spaces are removed
void ast::buildAST(vector<string> tokens)
{
	program(tokens);
}

void ast::program(vector<string> &tokens)
{
	vector<string>::iterator it = tokens.begin();
	if (root == NULL) {
		root = Tnode::createNode(Tnode::PROGRAM, *(it+1)); //creates a root node with type:PROGRAM and name of the first procedure
	}
	Tnode::createLink(Tnode::PARENT, *root, *procedure(tokens, it));
	return;
}

//returns procedure node, the first recursion will return topmost node will return first child (i.e. first procedure) of program
Tnode* ast::procedure(vector<string> &tokens, vector<string>::iterator &it)
{
	Tnode *curNode, *nextNode, *curNodeStmtLst;
	match(it, stringify(PROCEDURE));
	curNode = Tnode::createNode(Tnode::PROCEDURE, *it);
	(*procTable).push_back(make_pair(*it, curNode));
	curNodeStmtLst = Tnode::createNode(Tnode::STMTLST, "");
	Tnode::createLink(Tnode::PARENT, *curNode, *curNodeStmtLst);
	match(it, *it);
	match(it, "{");
	Tnode::createLink(Tnode::PARENT, *curNodeStmtLst, *stmtLst(tokens, it)); //set stmtLst's first child
	if (it >= tokens.end()) {
		return curNode;
	}
	else {
		nextNode = procedure(tokens, it);
		Tnode::createLink(Tnode::RIGHTSIB, *curNode, *nextNode);
		return curNode;
	}
}

//return Tnode containing statement type, the first recursion will return first child of current statement list
Tnode* ast::stmtLst(vector<string> &tokens, vector<string>::iterator &it)
{
	Tnode *curNode, *nextNode;
	curNode = stmt(tokens, it);
	if ((*curNode).getType() == Tnode::STMT_ASSIGN || (*curNode).getType() == Tnode::STMT_CALL) {
		match(it, ";");
	}
	if (*it == "}") {
		match(it, *it);
		return curNode;
	}
	else {
		nextNode = stmtLst(tokens, it);
		Tnode::createLink(Tnode::RIGHTSIB, *curNode, *nextNode);
		return curNode;
	}
}

//return Tnode containing statement type
Tnode* ast::stmt(vector<string> &tokens, vector<string>::iterator &it)
{
	Tnode *st;
	if (toUpperCase(*it) == "CALL") {
		match(it, stringify(CALL));
		st = Tnode::createNode(Tnode::STMT_CALL, *it);
		match(it, *it);
	}
	else if (toUpperCase(*it) == "WHILE") {
		match(it, stringify(WHILE));
		st = Tnode::createNode(Tnode::STMT_WHILE, "");
		Tnode::createLink(Tnode::PARENT, *st, *whileSt(tokens, it));
	}
	else if (toUpperCase(*it) == "IF") {
		st = Tnode::createNode(Tnode::STMT_IF, "");
		match(it, stringify(IF));
		Tnode::createLink(Tnode::PARENT, *st, *ifSt(tokens, it));
	}
	else {
		st = Tnode::createNode(Tnode::STMT_ASSIGN, "");
		Tnode::createLink(Tnode::PARENT, *st, *assign(tokens,it));
	}
	return st;
}

//returns first child of while statement i.e. the control variable
Tnode *ast::whileSt(vector<string> &tokens, vector<string>::iterator &it)
{
	Tnode *stLst, *var;
	var = createVariable(Tnode::VARIABLE, *it);
	match(it, *it);
	stLst = Tnode::createNode(Tnode::STMTLST, "");
	Tnode::createLink(Tnode::RIGHTSIB, *var, *stLst);
	match(it, "{");
	Tnode::createLink(Tnode::PARENT, *stLst, *stmtLst(tokens, it));
	return var;
}

//returns first child of if statement i.e. the control variable
Tnode *ast::ifSt(vector<string> &tokens, vector<string>::iterator &it)
{
	Tnode *thenStLst, *elseStLst, *var;
	var = createVariable(Tnode::VARIABLE, *it);
	match(it, *it);
	match(it, "then");
	thenStLst = Tnode::createNode(Tnode::STMTLST, "then");
	Tnode::createLink(Tnode::RIGHTSIB, *var, *thenStLst);
	match(it, "{");
	Tnode::createLink(Tnode::PARENT, *thenStLst, *stmtLst(tokens, it));
	match(it, "else");
	elseStLst = Tnode::createNode(Tnode::STMTLST, "else");
	Tnode::createLink(Tnode::RIGHTSIB, *thenStLst, *elseStLst);
	match(it, "{");
	Tnode::createLink(Tnode::PARENT, *elseStLst, *stmtLst(tokens, it));
	return var;
}

Tnode *ast::assign(vector<string> &tokens, vector<string>::iterator &it)
{
	Tnode *var;
	var = createVariable(Tnode::VARIABLE, *it);
	match(it, *it);
	match(it, "=");
	vector<string>::iterator exprIt = it;
	while (*it != ";") { 
		match(it, *it);
	}
	Tnode::createLink(Tnode::RIGHTSIB, *var, *expr(tokens, exprIt, it-1));
	return var;
}

//start from the back of the expression
//returns the topmost node in the expression AST (either + or - or term node)
Tnode *ast::expr(vector<string> &tokens, vector<string>::iterator start, vector<string>::iterator end)
{
	Tnode *op = NULL, *ex, *t;
	int brackets = 0;
	vector<string>::iterator it = end;
	while (it >= start) {
		if (*it == ")") {
			brackets++;
		}
		else if (*it == "(") {
			brackets--;
		}
		if (brackets == 0) {
			if (*it == "+") {
				op = Tnode::createNode(Tnode::EXPR_PLUS, "");
				ex = expr(tokens, start, it - 1);
				t = term(tokens, it + 1, end);
				Tnode::createLink(Tnode::PARENT, *op, *ex);
				Tnode::createLink(Tnode::RIGHTSIB, *ex, *t);
				break;
			}
			else if (*it == "-") {
				op = Tnode::createNode(Tnode::EXPR_MINUS, "");
				ex = expr(tokens, start, it - 1);
				t = term(tokens, it + 1, end);
				Tnode::createLink(Tnode::PARENT, *op, *ex);
				Tnode::createLink(Tnode::RIGHTSIB, *ex, *t);
				break;
			}
		}
		--it;
	}
	if (op == NULL) {
		op = term(tokens, start, end);
	}
	return op;
}

//returns the topmost node in the term AST (either * or factor node)
Tnode *ast::term(vector<string> &tokens, vector<string>::iterator start, vector<string>::iterator end)
{
	Tnode *op = NULL, *t = NULL, *fac;
	int brackets = 0;
	vector<string>::iterator it = end;
	while (it >= start) {
		if (*it == ")") {
			brackets++;
		}
		else if (*it == "(") {
			brackets--;
		}
		if (brackets == 0) {
			if (*it == "*") {
				op = Tnode::createNode(Tnode::EXPR_TIMES, "");
				t = term(tokens, start, it - 1);
				fac = factor(tokens, it + 1, end);
				Tnode::createLink(Tnode::PARENT, *op, *t);
				Tnode::createLink(Tnode::RIGHTSIB, *t, *fac);
				break;
			}
		}
		--it;
	}
	if (t == NULL) {
		t = factor(tokens, start, end);
	}
	if (op != NULL) {
		return op;
	}
	return t;
}

//returns topmost node in the factor AST (either a variable or constant or expresion==> for "()")
Tnode *ast::factor(vector<string> &tokens, vector<string>::iterator start, vector<string>::iterator end)
{
	Tnode *fac, *ex;
	fac = NULL;
	if (*start == "(" && *end == ")") {
		ex = expr(tokens, start+1, end-1);
		return ex;
	}
	else if (start == end) {
		if (isNum(*start)) {
			fac = Tnode::createNode(stoi(*start));
		}
		else {
			fac = createVariable(Tnode::VARIABLE, *start);
		}
	}
	return fac;
}

Tnode * ast::createVariable(Tnode::Type t, string n)
{
	Tnode* varNode;
	varNode = Tnode::createNode(Tnode::VARIABLE, n);
	addToVarTable(n, varNode);
	return varNode;
}

void ast::addToVarTable(string var, Tnode* varNode)
{
	for (unsigned int i = 0; i < (*varTable).size(); i++) {
		if (get<0>((*varTable).at(i)) == var) {
			get<1>((*varTable).at(i)).push_back(varNode);
			return;
		}
	}
	vector<Tnode*> addressList;
	addressList.push_back(varNode);
	(*varTable).push_back(make_pair(var, addressList));
	return;
}

void ast::match(vector<string>::iterator &it, string token)
{
	if (toUpperCase(*it) == toUpperCase(token)) {
		++it;
	}
	else {
		cout << "Exiting..." << *it << " <> " << token << endl;
		exit(0);
	}
}

void ast::printASTCall(vector<vector<Tnode*>>& nss, vector<Tnode*> s, Tnode * curNode, unsigned int lvl)
{
	if (lvl + 1 > nss.size()) {
		nss.push_back(s);
	}
	nss.at(lvl).push_back(curNode);
	if ((*curNode).getRightSib() != NULL) {
		printASTCall(nss, s, (*curNode).getRightSib(), lvl);
	}
	if ((*curNode).getFirstChild() != NULL) {
		printASTCall(nss, s, (*curNode).getFirstChild(), lvl + 1);
	}
	return;
}

string ast::toUpperCase(string s)
{
	for (unsigned int i = 0; i < s.length(); i++)
	{
		s[i] = toupper(s[i]);
	}
	return s;
}

bool ast::isNum(string &s)
{
	for (unsigned int i = 0; i < s.length(); i++)
	{
		if (!isdigit(s[i])) {
			return false;
		}
	}
	return true;
}

Tnode *ast::getRoot()
{
	return root;
}

vector<pair<string, Tnode*>>* ast::getProcTable()
{
	return procTable;
}

vector<pair<string, vector<Tnode*>>>* ast::getVarTable()
{
	return varTable;
}

void ast::printAST()
{
	vector<vector<Tnode*>> notSoSimple;
	vector<Tnode*> simple;
	printASTCall(notSoSimple, simple, (*this).root, 0);
	for (unsigned int i = 2; i < notSoSimple.size(); i++) {
		simple = vector<Tnode*>();
		for (unsigned int k = 0; k < notSoSimple.at(i-1).size(); k++) {
			for (unsigned int j = 0; j < notSoSimple.at(i).size(); j++) {
				if ((*notSoSimple.at(i).at(j)).getParent() == notSoSimple.at(i - 1).at(k)) {
					while ((*notSoSimple.at(i).at(j)).getRightSib() != NULL) {
						simple.push_back(*(notSoSimple.at(i).begin() + j));
						j++;
					}
					simple.push_back(*(notSoSimple.at(i).begin() + j));
				}
			}
		}
		notSoSimple.at(i) = simple;
	}
	for (unsigned int i = 0; i < notSoSimple.size(); i++) {
		cout << endl << "<---------------------------------------- Level " << i << ": ---------------------------------------->" << endl << endl;
		for (unsigned int j = 0; j < notSoSimple.at(i).size(); j++) {
			cout << "Node " << j << ": " << endl;
			(*notSoSimple.at(i).at(j)).printNode();
			cout << "\n";
		}
	}
}

void ast::printProcTable()
{
	cout << endl << "<---------------------------------------- Procedure Table: ---------------------------------------->" << endl << endl;
	for (vector<pair<string, Tnode*>>::iterator i = (*procTable).begin(); i != (*procTable).end(); i++) {
		cout << "Index :" << (i - (*procTable).begin()) << ", Name: " << (*i).first << ", Address: <" << (*i).second << ">" << endl;
	}
}

void ast::printVarTable()
{
	cout << endl << "<---------------------------------------- Variable Table: ---------------------------------------->" << endl << endl;
	for (vector<pair<string, vector<Tnode*>>>::iterator i = (*varTable).begin(); i != (*varTable).end(); i++) {
		cout << "Index :" << (i - (*varTable).begin()) << ", Name: " << (*i).first << ", Address(es): ";
		for (vector<Tnode*>::iterator j = (*i).second.begin(); j != (*i).second.end(); j++) {
			cout << "<" << (*j) << ">";
			if (j < (*i).second.end() - 1) {
				cout << ", ";
			}
		}
		cout << "\n";
	}
}
