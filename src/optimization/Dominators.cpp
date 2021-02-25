#include "Dominators.h"
#include <algorithm>
#include <string>
/*
 静态单赋值格式构造.pdf笔记：
 支配树：
    IDOM(n):直接支配结点，是严格支配n的结点集合中与n最接近的一个，流图的入口结点没有直接支配结点
    支配树中的一个结点n,IDOM(n)是其在树中的父节点，DOM(n)中的各个结点，就是从支配树的根节点到n的路径上的那些结点（包含根节点和n）
 同时使用支配树和CFG，为每一个结点n计算支配边界DF(n)
 算法:定位CFG中的各个汇合点j,对j的每个前驱结点p，从p开始沿支配者树向上走，直到找到支配j的一个结点。在遍历过程中，除了遍历到支配j的结点之外，则对其余结点l,都有j属于DF(l).需要确保每个n都只添加到某个节点的支配边界一次
 φ函数插入算法：
    Globals:活动于多个程序块的变量名字集合（在block中先引用，后定义的变量名）
    Block(x):定义了x的结点集合
    对每个全局名字x,将WorkList初始化为Block(x),对WorkList上的每个基本块b,在DF(b)中的每个程序块d的起始处插入φ函数（并发执行），插入函数后将d也加入x的WorkList,以反映d对x的新赋值
    效率改进：维护WorkList的同时，维护一个WorkList上已处理的基本块List和已包含针对变量x的φ函数的基本块List
重命名算法：
    每个全局变量有一个基本名（x）,第一次定义命名为x0...，利用栈和计数器来实现算法
    对支配者树进行先根遍历。对每个block,先重命名顶部的φ函数定义的值，按序访问块中操作并进行重写和定义新名字。
    之后对于CFG的后继结点，重写φ函数。并对支配者树的子节点递归。
    递归完成后，将静态单赋值形式名的集合恢复到访问当前程序块之前的状态
 
 */
void Dominators::run()
{
    for (auto f : m_->get_functions()) {
        if (f->get_basic_blocks().size() == 0)
            continue;
        for (auto bb : f->get_basic_blocks() )
        {
            doms_.insert({bb ,{}});
            idom_.insert({bb ,{}});
            dom_frontier_.insert({bb ,{}});
            dom_tree_succ_blocks_.insert({bb ,{}});
        }
        
        create_reverse_post_order(f);
        create_idom(f);
        create_dominance_frontier(f);
        create_dom_tree_succ(f);
        // for debug
        // print_idom(f);
        // print_dominance_frontier(f);
    }
}

void Dominators::create_doms(Function *f)
{
    // init
    for (auto bb : f->get_basic_blocks()) {
        add_dom(bb, bb);
    }
    // iterate
    bool changed = true;
    std::vector<BasicBlock *> ret(f->get_num_basic_blocks());
    std::vector<BasicBlock *> pre(f->get_num_basic_blocks());
    while (changed) {
        changed = false;
        for (auto bb : f->get_basic_blocks()) {
            auto &bbs = bb->get_pre_basic_blocks();
            auto &first = get_doms((*bbs.begin()));
            pre.insert(pre.begin(), first.begin(), first.end());
            pre.resize(first.size());
            ret.resize(f->get_num_basic_blocks());
            for (auto iter = ++bbs.begin(); iter != bbs.end(); ++iter) {
                auto &now = get_doms((*iter));
                auto it = std::set_intersection(pre.begin(), pre.end(), 
                                    now.begin(), now.end(), ret.begin());
                ret.resize(it-ret.begin());
                pre.resize(ret.size());
                pre.insert(pre.begin(), ret.begin(), ret.end());
            }
            std::set<BasicBlock *> doms;
            doms.insert(bb);
            doms.insert(pre.begin(), pre.end());
            if (get_doms(bb) != doms) {
                set_doms(bb, doms);
                changed = true;
            }
        }
    }
}

void Dominators::create_reverse_post_order(Function *f)
{
    reverse_post_order_.clear();
    post_order_id_.clear();
    std::set<BasicBlock *> visited;
    post_order_visit(f->get_entry_block(), visited);
    reverse_post_order_.reverse();
}

