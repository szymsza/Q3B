#ifndef VARIABLEORDERER_H
#define VARIABLEORDERER_H

#include "UnionFind.cpp"
#include <map>
#include <string>
#include <list>
#include <z3++.h>
#include <set>
#include <vector>

typedef std::pair<std::string, int> var;

class VariableOrderer
{
private:
    std::set<var> vars;
    std::map<std::string, int> varIndices;
    UF *unionFind = NULL;
    z3::context* context;

    void MarkDependent(const std::string&, const std::string&);
    void MergeByExpression(const z3::expr&, std::vector<std::string> boundVars);
    std::set<std::string> GetVars(const z3::expr&, std::vector<std::string>);

public:
    VariableOrderer(const std::set<var>&, z3::context&);
    void OrderFor(const z3::expr&);

    std::list<std::list<var>> GetOrdered() const;
};

#endif // VARIABLEORDERER_H