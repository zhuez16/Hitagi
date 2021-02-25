#include <algorithm>
#include "logging.hpp"
#include "LoopSearch.hpp"
#include "LoopInvHoist.hpp"
#define IS_GLOBAL_VARIABLE(l_val) dynamic_cast<GlobalVariable *>(l_val)
#define IS_GEP_INSTR(l_val) dynamic_cast<GetElementPtrInst *>(l_val)

void LoopInvHoist::run()
{
    // 先通过LoopSearch获取循环的相关信息
    LoopSearch loop_searcher(m_, false);
    loop_searcher.run();
    // 接下来由你来补充啦！
    for(auto f : m_->get_functions()){
        auto BBSET_IN_FUNC=loop_searcher.get_loops_in_func(f);
        //初始化func中的循环都没有分析过
        for(auto BBSET=BBSET_IN_FUNC.begin();BBSET!=BBSET_IN_FUNC.end();BBSET++){
            residual_loop[*BBSET]=1;
        }
        //对func中的循环遍历
        for(auto BBSET=BBSET_IN_FUNC.begin();BBSET!=BBSET_IN_FUNC.end();BBSET++){
            //找到BBSET的最内层循环
            auto bbset=*BBSET;
            while(bbset!=nullptr){
                //找到内层循环后自内向外遍历
                int flag=1;
                auto end_bb=loop_searcher.get_loop_base(bbset);
                while(flag==1){
                    //找到最内层循环
                    for(auto bb=bbset->begin();bb!=bbset->end();++bb){
                        auto inner_loop=loop_searcher.get_inner_loop(*bb);
                        if(bbset!=inner_loop&&residual_loop[inner_loop]==1){
                            bbset=loop_searcher.get_inner_loop(*bb);
                            flag=1;
                            break;
                        }
                        else{
                            flag=0;
                            end_bb=*bb;
                        }
                    }
                }
                auto base=loop_searcher.get_loop_base(bbset);
                auto base_prevs=base->get_pre_basic_blocks();
                for(auto tmp=base_prevs.begin();tmp!=base_prevs.end();tmp++){
                    if(bbset->find(*tmp)==bbset->end()){
                        find_invariant(bbset,*tmp, end_bb);
                        break;
                    }
                }
                residual_loop[bbset]=0;
               // BBSET_IN_FUNC.erase(bbset);
               // std::cout<<"erase bbset"<<std::endl;
                bbset=loop_searcher.get_parent_loop(bbset);
                //std::cout<<"get parent bbset"<<std::endl;
            }
            //std::cout<<"end a loop"<<std::endl;
        }
    }
    
}

void LoopInvHoist::find_invariant(BBset_t* set,BasicBlock* front,BasicBlock*end){
    std::set<Value*> left_var;//左值变量
    std::set<Instruction*> need_delete;//
    //遍历loop中的basicblock
    for(auto bb=set->begin();bb!=set->end();bb++){
        for(auto instr :(*bb)->get_instructions()){
            left_var.insert(instr);
        }
    }
    for(auto bb=set->begin();bb!=set->end();++bb){
        for(auto instr:(*bb)->get_instructions()){
            auto val=instr->get_operands();
            int num=instr->get_num_operand();
            //不是phi
            if(!instr->is_phi()&&!instr->is_br()&&!instr->is_load()&&!instr->is_store()){
                int i=0;
                
                for(i=0;i<num;i++){
                    //右值在左值里面
                    if(left_var.find(val[i])!=left_var.end()&&!IS_GLOBAL_VARIABLE(val[i])&&!IS_GEP_INSTR(val[i])){
                        break;
                    }
                }
                if(i==num){
                    need_delete.insert(instr);
                    left_var.erase(instr);
                }
            }
            
        }
        //遍历完bb中的所有指令
        auto lastInstr=front->get_terminator();
        front->delete_instr(lastInstr);
        for(auto instr=need_delete.begin();instr!=need_delete.end();instr++){
            (*bb)->delete_instr(*instr);
            front->add_instruction(*instr);
        }
        need_delete.clear();
        front->add_instruction(lastInstr);
    }
}

