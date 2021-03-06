#ifndef TESTWRAPPER_H
#define TESTWRAPPER_H

#include <string>
#include <iostream>
#include <list>
#include <cstdlib>
#include <vector>

#include "AbstractWrapper.h"
#include "Database.h"
#include "EntityTable.h"
#include "Parser.h"
#include "ProgramKnowledgeBase.h"
#include "QueryEvaluator.h"
#include "QueryPreProcessor.h"
#include "QueryObject.h"
#include "RelationshipTable.h"
#include "SemanticsCheck.h"
#include "SIMPLEParser.h"
#include "SIMPLERules.h"
#include "Tnode.h"
#include "ASTView.h"

class TestWrapper : public AbstractWrapper {
 public:
  // default constructor
  TestWrapper();
  
  // destructor
  ~TestWrapper();
  
  ProgramKnowledgeBase pkb;
  QueryEvaluator evaluator;
  // method for parsing the SIMPLE source
  virtual void parse(std::string filename);
  
  // method for evaluating a query
  virtual void evaluate(std::string query, std::list<std::string>& results);
};

#endif
