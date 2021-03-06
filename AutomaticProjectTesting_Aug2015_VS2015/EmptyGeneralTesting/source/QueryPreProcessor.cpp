#pragma once

#include "QueryPreProcessor.h"

SemanticsCheck sCheck;
EntTable entityTable;
vector<string> resultList;
vector<QueryObject> queryList;
RelationshipTable relTable;

bool QueryPreProcessor::sortQueries(QueryObject obj1, QueryObject obj2) {
	if (getNumUnknownRank(obj1) < getNumUnknownRank(obj2)) {
		return true;
	}
	else if (getNumUnknownRank(obj1) == getNumUnknownRank(obj2)) {
		return obj1.getDifficultyRank() < obj2.getDifficultyRank();
	}
	else {
		return false;
	}
}

string QueryPreProcessor::toLowerCase(string s) {
	transform(s.begin(), s.end(), s.begin(), ::tolower);
	return s;
}

int QueryPreProcessor::getNumUnknownRank(QueryObject obj) {
	string arg1 = obj.getRelation();
	int numUnknowns = obj.getNumUnknowns();

	if (numUnknowns == 0) {
		return 1;
	}

	if (toLowerCase(arg1) == "affects*") {
		if (numUnknowns == 2) {
			return 7;
		}
		else {
			return 6;
		}
	}
	else if (toLowerCase(arg1) == "affects" || toLowerCase(arg1) == "next*") {
		if (numUnknowns == 2) {
			return 5;
		}
		else {
			return 4;
		}
	}
	else {
		if (numUnknowns == 2) {
			return 3;
		}
		else if (numUnknowns ==1) {
			return 2;
		}
	}
}

vector<string> QueryPreProcessor::split(string s, string delim) {
	stringstream stringStream(s);
	string line;
	vector<string> wordVector;
	while (getline(stringStream, line))
	{
		size_t prev = 0, pos;
		while ((pos = line.find_first_of(delim, prev)) != string::npos)
		{
			if (pos > prev)
				wordVector.push_back(line.substr(prev, pos - prev));
			prev = pos + 1;
		}
		if (prev < line.length())
			wordVector.push_back(line.substr(prev, string::npos));
	}
	return wordVector;
}

bool QueryPreProcessor::inputEntitiesIntoTable(vector<string> v) {
	vector<string> designEntities = { "stmt","assign","while","if","procedure","variable","constant","prog_line","call" };

	//Find stmt, assign, while... design-entities and put them into entity table
	for (vector<string>::iterator it = v.begin(); it != v.end(); ++it) {
		string line = *it;
		vector<string> wordVector = split(line, " ,");
		//if first vector of line is equals to stmt, assign, while..proceed to add subsequent variables into entity table
		if (find(designEntities.begin(), designEntities.end(), wordVector[0]) != designEntities.end()) {
			for (size_t i = 1; i < wordVector.size(); i++) {
				if (sCheck.isIdent(wordVector[i])) {
					if (entityTable.exist(wordVector[i])) {
						return false;
					}
					else {
						entityTable.add(wordVector[i], wordVector[0]);
					}
				}
			}
		}
	}
	return true;
}

