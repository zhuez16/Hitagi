#include "LoopSearch.hpp"
#include <iostream>
#include <unordered_set>
#include <fstream>
#include "logging.hpp"

struct CFGNode
{
    std::unordered_set<CFGNodePtr> succs;
    std::unordered_set<CFGNodePtr> prevs;
    BasicBlock *bb;
    int index;   // the index of the node in CFG
    int lowlink; // the min index of the node in the strongly connected componets
    bool onStack;
};

// build control flow graph used in loop search pass
/*
 build_cfg: 传入参数时一个函数指针，输出为一个CFGNode的集合
 功能：顺序读取函数中的basic_block，赋值给一个新的CFGNode中的basicblock指针。因为是无序的集合，此时CFGNode中的index,lowlink=-1
      维护了一个bb到CFGNode的表，将新加的CFGNode和bb放入，将CFGNode放入无序集合result中。
再顺序读取函数中的basicblock,通过bb到CFGNode的表找到CFGNode，并通过bb->get_succ_basicblock,bb->get_prev_basicblocks找到该bb的所有前驱和后继basicblock,再次利用bb到CFGNode的表并将这些前驱后继CFGNode的指针存入该CFGNode的succs,prevs指针中
 */
void LoopSearch::build_cfg(Function *func, std::unordered_set<CFGNode *> &result)
{
    std::unordered_map<BasicBlock *, CFGNode *> bb2cfg_node;
    for (auto bb : func->get_basic_blocks())
    {
        auto node = new CFGNode;
        node->bb = bb;
        node->index = node->lowlink = -1;
        node->onStack = false;
        bb2cfg_node.insert({bb, node});

        result.insert(node);
    }
    for (auto bb : func->get_basic_blocks())
    {
        auto node = bb2cfg_node[bb];
        std::string succ_string = "success node: ";
        for (auto succ : bb->get_succ_basic_blocks())
        {
            succ_string = succ_string + succ->get_name() + " ";
            node->succs.insert(bb2cfg_node[succ]);
        }
        std::string prev_string = "previous node: ";
        for (auto prev : bb->get_pre_basic_blocks())
        {
            prev_string = prev_string + prev->get_name() + " ";
            node->prevs.insert(bb2cfg_node[prev]);
        }
    }
}

// Tarjan algorithm
// reference: https://baike.baidu.com/item/tarjan%E7%AE%97%E6%B3%95/10687825?fr=aladdin
/*
 Tarjan算法是基于对图深度优先搜索的算法，每个强连通分量为搜索树中的一棵子树。
 搜索时，把当前搜索树中未处理的节点加入一个堆栈，回溯时可以判断栈顶到栈中的节点是否为一个强连通分量。
 定义DFN(u)为节点u搜索的次序编号(时间戳)，Low(u)为u或u的子树能够追溯到的最早的栈中节点的次序号。
 当DFN(u)=Low(u)时，以u为根的搜索子树上所有节点是一个强连通分量。
 
算法理解: 当CFGNode没有进过栈时，n->index==-1(没有被分配序号)
 strongly_connected_components 传入CFGNode的集合，返回了CFGNode强连通分量的集合的集合 result中的每个元素是一个set,set里面的元素是CFGNode的指针
 */
bool LoopSearch::strongly_connected_components(
    CFGNodePtrSet &nodes,
    std::unordered_set<CFGNodePtrSet *> &result)
{
    index_count = 0;
    stack.clear();
    for (auto n : nodes)
    {
        if (n->index == -1)
            traverse(n, result);
    }
    return result.size() != 0;
}
void LoopSearch::traverse(
    CFGNodePtr n,
    std::unordered_set<CFGNodePtrSet *> &result)
{
    n->index = index_count++;
    n->lowlink = n->index;
    stack.push_back(n);
    n->onStack = true;

    for (auto su : n->succs)
    {
        // has not visited su
        if (su->index == -1)
        {
            traverse(su, result);
            n->lowlink = std::min(su->lowlink, n->lowlink);
        }
        // has visited su
        else if (su->onStack)
        {
            n->lowlink = std::min(su->index, n->lowlink);
        }
    }

    // nodes that in the same strongly connected component will be popped out of stack
    if (n->index == n->lowlink)
    {
        auto set = new CFGNodePtrSet;
        CFGNodePtr tmp;
        do
        {
            tmp = stack.back();
            tmp->onStack = false;
            set->insert(tmp);
            stack.pop_back();
        } while (tmp != n);
        if (set->size() == 1)
            delete set;
        else
            result.insert(set);
    }
}
 /*
  
  */

