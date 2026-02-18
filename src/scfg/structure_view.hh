#ifndef SCFG_STRUCTURE_VIEW_HH_
#define SCFG_STRUCTURE_VIEW_HH_

#include "base_types.hh"
#include "scfg/constraint_oracle.hh"
#include "sparse_tree.hh"

#include <algorithm>

namespace scfg {

class StructureView {
  public:
    virtual ~StructureView() = default;

    virtual cand_pos_t n() const = 0;
    virtual cand_pos_t pair_any(cand_pos_t i) const = 0;
    virtual cand_pos_t pair_round(cand_pos_t i) const = 0;
    virtual cand_pos_t pair_square(cand_pos_t i) const = 0;
    virtual bool is_pair_round(cand_pos_t i, cand_pos_t j) const = 0;
    virtual bool is_pair_square(cand_pos_t i, cand_pos_t j) const = 0;
    virtual bool is_unpaired(cand_pos_t i) const = 0;

    // Band (square) view.
    virtual cand_pos_t parent_index(cand_pos_t i) const = 0;
    virtual bool weakly_closed(cand_pos_t i, cand_pos_t j) const = 0;
    virtual cand_pos_t b(cand_pos_t i, cand_pos_t j) const = 0;
    virtual cand_pos_t bp(cand_pos_t i, cand_pos_t j) const = 0;
    virtual cand_pos_t B(cand_pos_t i, cand_pos_t j) const = 0;
    virtual cand_pos_t Bp(cand_pos_t i, cand_pos_t j) const = 0;

    // Round view.
    virtual cand_pos_t unpaired_prefix(cand_pos_t i) const = 0;
    virtual bool can_pair_left_span(cand_pos_t i, cand_pos_t k) const = 0;
    virtual bool can_pair_right_span(cand_pos_t k, cand_pos_t j) const = 0;

    // Any-pair empty check.
    virtual bool is_empty_region(cand_pos_t i, cand_pos_t j) const = 0;
};

namespace detail {

inline cand_pos_t parent_index_from_tree(const sparse_tree &tree, cand_pos_t i) {
    if (i < 0 || static_cast<size_t>(i) >= tree.tree.size()) return -1;
    const Node *parent = tree.tree[i].parent;
    return parent ? parent->index : -1;
}

inline cand_pos_t unpaired_prefix_from_tree(const sparse_tree &tree, cand_pos_t index) {
    if (tree.up.empty() || index <= 0) return 0;
    const cand_pos_t capped = std::min<cand_pos_t>(index, static_cast<cand_pos_t>(tree.up.size() - 1));
    return static_cast<cand_pos_t>(tree.up[capped]);
}

inline bool can_pair_left_span_from_tree(const sparse_tree &tree, cand_pos_t left, cand_pos_t split) {
    if (split < left) return false;
    if (split == left) return true;
    return unpaired_prefix_from_tree(tree, split - 1) >= (split - left);
}

inline bool can_pair_right_span_from_tree(const sparse_tree &tree, cand_pos_t split, cand_pos_t right) {
    if (right < split) return false;
    if (right == split) return true;
    return unpaired_prefix_from_tree(tree, right - 1) >= (right - split);
}

} // namespace detail

class PkFreeView final : public StructureView {
  public:
    explicit PkFreeView(sparse_tree &tree) : tree_(tree) {}

    cand_pos_t n() const override { return static_cast<cand_pos_t>(tree_.n); }
    cand_pos_t pair_any(cand_pos_t i) const override { return pair_round(i); }
    cand_pos_t pair_round(cand_pos_t i) const override { return tree_.tree[i].pair; }
    cand_pos_t pair_square(cand_pos_t) const override { return -1; }
    bool is_pair_round(cand_pos_t i, cand_pos_t j) const override { return pair_round(i) == j; }
    bool is_pair_square(cand_pos_t, cand_pos_t) const override { return false; }
    bool is_unpaired(cand_pos_t i) const override { return pair_round(i) < 0; }

