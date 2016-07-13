#include "edgemerger.h"

// PUBLIC
EdgeMerger::EdgeMerger()
{
	deltaPtr = -1;
	oUnUdPtr = -1;
	currID = -1;
}

// getters
std::vector<int> EdgeMerger::getoUnUdEdges() { return srcoUnUdEdges; }
std::vector<char> EdgeMerger::getoUnUdVals() { return srcoUnUdVals; }

std::vector<int> EdgeMerger::getDeltaEdges() { return srcDeltaEdges; }
std::vector<char> EdgeMerger::getDeltaVals() { return srcDeltaVals; }

void EdgeMerger::mergeVectors(std::vector< std::vector<int> > &edgeVecsToMerge,
		std::vector< std::vector<char> > &valVecsToMerge, int srcID)
{
	Timer mergeTime;
	mergeTime.startTimer();

	std::vector<MinSet> minsets(edgeVecsToMerge.size());
	
	int totTgtRowSize = 0;
    for (int i = 0; i < minsets.size(); i++)
    {
        minsets[i].setMinSetID(i);
        updateMinSet(minsets[i], edgeVecsToMerge[i], valVecsToMerge[i]);

        if (i != srcID) {
            totTgtRowSize += edgeVecsToMerge[i].size();
            minEdges.push(minsets[i]);
        }
    }

    srcDeltaEdges = std::vector<int>(totTgtRowSize, -1);
    srcDeltaVals = std::vector<char>(totTgtRowSize, ((char)63));

    srcoUnUdEdges = std::vector<int>(edgeVecsToMerge[srcID].size() + totTgtRowSize, -1);
    srcoUnUdVals = std::vector<char>(edgeVecsToMerge[srcID].size() + totTgtRowSize, ((char)63));

    MinSet src, tgt;
    while (1)
    {
        if (!minEdges.empty()) {
            tgt = minEdges.top();
            minEdges.pop();
        }

        int max = std::numeric_limits<int>::max();
        if (minsets[srcID].getCurrVID() == max && tgt.getCurrVID() == max) {
            break;
        }

        int indOfTgt = tgt.getMinSetID();
        processMinSets(minsets[srcID], tgt, edgeVecsToMerge[srcID], valVecsToMerge[srcID], edgeVecsToMerge[indOfTgt], valVecsToMerge[indOfTgt]);

    }

    removeExtraSpace();

    std::cout << "oldUnewUdelta AFTER update  ->  ";
    for (int k = 0; k < srcoUnUdEdges.size(); k++)
        std::cout << "(" << srcoUnUdEdges[k]  << ", " << srcoUnUdVals[k] << ")  ";
    std::cout << std::endl;
    std::cout << "delta AFTER update -> ";
    for (int l = 0; l < srcDeltaEdges.size(); l++)
        std::cout << "(" << srcDeltaEdges[l] << ", " << srcDeltaVals[l] << ")  ";
    std::cout << std::endl << std::endl;

	mergeTime.endTimer();
}


// PRIVATE
void EdgeMerger::removeExtraSpace()
{
	srcoUnUdEdges = std::vector<int>(srcoUnUdEdges.begin(), srcoUnUdEdges.begin() + oUnUdPtr + 1);
	srcoUnUdVals = std::vector<char>(srcoUnUdVals.begin(), srcoUnUdVals.begin() + oUnUdPtr + 1);

	if (deltaPtr >= 0) {
		srcDeltaEdges = std::vector<int>(srcDeltaEdges.begin(), srcDeltaEdges.begin() + deltaPtr + 1);
        srcDeltaVals = std::vector<char>(srcDeltaVals.begin(), srcDeltaVals.begin() + deltaPtr + 1);
	}
}

void EdgeMerger::updateMinSet(MinSet &minset, std::vector<int> &edges, std::vector<char> &vals)
{
	minset.setCurrVID(std::numeric_limits<int>::max());
	minset.clearEvalSet();

	for (int i = minset.getPtr(); i < edges.size() && edges[i] <= minset.getCurrVID(); i++)
	{
		minset.setCurrVID(edges[i]);
		minset.addEval(vals[i]);
		minset.incPtr();
	}
}

