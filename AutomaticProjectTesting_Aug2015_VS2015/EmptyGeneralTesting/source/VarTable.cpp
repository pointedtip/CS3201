#include "VarTable.h"



VarTable::VarTable()
{
	this->varTable = new unordered_map< string, vector<Tnode*>>;
}


VarTable::~VarTable()
{
}

int VarTable::getSize()
{
	return varTable->size();
}

int VarTable::addVariable(string varName, Tnode *varNode)
{
	if (varTable->find(varName) != varTable->end()) {
		(varTable->at(varName)).push_back(varNode);
	}
	else {
		vector<Tnode*> listNode = {varNode};
		varTable->insert({ varName, listNode});
	}
	return distance(varTable->begin(), varTable->find(varName));
}

string VarTable::getVariableName(int i)
{
	if ((unsigned)i >= varTable->size()) {
		return "";
	}
	else {
		auto j = varTable->begin();
		while (i != distance(varTable->begin(), j)) {
			j++;
		}
		return j->first;
	}
}

int VarTable::getVariableIndex(string varName)
{
	if (varTable->find(varName) != varTable->end()) {
		return distance(varTable->begin(), varTable->find(varName));
	}
	else {
		return -1;
	}
}

vector<Tnode*>* VarTable::getVariableAddress(int i)
{
	return getVariableAddress(getVariableName(i));
}

vector<Tnode*>* VarTable::getVariableAddress(string varName)
{
	try {
		return &(varTable->at(varName));
	}
	catch (...) {
		return NULL;
	}
}

void VarTable::printVariableTable()
{
	cout << endl << "<---------------------------------------- Variable Table: ----------------------------------------> Size: " << varTable->size() << endl << endl;
	for (auto i = (*varTable).begin(); i != (*varTable).end(); i++) {
		cout << "Index :" << distance(varTable->begin(), i) << ", Name: " << (*i).first << ", Address: ";
		printNodeVector((*i).second);
	}
}

void VarTable::printNodeVector(vector<Tnode*> list)
{
	for (auto i =list.begin(); i != list.end(); i++) {
		cout << "<" << (*i) << ">";
		if (i < list.end() - 1) {
			cout << ", ";
		}
	}
	cout << "\n";
}