    cand_pos_t parent_index(cand_pos_t i) const override { return detail::parent_index_from_tree(tree_, i); }
    bool weakly_closed(cand_pos_t i, cand_pos_t j) const override { return tree_.weakly_closed(i, j); }
    cand_pos_t b(cand_pos_t i, cand_pos_t j) const override { return tree_.b(i, j); }
    cand_pos_t bp(cand_pos_t i, cand_pos_t j) const override { return tree_.bp(i, j); }
    cand_pos_t B(cand_pos_t i, cand_pos_t j) const override { return tree_.B(i, j); }
    cand_pos_t Bp(cand_pos_t i, cand_pos_t j) const override { return tree_.Bp(i, j); }

    cand_pos_t unpaired_prefix(cand_pos_t i) const override { return detail::unpaired_prefix_from_tree(tree_, i); }
    bool can_pair_left_span(cand_pos_t i, cand_pos_t k) const override {
        return detail::can_pair_left_span_from_tree(tree_, i, k);
    }
    bool can_pair_right_span(cand_pos_t k, cand_pos_t j) const override {
        return detail::can_pair_right_span_from_tree(tree_, k, j);
    }

    bool is_empty_region(cand_pos_t i, cand_pos_t j) const override { return scfg::is_empty_region(tree_, i, j); }

  private:
    sparse_tree &tree_;
};

class KTypeView final : public StructureView {
  public:
    KTypeView(sparse_tree &round_tree, sparse_tree &square_tree)
        : round_tree_(round_tree), square_tree_(square_tree) {}

    cand_pos_t n() const override { return static_cast<cand_pos_t>(round_tree_.n); }
    cand_pos_t pair_any(cand_pos_t i) const override {
        const cand_pos_t round = pair_round(i);
        if (round >= 0) return round;
        return pair_square(i);
    }
    cand_pos_t pair_round(cand_pos_t i) const override { return round_tree_.tree[i].pair; }
    cand_pos_t pair_square(cand_pos_t i) const override { return square_tree_.tree[i].pair; }
    bool is_pair_round(cand_pos_t i, cand_pos_t j) const override { return pair_round(i) == j; }
    bool is_pair_square(cand_pos_t i, cand_pos_t j) const override { return pair_square(i) == j; }
    bool is_unpaired(cand_pos_t i) const override { return pair_round(i) < 0 && pair_square(i) < 0; }

    cand_pos_t parent_index(cand_pos_t i) const override { return detail::parent_index_from_tree(square_tree_, i); }
    bool weakly_closed(cand_pos_t i, cand_pos_t j) const override { return square_tree_.weakly_closed(i, j); }
    cand_pos_t b(cand_pos_t i, cand_pos_t j) const override { return square_tree_.b(i, j); }
    cand_pos_t bp(cand_pos_t i, cand_pos_t j) const override { return square_tree_.bp(i, j); }
    cand_pos_t B(cand_pos_t i, cand_pos_t j) const override { return square_tree_.B(i, j); }
    cand_pos_t Bp(cand_pos_t i, cand_pos_t j) const override { return square_tree_.Bp(i, j); }

    cand_pos_t unpaired_prefix(cand_pos_t i) const override {
        return detail::unpaired_prefix_from_tree(round_tree_, i);
    }
    bool can_pair_left_span(cand_pos_t i, cand_pos_t k) const override {
        return detail::can_pair_left_span_from_tree(round_tree_, i, k);
    }
    bool can_pair_right_span(cand_pos_t k, cand_pos_t j) const override {
        return detail::can_pair_right_span_from_tree(round_tree_, k, j);
    }

    bool is_empty_region(cand_pos_t i, cand_pos_t j) const override {
        return scfg::is_empty_region(round_tree_, i, j) && scfg::is_empty_region(square_tree_, i, j);
    }

  private:
    sparse_tree &round_tree_;
    sparse_tree &square_tree_;
};

} // namespace scfg

#endif
