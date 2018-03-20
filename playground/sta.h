#ifndef STA_H
#define STA_H

#include <string>
#include <map>
#include <ectl_types.h>

std::map<Node, int> CalculateSlack(Network ntk);

void KMostCriticalPaths(Network ntk, int k = 10, bool show_slack = false);

std::vector<Node> MinCut(const Network ntk, std::map<Node, double> error);

#endif
