#include "utils/components.h"
#include "utils/interval_rep_utils.h"
#include "data_structures/interval.h"

#include <set>
#include <stack>
#include <algorithm>

#include <iostream>

namespace cg::components
{
    // This is the naive algorithm for computing the connected components of a circle graph, requiring O(n^2) time and space.
    std::vector<std::vector<cg::data_structures::Interval>> getConnectedComponentsNaive(std::span<const cg::data_structures::Interval> intervals)
    {
        cg::utils::verifyIndicesDense(intervals);
     
        // Just construct the entire adjacency matrix and then DFS.
        std::vector<std::vector<bool>> isAdjacent(intervals.size(), std::vector<bool>(intervals.size()));

        for(const auto& i : intervals)
        {
            for(const auto& j : intervals)
            {
                isAdjacent[i.Index][j.Index] = i.Index != j.Index && i.overlaps(j);
            }
        }

        std::vector<bool> visited(intervals.size(), false);
        std::stack<cg::data_structures::Interval> pending;
        std::vector<std::vector<cg::data_structures::Interval>> components;
        for(const auto& i : intervals)
        {
            if(!visited[i.Index])
            {
                auto currentComponent = std::vector<cg::data_structures::Interval>();
                pending.push(i);
                visited[i.Index] = true;
                while(!pending.empty())
                {
                    auto j = pending.top();
                    pending.pop();
                    currentComponent.push_back(j);
                    for(const auto& k : intervals)
                    {
                        if(isAdjacent[j.Index][k.Index] && !visited[k.Index])
                        {
                            visited[k.Index] = true;
                            pending.push(k);
                        }
                    }
                }
                components.push_back(std::move(currentComponent));
            }
        }
        return components;
    }

    // This is the simple O(n \log^2 n) time algorithm to find the connected components of a circle graph given an interval model.
    //
    // "Subquadratic Time Algorithm to Find the Connected Components of Circle Graphs"
    // Kim Jae-hoon, Korea Institute of Information and Communication Engineering, v.22 no.11, 2018, pp.1538 - 1543
    //
    std::vector<std::vector<cg::data_structures::Interval>> getConnectedComponents(std::span<const cg::data_structures::Interval> intervals)
    {
        cg::utils::verifyEndpointsInRange(intervals);
        cg::utils::verifyEndpointsUnique(intervals);

        std::vector<cg::data_structures::Interval> sortedIntervals(intervals.begin(), intervals.end());
            
        std::sort(sortedIntervals.begin(), sortedIntervals.end(), cg::data_structures::IntervalDistinctLeftCompare());

        std::stack<std::set<cg::data_structures::Interval, cg::data_structures::IntervalDistinctRightCompare>> componentsInProgress;

        std::vector<std::vector<cg::data_structures::Interval>> completeComponents;
        for (const auto& interval : sortedIntervals)
        {
            // Remove all components that don't intersect the current interval and mark them as complete.
            // No future interval can be connected to any of them, since it will have a larger left end point than the current interval.
            while (!componentsInProgress.empty())
            {
                const auto& mostRecentComponent = componentsInProgress.top();
                if (mostRecentComponent.empty())
                {
                    throw std::runtime_error("Internal error: encountered an empty most recent component");
                }
                const auto& last = *mostRecentComponent.rbegin();
                if (last.Right > interval.Left)
                {
                    // This component may include 'interval'.
                    break;
                }
                // This component cannot have any more intervals, remove it from the in progress ones and add it to the complete components.
                auto complete = std::vector<cg::data_structures::Interval>{mostRecentComponent.begin(), mostRecentComponent.end()};
                completeComponents.push_back(std::move(complete));
                componentsInProgress.pop();
            }
            // At this point, there are either no more components, or there are no right end-points in the top component, within interval
            if (componentsInProgress.empty())
            {
                // Begin a new component, there is no top component
                componentsInProgress.push(std::set<cg::data_structures::Interval, cg::data_structures::IntervalDistinctRightCompare>{interval});
            }
            else
            {
                const auto& top = componentsInProgress.top();
                auto it = std::prev(top.upper_bound(interval)); // Find the interval with largest right end-point before interval's right end-point.
                if (!it->overlaps(interval))
                {
                    // This interval may be its own component (i.e. at this point in the scan, that is the case, it may become false later)
                    componentsInProgress.push(std::set<cg::data_structures::Interval, cg::data_structures::IntervalDistinctRightCompare>{interval});
                }
                else
                {
                    // This interval intersects at least one other component.
                    auto mergedComponent = std::move(componentsInProgress.top()); // The new interval intersects this component, so it should be added here.
                    componentsInProgress.pop();
                    // However, if there are other components it may overlap them too. If that's the case, we need to merge them with 'mergedComponent'
                    // as they're all part of the same connected component of the graph.
                    while (!componentsInProgress.empty())
                    {
                        auto& prevComponent = componentsInProgress.top();
                        auto it = std::prev(prevComponent.upper_bound(interval));
                        if (!it->overlaps(interval))
                        {
                            break;
                        }
                        auto movedPrevComponent = std::move(componentsInProgress.top());
                        componentsInProgress.pop();
                        // Insert the smaller component into the larger, and ensure 'mergedComponent' points to the resulting component (where interval belongs)
                        // Inserting the smaller component into the larger is important so that each interval can participate in at most O(\log n) merges
                        if (mergedComponent.size() < movedPrevComponent.size())
                        {
                            movedPrevComponent.merge(mergedComponent); // moves nodes from merged → prev
                            mergedComponent.swap(movedPrevComponent);  // now merged has the union
                        }
                        else
                        {
                            mergedComponent.merge(movedPrevComponent); // moves nodes from prev → merged
                        }
                    }
                    mergedComponent.insert(interval);
                    componentsInProgress.push(std::move(mergedComponent));
                }
            }
        }
        while (!componentsInProgress.empty())
        {
            auto top = std::move(componentsInProgress.top());
            componentsInProgress.pop();
            auto complete = std::vector<cg::data_structures::Interval>{top.begin(), top.end()};
            completeComponents.push_back(std::move(complete));
        }
        return completeComponents;
    }
}