CFGNodePtr LoopSearch::find_loop_base(
    CFGNodePtrSet *set,
    CFGNodePtrSet &reserved)
{

    CFGNodePtr base = nullptr;
    bool hadBeen = false;
    for (auto n : *set)
    {
        for (auto prev : n->prevs)
        {
            if (set->find(prev) == set->end())  //当set中一个结点的某个前驱不在这个set中时，这个结点就是循环的base
            {
                base = n;
            }
        }
    }
    if (base != nullptr)
        return base;
    for (auto res : reserved)
    {
        for (auto succ : res->succs)
        {
            if (set->find(succ) != set->end())  //当reserved中一个结点的后继在set中，则base为这个后继结点
            {
                base = succ;
            }
        }
    }

    return base;
}
void LoopSearch::run()
{
    

    auto func_list = m_->get_functions();
    for (auto func : func_list)
    {

        if (func->get_basic_blocks().size() == 0)
        {
            continue;
        }
        else    //对于func_list中的每一个有basicblock的函数来说
        {
            CFGNodePtrSet nodes;
            CFGNodePtrSet reserved;
            std::unordered_set<CFGNodePtrSet *> sccs; //node指针集合的集合
            
            // step 1: build cfg
            build_cfg(func, nodes);
            // dump graph
            dump_graph(nodes, func->get_name());
            // step 2: find strongly connected graph from external to internal
            int scc_index = 0;
            while (strongly_connected_components(nodes, sccs))
            {

                if (sccs.size() == 0)
                {
                    
                    break;
                }
                else
                {
                    // step 3: find loop base node for each strongly connected graph

                    for (auto scc : sccs)
                    {
                        scc_index += 1;

                        auto base = find_loop_base(scc, reserved);

                        // step 4: store result
                        auto bb_set = new BBset_t;
                        std::string node_set_string = "";

                        for (auto n : *scc)
                        {
                            bb_set->insert(n->bb); //bb_set 一个循环中每个节点的bb的集合
                            node_set_string = node_set_string + n->bb->get_name() + ',';
                        }
                        loop_set.insert(bb_set); //里面有每个循环的bb_set
                        func2loop[func].insert(bb_set);  //函数对应的所有循环的bb_set
                        base2loop.insert({base->bb, bb_set}); // 每个循环 base_bb->bb_set
                        loop2base.insert({bb_set, base->bb}); //每个循环 bb_set->base_bb
                        for (auto bb : *bb_set)
                        {
                            if (bb2base.find(bb) == bb2base.end()) //bb2base 这个结点在最内层循环中的base_bb
                            {
                                bb2base.insert({bb, base->bb});
                            }
                            else
                            {
                                bb2base[bb] = base->bb;
                            }
                        }
                        // step 5: map each node to loop base
                        for (auto bb : *bb_set)
                        {
                            if (bb2base.find(bb) == bb2base.end())
                                bb2base.insert({bb, base->bb});
                            else
                                bb2base[bb] = base->bb;
                        }

                        // step 6: remove loop base node for researching inner loop
                        reserved.insert(base);
                        dump_graph(*scc, func->get_name() + '_' + std::to_string(scc_index));
                        nodes.erase(base);
                        for (auto su : base->succs)
                        {
                            su->prevs.erase(base);
                        }
                        for (auto prev : base->prevs)
                        {
                            prev->succs.erase(base);
                        }

                    } // for (auto scc : sccs)
                    for (auto scc : sccs)
                        delete scc;
                    sccs.clear();
                    for (auto n : nodes)
                    {
                        n->index = n->lowlink = -1;
                        n->onStack = false;
                    }
                } // else
            }     // while (strongly_connected_components(nodes, sccs))
            // clear
            reserved.clear();
            for (auto node : nodes)
            {
                delete node;
            }
            nodes.clear();
        } // else

    } // for (auto func : func_list)
}

void LoopSearch::dump_graph(CFGNodePtrSet &nodes, std::string title)
{
    if (dump)
    {
        std::vector<std::string> edge_set;
        for (auto node : nodes)
        {
            if (node->bb->get_name() == "")
            {
                
                return;
            }
            if (base2loop.find(node->bb) != base2loop.end())
            {

                for (auto succ : node->succs)
                {
                    if (nodes.find(succ) != nodes.end())
                    {
                        edge_set.insert(edge_set.begin(), '\t' + node->bb->get_name() + "->" + succ->bb->get_name() + ';' + '\n');
                    }
                }
                edge_set.insert(edge_set.begin(), '\t' + node->bb->get_name() + " [color=red]" + ';' + '\n');
            }
            else
            {
                for (auto succ : node->succs)
                {
                    if (nodes.find(succ) != nodes.end())
                    {
                        edge_set.push_back('\t' + node->bb->get_name() + "->" + succ->bb->get_name() + ';' + '\n');
                    }
                }
            }
        }
        std::string digragh = "digraph G {\n";
        for (auto edge : edge_set)
        {
            digragh += edge;
        }
        digragh += '}';
        std::ofstream file_output;
        file_output.open(title + ".dot", std::ios::out);
        
        file_output << digragh;
        file_output.close();
        std::string dot_cmd = "dot -Tpng " + title + ".dot" + " -o " + title + ".png";
        std::system(dot_cmd.c_str());
    }
}

BBset_t *LoopSearch::get_parent_loop(BBset_t *loop)
{
    auto base = loop2base[loop];
    for (auto prev : base->get_pre_basic_blocks())
    {
        if (loop->find(prev) != loop->end())
            continue;
        auto loop = get_inner_loop(prev);
        if (loop == nullptr || loop->find(base) == loop->end())
            return nullptr;
        else
        {
            return loop;
        }
    }
    return nullptr;
}

std::unordered_set<BBset_t *> LoopSearch::get_loops_in_func(Function *f)
{
    return func2loop.count(f) ? func2loop[f] : std::unordered_set<BBset_t *>();
}