bool QueryPreProcessor::verifySuchThatQuery(vector<string> temp) {
	if (temp.size() != 3) {
		cout << "querySTObj does not have 3 arguments" << endl;
		return false;
	}

	vector<string> definedArg = relTable.getArguments(toLowerCase(temp[0]));
	if (sCheck.initSemanticsCheck((temp[1]), definedArg[0], entityTable) && sCheck.initSemanticsCheck(temp[2], definedArg[1], entityTable)) {
		string relType = toLowerCase(temp[0]);
		string arg1 = temp[1];
		string arg2 = temp[2];
		if (relType.compare("modifies") == 0 || relType.compare("uses") == 0) {
			if (arg1.compare("_") == 0) {
				return false;
			}
			if (sCheck.isSynonym(arg2, entityTable) && !sCheck.isSynVariable(arg2, entityTable)) {
				return false;
			}
		}
		else if (relType.compare("calls") == 0 || relType.compare("calls*") == 0) {
			if (sCheck.isSynonym(arg1, entityTable) && !sCheck.isSynProcedure(arg1, entityTable)) {
				return false;
			}
			if (sCheck.isSynonym(arg2, entityTable) && !sCheck.isSynProcedure(arg2, entityTable)) {
				return false;
			}
		}
		else if (relType.compare("next") == 0 || relType.compare("next*") == 0) {
			if (sCheck.isSynonym(arg1, entityTable) && !sCheck.isSynProgLine(arg1, entityTable)) {
				return false;
			}
			if (sCheck.isSynonym(arg2, entityTable) && !sCheck.isSynProgLine(arg2, entityTable)) {
				return false;
			}
		}
		else if (relType.compare("affects") == 0 || relType.compare("affects*") == 0) {
			if (sCheck.isSynonym(arg1, entityTable) && !sCheck.isSynAssign(arg1, entityTable)) {
				return false;
			}
			if (sCheck.isSynonym(arg2, entityTable) && !sCheck.isSynAssign(arg2, entityTable)) {
				return false;
			}
		}
		else if (relType.compare("parent") == 0 || relType.compare("parent*") == 0 || relType.compare("follows") == 0 || relType.compare("follows*") == 0) {
			if (sCheck.isSynonym(arg1, entityTable) && sCheck.isSynProcedure(arg1, entityTable)) {
				return false;
			}
			if (sCheck.isSynonym(arg2, entityTable) && sCheck.isSynProcedure(arg2, entityTable)) {
				return false;
			}
		}
		return true;
	}
	else {
		cout << "relationship arguments mismatch" << endl;
		return false;
	}
}

bool QueryPreProcessor::verifyPatternQuery(vector<string> temp) {
	if (temp.size() != 3) {
		cout << "queryPatternObj does not have 3 argument:";
		return false;
	}

	//assign pattern
	if (sCheck.isSynAssign(temp[0], entityTable)) {
		if (sCheck.isVarRef(temp[1], entityTable)) {
			if (sCheck.isExpressionSpec(temp[2])) {
				return true;
			}
		}
	}
	//if pattern
	else if (sCheck.isSynIf(temp[0], entityTable)) {
		if (sCheck.isEntRef(temp[1], entityTable)) {
			if (temp[2].compare("_") == 0) {
				return true;
			}
		}
	}
	//while pattern
	else if (sCheck.isSynWhile(temp[0], entityTable)) {
		if (sCheck.isEntRef(temp[1], entityTable)) {
			if (temp[2].compare("_") == 0) {
				return true;
			}
		}
	}
	cout << "pattern arguments mismatch" << endl;
	return false;
}

void QueryPreProcessor::addQueryObject(vector<string> temp) {

	int numUnknowns;
	if ((entityTable.exist(temp[1]) || temp[1] == "_") && (entityTable.exist(temp[2]) || temp[2] == "_")) {
		numUnknowns = 2;
	}
	else if ((entityTable.exist(temp[1]) || temp[1] == "_") || (entityTable.exist(temp[2]) || temp[2] == "_")) {
		numUnknowns = 1;
	}
	else {
		numUnknowns = 0;
	}

	QueryObject qo = QueryObject(temp[0], temp[1], temp[2], numUnknowns);
	queryList.push_back(qo);
}

//Helps to group quotation expressions into one token e.g. "x + y + z" -> "x+y+z"
vector<string> QueryPreProcessor::mergeQuotations(vector<string> temp) {
	vector<string> newVect;
	size_t counter;
	string combine;
	bool flag = false;
	for (size_t i = 0; i < temp.size(); i++) {
		counter = count(temp[i].begin(), temp[i].end(), '\"');
		if (flag == false) {
			if (counter != 1) {
				newVect.push_back(temp[i]);
			}
			else {
				combine = temp[i];
				flag = true;
			}
		}
		//flag==true
		else {
			if (counter != 1) {
				combine = combine + temp[i];
				flag = true;
			}
			//found the next quotation:"
			else {
				combine = combine + temp[i];
				newVect.push_back(combine);
				flag = false;
			}
		}

	}
	return newVect;
}