void EdgeMerger::processMinSets(MinSet &srcMS, MinSet &tgtMS, std::vector<int> &srcEdgesToMerge,
		std::vector<char> &srcValsToMerge, std::vector<int> &tgtEdgesToMerge,
		std::vector<char> &tgtValsToMerge)
{
	// case 1
	if (srcMS.getCurrVID() > tgtMS.getCurrVID()) {
        if (currID != tgtMS.getCurrVID()) {
            currID = tgtMS.getCurrVID();
            currEvals.clear();
        }

        std::unordered_set<char> tgtVals = tgtMS.getEvals();
        for (std::unordered_set<char>::iterator iter = tgtVals.begin(); iter != tgtVals.end(); iter++) {
            if (currEvals.find(*iter) == currEvals.end()) {
                oUnUdPtr++;
                if (oUnUdPtr < srcoUnUdEdges.size()) {
                    srcoUnUdEdges[oUnUdPtr] = tgtMS.getCurrVID();
                    srcoUnUdVals[oUnUdPtr] = *iter;
                }
                else std::cout << "ERROR: oUnUdPtr out of bounds" << std::endl;

                deltaPtr++;
                if (deltaPtr < srcDeltaEdges.size()) {
                    srcDeltaEdges[deltaPtr] = tgtMS.getCurrVID();
                    srcDeltaVals[deltaPtr] = *iter;
                    currEvals.insert(*iter);
                }
                else std::cout << "ERROR: deltaPtr out of bounds" << std::endl;
            }
        }
        if (tgtEdgesToMerge.size() > 0) updateMinSet(tgtMS, tgtEdgesToMerge, tgtValsToMerge);
        minEdges.push(tgtMS);

        return;
    }

	// case 2
	if (srcMS.getCurrVID() == tgtMS.getCurrVID()) {
        if (currID != tgtMS.getCurrVID()) {
            currID = tgtMS.getCurrVID();
            currEvals.clear();
        }

        std::unordered_set<char> srcVals = srcMS.getEvals();
        std::unordered_set<char> tgtVals = tgtMS.getEvals();
        for (std::unordered_set<char>::iterator iter = tgtVals.begin(); iter != tgtVals.end(); iter++) {
            if (srcVals.find(*iter) == srcVals.end()) {
                if (currEvals.find(*iter) == currEvals.end()) {
                    oUnUdPtr++;
                    if (oUnUdPtr < srcoUnUdEdges.size()) {
                        srcoUnUdEdges[oUnUdPtr] = tgtMS.getCurrVID();
                        srcoUnUdVals[oUnUdPtr] = *iter;
                    }
                    else std::cout << "ERROR: oUnUdPtr out of bounds" << std::endl;

                    deltaPtr++;
                    if (deltaPtr < srcDeltaEdges.size()) {
                        srcDeltaEdges[deltaPtr] = tgtMS.getCurrVID();
                        srcDeltaVals[deltaPtr] = *iter;
                        currEvals.insert(*iter);
                    }
                    else std::cout << "ERROR: deltaPtr out of bounds" << std::endl;

                }
            }
        }
        if (tgtEdgesToMerge.size() > 0) updateMinSet(tgtMS, tgtEdgesToMerge, tgtValsToMerge);
        minEdges.push(tgtMS);

        return;
    }

	// case 3
	if (srcMS.getCurrVID() < tgtMS.getCurrVID()) {
        if (currID != srcMS.getCurrVID()) {
            currID = srcMS.getCurrVID();
            currEvals.clear();
        }

        std::unordered_set<char> srcVals = srcMS.getEvals();
        for (std::unordered_set<char>::iterator iter = srcVals.begin(); iter != srcVals.end(); iter++) {
            if (currEvals.find(*iter) == currEvals.end()) {
                oUnUdPtr++;
                if (oUnUdPtr < srcoUnUdEdges.size()) {
                    srcoUnUdEdges[oUnUdPtr] = srcMS.getCurrVID();
                    srcoUnUdVals[oUnUdPtr] = *iter;
                    currEvals.insert(*iter);
                }
                else std::cout << "ERROR: oUnUdPtr out of bounds" << std::endl;
            }
        }
        if (srcEdgesToMerge.size() > 0) updateMinSet(srcMS, srcEdgesToMerge, srcValsToMerge);
        minEdges.push(tgtMS);

        return;
    }
}
