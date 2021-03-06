#pragma once
#include <stdio.h>
#include "EntityTable.h"
#include "QueryObject.h"
#include "SemanticsCheck.h"
#include "RelationshipTable.h"

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <regex>

using namespace std;

class QueryPreProcessor
{
	vector<QueryObject> queryList;
	vector<string> entityList;
	EntTable entityTable;

public:
	bool query(string s);
	EntTable getEntityTable();
	vector<string> getSelectEntities();
	vector<QueryObject> getQueries();
	void clearAll();
	static bool sortQueries(QueryObject obj1, QueryObject obj2);


private:
	vector<string> checkForBracketsAndComma(vector<string> argVector);
	void addQueryObject(vector<string> temp);
	bool inputEntitiesIntoTable(vector<string> v);
	vector<string> split(string s, string delim);
	bool verifySuchThatQuery(vector<string> temp);
	bool verifyPatternQuery(vector<string> temp);
	vector<string> mergeQuotations(vector<string> temp);
	vector<string> removeAndTokens(vector<string> temp);
	vector<string> formatWithQuery(vector<string> temp);
	bool verifyWithQuery(vector<string> temp);
	string getTypeOfRef(string s);
	vector<string> removeAttrRef(vector<string> temp);
	bool addEntities(vector<string> entities);
	void optimizeWithClause(vector<string> temp);
	static string toLowerCase(string s);
	static int getNumUnknownRank(QueryObject obj);

};