//takes in rel-clause / pattern-clause to check for ( , ) format before returning a vector of 3 arguments in a form rel, arg1, arg2
//or ( , , ) format for if pattern
vector<string> QueryPreProcessor::checkForBracketsAndComma(vector<string> argVector) {

	string mergedString;
	for (size_t i = 0; i < argVector.size(); i++) {
		mergedString = mergedString + argVector[i];
	}

	// matches delimiters or consecutive non-delimiters
	regex e("([,()]|[^,()]+)");

	regex_iterator<string::iterator> start(mergedString.begin(), mergedString.end(), e);
	regex_iterator<string::iterator> end;

	vector<string> checkVector;
	while (start != end) {
		string it = start->str();
		checkVector.push_back(it);
		++start;
	}

	argVector.clear();
	if (checkVector.size() == 6) {
		if (checkVector[1].compare("(") == 0 && checkVector[3].compare(",") == 0 && checkVector[5].compare(")") == 0) {
			argVector.push_back(checkVector[0]);
			argVector.push_back(checkVector[2]);
			argVector.push_back(checkVector[4]);
			//cout << "Brackets and commas in place" << endl;
		}
		else {
			cout << "missing brackets or commas" << endl;
		}
	}
	//particularly for if pattern i.e. ifstat(_,_,_) is correct, will return a argVect object: (ifstat, "x" , _)
	else if (checkVector.size() == 8) {
		if (checkVector[1].compare("(") == 0 && checkVector[3].compare(",") == 0 && checkVector[5].compare(",") == 0 && checkVector[7].compare(")") == 0) {
			argVector.push_back(checkVector[0]);
			argVector.push_back(checkVector[2]);
			argVector.push_back(checkVector[4]);
			//cout << "Brackets and commas in place" << endl;
		}
		else {
			cout << "missing brackets or commas" << endl;
		}
		if (!(checkVector[6].compare("_") == 0) || !(checkVector[4].compare("_") == 0)) {
			argVector.clear();
		}
	}
	else {
		cout << "num arguments invalid (or) missing brackets/commas" << endl;
	}
	checkVector.clear();
	mergedString.erase();
	return argVector;
}

vector<string> QueryPreProcessor::removeAndTokens(vector<string> temp) {
	vector<string> newVect;
	for (size_t i = 0; i < temp.size(); i++) {
		if (toLowerCase(temp[i]) != "and") {
			newVect.push_back(temp[i]);
		}
	}
	return newVect;
}

vector<string> QueryPreProcessor::formatWithQuery(vector<string> temp) {
	vector<string> newVect;
	string mergedString;
	for (size_t i = 0; i < temp.size(); i++) {
		mergedString = mergedString + temp[i];
	}
	vector<string> withVector = split(mergedString, " =");
	newVect.push_back("with");
	newVect.push_back(withVector[0]);
	newVect.push_back(withVector[1]);
	return newVect;
}

string QueryPreProcessor::getTypeOfRef(string s) {
	string result;
	if (sCheck.isAttrRef(s, entityTable)) {
		vector<string> temp = split(s, ".");
		if (temp[1] == "procName" || temp[1] == "varName") {
			result = "string";
			return result;
		}
		else if (temp[1] == "value" || temp[1] == "stmt#") {
			result = "integer";
			return result;
		}
	}
	else if (sCheck.isProgLine(s, entityTable)) {
		result = "integer";
		return result;
	}
	else if (sCheck.wrappedInQuotation(s)) {
		string substring = s.substr(1, s.length() - 2);
		if (sCheck.isIdent(substring)) {
			result = "string";
			return result;
		}
	}
	else if (sCheck.isInteger(s)) {
		result = "integer";
		return result;
	}
	else {
		result = "errorType";
		return result;
	}
}

bool QueryPreProcessor::verifyWithQuery(vector<string> temp) {
	if (temp.size() != 3) {
		return false;
	}
	string leftAttr = temp[1];
	string rightAttr = temp[2];
	if (sCheck.isRef(leftAttr, entityTable) && sCheck.isRef(rightAttr, entityTable)) {
		string type1 = getTypeOfRef(leftAttr);
		string type2 = getTypeOfRef(rightAttr);
		return type1.compare(type2) == 0;
	}
	else {
		return false;
	}
}

