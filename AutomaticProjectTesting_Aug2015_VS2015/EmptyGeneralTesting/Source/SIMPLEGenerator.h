#pragma once

#include <string>
#include <vector>

using namespace std;

void generateSIMPLE();
void generateProgram(int procedures, int nesting, int assign, int brackets);
vector<string>generateAssigns(int assign, int brackets);