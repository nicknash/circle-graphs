#pragma once

#include <vector>
#include <optional>
#include <list>

#include <optional>
#include <boost/intrusive/rbtree.hpp>
#include <boost/intrusive/set_hook.hpp>
#include <boost/intrusive/options.hpp>
#include "data_structures/interval.h"

namespace cg::data_structures
{
    class Interval;
}

namespace cg::mis
{
    namespace bi = boost::intrusive;

    struct IntervalEndpoints
    {
        int left, right;
    };
    // An intrusive node storing one interval plus metadata
    struct IntervalNode
    {
        //cg::data_structures::Interval interval;
        int left, right;
        int subtree_max_right;
        bi::set_member_hook<> hook;

        IntervalNode(int left, int right);
    };


    // Compare by Left endpoint
    struct CompareLeft
    {
        bool operator()(IntervalNode const &a, IntervalNode const &b) const;
    };

    // Intrusive RB-tree keyed on Left, allows duplicates
    using IntervalTree = bi::rbtree<
        IntervalNode,
        bi::member_hook<IntervalNode, bi::set_member_hook<>, &IntervalNode::hook>,
        bi::compare<CompareLeft>>;


    class IntervalStore
    {
    public:
        IntervalStore();

        // Insert one interval in O(log n)
        void addInterval(const cg::data_structures::Interval &iv);

        // Query in O(log n): largest Left < L0 with Right in [Rlo,Rhi)
        std::optional<IntervalEndpoints> tryGetRelevantInterval(int L0, int Rlo, int Rhi) const;

    private:
        IntervalTree tree_;

        // Helper: recursive search with subtree pruning
        const IntervalNode *query_node(
            typename IntervalTree::value_traits::node_traits::const_node_ptr np,
            int L0, int Rlo, int Rhi) const;
    };

} // namespace cg::mis