vector<string> QueryPreProcessor::removeAttrRef(vector<string> temp) {
	vector<string> newVect;
	newVect.push_back("with");
	if (sCheck.isAttrRef(temp[1], entityTable)) {
		vector<string> temp1 = split(temp[1], ".");
		newVect.push_back(temp1[0]);
	}
	else {
		newVect.push_back(temp[1]);
	}

	if (sCheck.isAttrRef(temp[2], entityTable)) {
		vector<string> temp2 = split(temp[2], ".");
		newVect.push_back(temp2[0]);
	}
	else {
		newVect.push_back(temp[2]);
	}

	return newVect;
}

bool QueryPreProcessor::addEntities(vector<string> entities) {
	string mergedString;
	for (size_t i = 0; i < entities.size(); i++) {
		mergedString = mergedString + entities[i];
	}

	vector<string> filteredEntities = split(mergedString, "<>, ");
	if (filteredEntities.size() == 1) {
		if (sCheck.isSynonym(filteredEntities.at(0), entityTable) || (toLowerCase(filteredEntities.at(0)).compare("boolean") == 0)) {
			resultList.push_back(filteredEntities.at(0));
		}
		else {
			return false;
		}
	}
	else {
		for (size_t i = 0; i<filteredEntities.size(); i++) {
			if (sCheck.isSynonym(filteredEntities.at(i), entityTable)) {
				resultList.push_back(filteredEntities.at(i));
			}
			else {
				return false;
			}
		}
	}
}

void QueryPreProcessor::optimizeWithClause(vector<string> temp) {

	if (sCheck.isSynonym(temp[2], entityTable) || queryList.size() == 0) {
		addQueryObject(temp);
	}
	else if (queryList.size() != 0) {
		QueryObject lastQueryObj = queryList[queryList.size() - 1];
		if ((lastQueryObj.getFirstArgument()).compare(temp[1]) == 0) {
			QueryObject newObj = QueryObject(lastQueryObj.getRelation(), temp[2], lastQueryObj.getSecondArgument(), lastQueryObj.getNumUnknowns()-1);
			queryList.pop_back();
			queryList.push_back(newObj);
			addQueryObject(temp);

		}
		else if ((lastQueryObj.getSecondArgument()).compare(temp[1]) == 0) {
			QueryObject newObj = QueryObject(lastQueryObj.getRelation(), lastQueryObj.getFirstArgument(), temp[2], lastQueryObj.getNumUnknowns()-1);
			queryList.pop_back();
			queryList.push_back(newObj);
			addQueryObject(temp);
		}
		else {
			addQueryObject(temp);
		}
	}

}