void Dominators::post_order_visit(BasicBlock *bb, std::set<BasicBlock *> &visited)
{
    visited.insert(bb);
    for (auto b : bb->get_succ_basic_blocks()) {
        if (visited.find(b) == visited.end())
            post_order_visit(b, visited);
    }
    post_order_id_[bb] = reverse_post_order_.size();
    reverse_post_order_.push_back(bb);
}

void Dominators::create_idom(Function *f)
{   
    // init
	for (auto bb : f->get_basic_blocks())
        set_idom(bb, nullptr);
    auto root = f->get_entry_block();
    set_idom(root, root);

    // iterate
	bool changed = true;
    while (changed) {
        changed = false;
        for (auto bb : this->reverse_post_order_) {
            if (bb == root) {
                continue;
            }

            // find one pred which has idom
            BasicBlock *pred = nullptr;
            for (auto p : bb->get_pre_basic_blocks()) {
                if (get_idom(p)) {
                    pred = p;
                    break;
                }
            }
            assert(pred);

            BasicBlock *new_idom = pred;
            for (auto p : bb->get_pre_basic_blocks()) {
                if (p == pred)
                    continue;
                if (get_idom(p)) {
                    new_idom = intersect(p, new_idom);
                }
            }
            if (get_idom(bb) != new_idom) {
                set_idom(bb, new_idom);
                changed = true;
            }
        }
    }
    
}

// find closest parent of b1 and b2
BasicBlock *Dominators::intersect(BasicBlock *b1, BasicBlock *b2)
{
    while (b1 != b2) {
        while (post_order_id_[b1] < post_order_id_[b2]) {
            assert(get_idom(b1));
            b1 = get_idom(b1);
        }
        while (post_order_id_[b2] < post_order_id_[b1]) {
            assert(get_idom(b2));
            b2 = get_idom(b2);
        }
    }
    return b1;   
}

void Dominators::create_dominance_frontier(Function *f)
{
    for (auto bb : f->get_basic_blocks()) {
        if (bb->get_pre_basic_blocks().size() >= 2) {
            for (auto p : bb->get_pre_basic_blocks()) {
                auto runner = p;
                while (runner != get_idom(bb)) {
                    add_dominance_frontier(runner, bb);
                    runner = get_idom(runner);
                }
            }
        }
    }
}

void Dominators::create_dom_tree_succ(Function *f)
{
    for (auto bb : f->get_basic_blocks()) {
        auto idom = get_idom(bb);
        // e.g, entry bb
        if (idom != bb) {
            add_dom_tree_succ_block(idom, bb);
        }
    }
}

void Dominators::print_idom(Function *f)
{
    int counter = 0;
    std::map<BasicBlock *, std::string> bb_id;
    for (auto bb : f->get_basic_blocks()) {
        if (bb->get_name().empty())
            bb_id[bb] = "bb" + std::to_string(counter);
        else 
            bb_id[bb] = bb->get_name();
        counter++;
    }
    printf("Immediate dominance of function %s:\n", f->get_name().c_str());
    for (auto bb : f->get_basic_blocks()) {
        std::string output;
        output = bb_id[bb] + ": ";
        if (get_idom(bb)) {
            output += bb_id[get_idom(bb)];
        }
        else {
            output += "null";
        }
        printf("%s\n", output.c_str());
    }
}

void Dominators::print_dominance_frontier(Function *f)
{
    int counter = 0;
    std::map<BasicBlock *, std::string> bb_id;
    for (auto bb : f->get_basic_blocks()) {
        if (bb->get_name().empty())
            bb_id[bb] = "bb" + std::to_string(counter);
        else 
            bb_id[bb] = bb->get_name();
        counter++;
    }
    printf("Dominance Frontier of function %s:\n", f->get_name().c_str());
    for (auto bb : f->get_basic_blocks()) {
        std::string output;
        output = bb_id[bb] + ": ";
        if (get_dominance_frontier(bb).empty()) {
            output += "null";
        }
        else {
            bool first = true;
            for (auto df : get_dominance_frontier(bb)) {
                if (first) {
                    first = false;
                }
                else {
                    output += ", ";
                }
                output += bb_id[df];
            }
        }
        printf("%s\n", output.c_str());
    }
}

