/*
 * BiasedMapEquation.h
 */

#include "BiasedMapEquation.h"
#include "FlowData.h"
#include "NodeBase.h"
#include "../utils/Log.h"
#include "../io/Config.h"
#include <vector>
#include <set>
#include <map>
#include <utility>
#include <cstdlib>

namespace infomap {

double BiasedMapEquation::getModuleCodelength() const {
	// std::cout << "\n$$$$$ getModuleCodelength: " << moduleCodelength << " + " << biasedCost << " = " << moduleCodelength + biasedCost << "\n";
	return moduleCodelength + biasedCost;
};

double BiasedMapEquation::getCodelength() const {
	// std::cout << "\n$$$$$ getCodelength: " << codelength << " + " << biasedCost << " = " << codelength + biasedCost << "\n";
	return codelength + biasedCost;
};

// ===================================================
// IO
// ===================================================

std::ostream& BiasedMapEquation::print(std::ostream& out) const {
	return out << indexCodelength << " + " << moduleCodelength <<
		" + " << biasedCost << " = " <<	io::toPrecision(getCodelength());
}

// std::ostream& operator<<(std::ostream& out, const BiasedMapEquation& mapEq) {
// 	return out << indexCodelength << " + " << moduleCodelength << " = " <<	io::toPrecision(codelength);
// }


// ===================================================
// Init
// ===================================================

void BiasedMapEquation::init(const Config& config)
{
	Log(3) << "BiasedMapEquation::init()...\n";
	preferredNumModules = config.preferredNumberOfModules;
}


void BiasedMapEquation::initNetwork(NodeBase& root)
{
	Log(3) << "BiasedMapEquation::initNetwork()...\n";
	Base::initNetwork(root);
}

void BiasedMapEquation::initSuperNetwork(NodeBase& root)
{
	Base::initSuperNetwork(root);
}

void BiasedMapEquation::initSubNetwork(NodeBase& root)
{
	Base::initSubNetwork(root);
}

void BiasedMapEquation::initPartition(std::vector<NodeBase*>& nodes)
{
	calculateCodelength(nodes);
}


// ===================================================
// Codelength
// ===================================================

double BiasedMapEquation::calcNumModuleCost(unsigned int numModules) const
{
	if (preferredNumModules == 0) return 0;
	int deltaNumModules = numModules - preferredNumModules;
	return 1 * std::abs(deltaNumModules);
}

void BiasedMapEquation::calculateCodelength(std::vector<NodeBase*>& nodes)
{
	calculateCodelengthTerms(nodes);

	calculateCodelengthFromCodelengthTerms();

	currentNumModules = nodes.size();

	biasedCost = calcNumModuleCost(currentNumModules);
}

double BiasedMapEquation::calcCodelength(const NodeBase& parent) const
{
	return parent.isLeafModule() ?
		calcCodelengthOnModuleOfLeafNodes(parent) :
		// Use first-order model on index codebook
		MapEquation::calcCodelengthOnModuleOfModules(parent);
}

double BiasedMapEquation::calcCodelengthOnModuleOfLeafNodes(const NodeBase& parent) const
{
	double indexLength = MapEquation::calcCodelength(parent);

	// double biasedCost	= calcNumModuleCost(parent.childDegree())
	// std::cout << "\n!!!!! calcCodelengthOnModuleOfLeafNodes(parent) -> biasedCost: " << biasedCost << "\n";

	// return indexLength + biasedCost;
	return indexLength;
}

int BiasedMapEquation::getDeltaNumModulesIfMoving(NodeBase& current,
			unsigned int oldModule, unsigned int newModule, std::vector<unsigned int>& moduleMembers) const
{
	bool removeOld = moduleMembers[oldModule] == 1;
	bool createNew = moduleMembers[newModule] == 0;
	int deltaNumModules = removeOld && !createNew ? -1 : (!removeOld && createNew ? 1 : 0);
	return deltaNumModules;
}

double BiasedMapEquation::getDeltaCodelengthOnMovingNode(NodeBase& current,
		DeltaFlowDataType& oldModuleDelta, DeltaFlowDataType& newModuleDelta, std::vector<FlowDataType>& moduleFlowData, std::vector<unsigned int>& moduleMembers)
{
	double deltaL = Base::getDeltaCodelengthOnMovingNode(current, oldModuleDelta, newModuleDelta, moduleFlowData, moduleMembers);

	if (preferredNumModules == 0)
		return deltaL;

	int deltaNumModules = getDeltaNumModulesIfMoving(current, oldModuleDelta.module, newModuleDelta.module, moduleMembers);

	double deltaBiasedCost = calcNumModuleCost(currentNumModules + deltaNumModules) - biasedCost;

	// std::cout << "\n!!!!! getDeltaCodelengthOnMovingNode(" << current.stateId << ") from " <<
	// 	oldModule << " (" << moduleMembers[oldModule] << ") to " <<
	// 	newModule << " (" << moduleMembers[newModule] << ") -> currentNumModules = " <<
	// 	currentNumModules << " + " << deltaNumModules << " => cost: " <<
	// 	biasedCost << " + " << deltaBiasedCost << " = " << (biasedCost + deltaBiasedCost) << "\n";

	return deltaL + deltaBiasedCost;
}


// ===================================================
// Consolidation
// ===================================================

void BiasedMapEquation::updateCodelengthOnMovingNode(NodeBase& current,
		DeltaFlowDataType& oldModuleDelta, DeltaFlowDataType& newModuleDelta, std::vector<FlowDataType>& moduleFlowData, std::vector<unsigned int>& moduleMembers)
{
	Base::updateCodelengthOnMovingNode(current, oldModuleDelta, newModuleDelta, moduleFlowData, moduleMembers);

	if (preferredNumModules == 0)
		return;

	int deltaNumModules = getDeltaNumModulesIfMoving(current, oldModuleDelta.module, newModuleDelta.module, moduleMembers);

	// double deltaBiasedCost = calcNumModuleCost(currentNumModules + deltaNumModules) - biasedCost;

	// std::cout << "\n!!!!! updateCodelengthOnMovingNode(" << current.stateId << ") from " <<
	// 	oldModule << " (" << moduleMembers[oldModule] << ") to " <<
	// 	newModule << " (" << moduleMembers[newModule] << ") -> currentNumModules = " <<
	// 	currentNumModules << " + " << deltaNumModules << " => cost: " <<
	// 	biasedCost << " + " << deltaBiasedCost << " = " << (biasedCost + deltaBiasedCost) << "\n";

	// biasedCost += deltaBiasedCost;

	currentNumModules += deltaNumModules;
	biasedCost = calcNumModuleCost(currentNumModules);

}


void BiasedMapEquation::consolidateModules(std::vector<NodeBase*>& modules)
{
	unsigned int numModules = 0;
	for (auto& module : modules) {
		if (module == nullptr)
			continue;
		++numModules;
	}
	currentNumModules = numModules;
}



// ===================================================
// Debug
// ===================================================

void BiasedMapEquation::printDebug()
{
	std::cout << "BiasedMapEquation\n";
	Base::printDebug();
}


}