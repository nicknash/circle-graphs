#include <stdexcept>

#include "data_structures/interval.h"

#include "mis/interval_store.h"
#include <iostream>

#include <boost/intrusive/rbtree.hpp>
#include <boost/intrusive/set_hook.hpp>
#include <boost/intrusive/options.hpp>
#include <algorithm>
#include <optional>
#include <climits>

namespace cg::mis {

IntervalNode::IntervalNode(int left, int right)
    : left(left), right(right)
    , subtree_max_right(right)
{}

bool CompareLeft::operator()(IntervalNode const &a, IntervalNode const &b) const {
    return a.left < b.left;
}
IntervalStore::IntervalStore() = default;

void IntervalStore::addInterval(const cg::data_structures::Interval& iv) {
    auto *node = new IntervalNode(iv.Left, iv.Right);
    auto it = tree_.insert_equal(*node);

    using VT = typename IntervalTree::value_traits;
    using NT = typename VT::node_traits;

    // Get raw node pointers for new node and for the header (sentinel)
    auto np = VT::to_node_ptr(*it);
    auto root_it = tree_.root();
    auto root_np = VT::to_node_ptr(*root_it);
    auto header_np = NT::get_parent(root_np);

    // Update subtree_max_right up to, but not including, the header sentinel
    for (auto parent = NT::get_parent(np); parent && parent != header_np;
         parent = NT::get_parent(parent))
    {
        auto *pn = VT::to_value_ptr(parent);
        int m = pn->right;
        if (auto ln = NT::get_left(parent))
            m = std::max(m, VT::to_value_ptr(ln)->subtree_max_right);
        if (auto rn = NT::get_right(parent))
            m = std::max(m, VT::to_value_ptr(rn)->subtree_max_right);
        pn->subtree_max_right = m;
    }
}

std::optional<cg::mis::IntervalEndpoints>
 IntervalStore::tryGetRelevantInterval(int L0, int Rlo, int Rhi) const {
    if (tree_.empty()) return {};

    using VT = typename IntervalTree::value_traits;
    using NT = typename VT::node_traits;

    // Get root node pointer
    auto root_it = tree_.root();
    auto root_np = VT::to_node_ptr(*root_it);

    if (auto *res = query_node(root_np, L0, Rlo, Rhi))
    {
        return IntervalEndpoints{res->left,res->right};
    }
    return {};
}

const IntervalNode*
IntervalStore::query_node(
    typename IntervalTree::value_traits::node_traits::const_node_ptr np,
    int L0, int Rlo, int Rhi
) const {
    using VT = typename IntervalTree::value_traits;
    using NT = typename VT::node_traits;

    if (!np) return nullptr;
    auto *iv = VT::to_value_ptr(np);

    // Prune by subtree_max_right
    if (iv->subtree_max_right < Rlo)
        return nullptr;

    // If key too large, go left only
    if (iv->left >= L0)
        return query_node(NT::get_left(np), L0, Rlo, Rhi);

    // Try right subtree first
    if (auto *r = query_node(NT::get_right(np), L0, Rlo, Rhi))
        return r;

    // Check current node
    if (iv->right + 1 >= Rlo && iv->right + 1 < Rhi)
        return iv;

    // Fall back to left subtree
    return query_node(NT::get_left(np), L0, Rlo, Rhi);
}

} // namespace cg::mis