bool QueryPreProcessor::query(string s) {
	//Initialize relationship table
	relTable.initRelTable();
	clearAll();

	//Create the design-entity(s) table by tokenizing string with delimiter ; and \n. Make sure that all subsequent synonyms used in a query are being declared.
	vector<string> temp;
	temp = split(s, ";\n");
	if (!inputEntitiesIntoTable(temp)) {
		return false;
	}
	//cout << "Parsed design-entities into table" << endl;

	//Work on relationship Query. Focusing on the 'Select...' line
	//cout << "Verify line: " + v.back() << endl;
	string newS = temp.back();
	vector<string> tempSelectCl = split(newS, " ");

	vector<string> selectCl = mergeQuotations(tempSelectCl);
	vector<string> entities;
	//first must be a Select, else return false
	if (toLowerCase(selectCl.at(0)).compare("select") == 0) {
		//cout << "'Select' found" << endl;
		size_t i = 1;

		//second phase must be a result-clause synonym (<tuple>, boolean), verify synonym, else false
		while (!(toLowerCase(selectCl.at(i)).compare("such") == 0 || toLowerCase(selectCl.at(i)).compare("pattern") == 0 || toLowerCase(selectCl.at(i)).compare("with") == 0)) {
			entities.push_back(selectCl.at(i));
			i++;
			if (selectCl.size() == i) {
				break;
			}
		}
		if (i == 1) {
			cout << "no result-clause found" << endl;
			return false;
		}
		else {
			bool entitiesDeclared = addEntities(entities);
			if (!entitiesDeclared) {
				cout << "result-clause not declared or invalid" << endl;
				return false;
			}
		}

		//third phase: followed by suchthat | pattern | with, else error
		vector<string> argVector;
		vector<string> queryVector;
		vector<string> queryVectorForWith;
		while (i < selectCl.size()) {

			//find suchthat-cl
			if (toLowerCase(selectCl.at(i)).compare("such") == 0) {
				i++;
				if (toLowerCase(selectCl.at(i)).compare("that") == 0) {
					i++;
					if (selectCl.size() == i) {
						return false; // cater for 'assign a; Select a such that' being a valid when it should be invalid
					}
					//extract relCond
					while (!(toLowerCase(selectCl.at(i)).compare("such") == 0 || toLowerCase(selectCl.at(i)).compare("pattern") == 0
						|| toLowerCase(selectCl.at(i)).compare("and") == 0 || toLowerCase(selectCl.at(i)).compare("with") == 0)) {
						argVector.push_back(selectCl.at(i));
						i++;
						if (selectCl.size() == i) {
							break;
						}
					}

					queryVector = checkForBracketsAndComma(argVector);
					if (verifySuchThatQuery(queryVector)) {
						addQueryObject(queryVector);
						queryVector.clear();
						argVector.clear();
					}
					else {
						//i.e. "... such that a.stmt#=7..."
						//if '=' is find in a such that clause, check if it is a with clause
						bool equalSignFound = false;
						for (size_t i = 0; i < argVector.size(); i++) {
							size_t found = argVector[i].find('=');
							if (found != std::string::npos) {
								queryVectorForWith = formatWithQuery(argVector);
								equalSignFound = true;
								break;
							}
						}
						if (equalSignFound) {
							if (verifyWithQuery(queryVectorForWith)) {
								optimizeWithClause(removeAttrRef(queryVectorForWith));
								queryVectorForWith.clear();
								argVector.clear();
							}
						}
						else {
							cout << "invalid such that query" << endl;
							queryVector.clear();
							queryVectorForWith.clear();
							argVector.clear();
							return false;
						}
					}

				}
				else {
					cout << "found 'such' but no 'that'";
					return false;
				}

			}
			//find pattern-cl
			else if (toLowerCase(selectCl.at(i)).compare("pattern") == 0) {
				i++;
				if (selectCl.size() == i) {
					return false; // cater for 'assign a; Select a pattern' being a valid when it should be invalid
				}
				//extract patternCond
				while (!(toLowerCase(selectCl.at(i)).compare("such") == 0 || toLowerCase(selectCl.at(i)).compare("pattern") == 0
					|| toLowerCase(selectCl.at(i)).compare("and") == 0 || toLowerCase(selectCl.at(i)).compare("with") == 0)) {
					argVector.push_back(selectCl.at(i));
					i++;
					if (selectCl.size() == i) {
						break;
					}
				}
				queryVector = checkForBracketsAndComma(argVector);
				if (verifyPatternQuery(queryVector)) {
					addQueryObject(queryVector);
					queryVector.clear();
					argVector.clear();
				}
				else {
					cout << "invalid pattern query" << endl;
					queryVector.clear();
					argVector.clear();
					return false;
				}

			}
			//find with-cl
			else if (toLowerCase(selectCl.at(i)).compare("with") == 0) {
				i++;
				if (selectCl.size() == i) {
					return false; // cater for 'assign a; Select a with' being a valid when it should be invalid
				}
				while (!(toLowerCase(selectCl.at(i)).compare("such") == 0 || toLowerCase(selectCl.at(i)).compare("pattern") == 0
					|| toLowerCase(selectCl.at(i)).compare("and") == 0 || toLowerCase(selectCl.at(i)).compare("with") == 0)) {
					argVector.push_back(selectCl.at(i));
					i++;
					if (selectCl.size() == i) {
						break;
					}
				}
				queryVector = formatWithQuery(argVector);
				if (verifyWithQuery(queryVector)) {
					optimizeWithClause(removeAttrRef(queryVector));
					queryVector.clear();
					argVector.clear();
				}
				else {
					cout << "invalid with query" << endl;
					queryVector.clear();
					argVector.clear();
					return false;
				}

			}
			//"and" is present, next clause can be a suchthat-cl or pattern-cl
			else if (toLowerCase(selectCl.at(i)).compare("and") == 0) {
				i++;
				if (selectCl.size() == i) {
					return false; // cater for 'assign a; Select a and' being a valid when it should be invalid
				}
				//i.e. and Pattern a(...
				if (toLowerCase(selectCl.at(i)).compare("pattern") == 0) {
					i++;
					if (selectCl.size() == i) {
						return false; // cater for 'assign a; Select a and pattern' being a valid when it should be invalid
					}
					//extract patternCond
					while (!(toLowerCase(selectCl.at(i)).compare("such") == 0 || toLowerCase(selectCl.at(i)).compare("pattern") == 0
						|| toLowerCase(selectCl.at(i)).compare("and") == 0 || toLowerCase(selectCl.at(i)).compare("with") == 0)) {
						argVector.push_back(selectCl.at(i));
						i++;
						if (selectCl.size() == i) {
							break;
						}
					}
					queryVector = checkForBracketsAndComma(argVector);
					if (verifyPatternQuery(queryVector)) {
						addQueryObject(queryVector);
						queryVector.clear();
						argVector.clear();
					}
					else {
						cout << "invalid pattern query" << endl;
						queryVector.clear();
						argVector.clear();
						return false;
					}

				}
				//i.e. and such that Parent(...
				else if (toLowerCase(selectCl.at(i)).compare("such") == 0) {
					i++;
					if (toLowerCase(selectCl.at(i)).compare("that") == 0) {
						i++;
						if (selectCl.size() == i) {
							return false; // cater for 'assign a; Select a and such that' being a valid when it should be invalid
						}
						//extract relCond
						while (!(toLowerCase(selectCl.at(i)).compare("such") == 0 || toLowerCase(selectCl.at(i)).compare("pattern") == 0
							|| toLowerCase(selectCl.at(i)).compare("and") == 0 || toLowerCase(selectCl.at(i)).compare("with") == 0)) {
							argVector.push_back(selectCl.at(i));
							i++;
							if (selectCl.size() == i) {
								break;
							}
						}
						queryVector = checkForBracketsAndComma(argVector);
						if (verifySuchThatQuery(queryVector)) {
							addQueryObject(queryVector);
							queryVector.clear();
							argVector.clear();
						}
						else {
							cout << "invalid such that query" << endl;
							queryVector.clear();
							argVector.clear();
							return false;
						}
					}
					else {
						cout << "found 'such' but no 'that'";
						return false;
					}
				}
				else {
					//i.e. and Parent(...
					//i.e. and a2( , )
					while (!(toLowerCase(selectCl.at(i)).compare("such") == 0 || toLowerCase(selectCl.at(i)).compare("pattern") == 0
						|| toLowerCase(selectCl.at(i)).compare("and") == 0 || toLowerCase(selectCl.at(i)).compare("with") == 0)) {
						argVector.push_back(selectCl.at(i));
						i++;
						if (selectCl.size() == i) {
							break;
						}
					}
					queryVector = checkForBracketsAndComma(argVector);
					if (verifySuchThatQuery(queryVector)) {
						addQueryObject(queryVector);
						queryVector.clear();
						argVector.clear();
					}
					else if (verifyPatternQuery(queryVector)) {
						addQueryObject(queryVector);
						queryVector.clear();
						argVector.clear();
					}
					else {
						cout << "invalid such that / pattern query" << endl;
						queryVector.clear();
						argVector.clear();
						return false;
					}

				}
			}
			else {
				cout << "no suchthat-cl / pattern-cl / with-cl found" << endl;
			}
		}

	}
	else {
		cout << "no 'Select' found" << endl;
		return false;
	}

	//Query Done
	cout << "Query is valid" << endl;
	sort(queryList.begin(), queryList.end(), sortQueries);
	return true;
}

EntTable QueryPreProcessor::getEntityTable() {
	return entityTable;
}

vector<string> QueryPreProcessor::getSelectEntities() {
	return resultList;
}

vector<QueryObject> QueryPreProcessor::getQueries() {
	return queryList;
}

void QueryPreProcessor::clearAll() {
	entityTable.clear();
	resultList.clear();
	queryList.clear();
}
