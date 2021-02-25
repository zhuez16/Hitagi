#include "ConstPropagation.hpp"
#include "logging.hpp"

// 给出了返回整形值的常数折叠实现，大家可以参考，在此基础上拓展
// 当然如果同学们有更好的方式，不强求使用下面这种方式
ConstantInt *ConstFolder::compute(
    float value){
    return ConstantInt::get((int)value,module_);
}
ConstantInt *ConstFolder::zextcom(
    int value){
    return ConstantInt::get((bool)value,module_);
}
ConstantInt *ConstFolder::compute(
    Instruction::OpID op,
    ConstantInt *value1,
    ConstantInt *value2)
{

    int c_value1 = value1->get_value();
    int c_value2 = value2->get_value();
    switch (op)
    {
    case Instruction::add:
        return ConstantInt::get(c_value1 + c_value2, module_);

        break;
    case Instruction::sub:
        return ConstantInt::get(c_value1 - c_value2, module_);
        break;
    case Instruction::mul:
        return ConstantInt::get(c_value1 * c_value2, module_);
        break;
    case Instruction::sdiv:
        return ConstantInt::get((int)(c_value1 / c_value2), module_);
        break;
    default:
        return nullptr;
        break;
    }
}
ConstantFP *ConstFolder::compute(
    int value){
    return ConstantFP::get((float)value,module_);
}
ConstantFP *ConstFolder::compute(
    Instruction::OpID op,
    ConstantFP *value1,
    ConstantFP *value2)
{

    float c_value1 = value1->get_value();
    float c_value2 = value2->get_value();
    switch (op)
    {
    case Instruction::fadd:
        return ConstantFP::get(c_value1 + c_value2, module_);

        break;
    case Instruction::fsub:
        return ConstantFP::get(c_value1 - c_value2, module_);
        break;
    case Instruction::fmul:
        return ConstantFP::get(c_value1 * c_value2, module_);
        break;
    case Instruction::fdiv:
        return ConstantFP::get((int)(c_value1 / c_value2), module_);
        break;
    default:
        return nullptr;
        break;
    }
}
// 用来判断value是否为ConstantFP，如果不是则会返回nullptr
ConstantFP *cast_constantfp(Value *value)
{
    auto constant_fp_ptr = dynamic_cast<ConstantFP *>(value);
    if (constant_fp_ptr)
    {
        return constant_fp_ptr;
    }
    else
    {
        return nullptr;
    }
}
ConstantInt *cast_constantint(Value *value)
{
    auto constant_int_ptr = dynamic_cast<ConstantInt *>(value);
    if (constant_int_ptr)
    {
        return constant_int_ptr;
    }
    else
    {
        return nullptr;
    }
}
//删除不会被跳转到的分支上的所有bb
/*void ConstPropagation::PostOrder_deletebb(Function* func,BasicBlock* bb,BasicBlock* limit){
    if(bb==limit) return;
    else{
        //遍历所有的后继结点
        for(auto iter=bb->get_succ_basic_blocks().begin();iter!=bb->get_succ_basic_blocks().end();iter++){
            PostOrder_deletebb(func,*iter, limit);
        }
        func->remove(bb);
    }
}
//得到bb后续的所有结点
std::list<BasicBlock*> ConstPropagation::find_succ(BasicBlock* bb){
    std::list<BasicBlock*> succ_list;
    std::queue<BasicBlock*> Queue;
    Queue.push(bb);
    
}
//找到true,false分支的汇合结点
BasicBlock* ConstPropagation::find_limitbb(std::list<BasicBlock*>t_succ,std::list<BasicBlock*>f_succ){
    std::unordered_set<BasicBlock*>set;
    set.insert(t_succ.begin(),t_succ.end());
    for(auto bb=f_succ.begin();bb!=f_succ.end();bb++){
        if(!set.insert(*bb).second) return *bb;
    }
    return nullptr;
}*/
std::vector<Instruction *> wait_delete;
void ConstPropagation::run()
{
    // 从这里开始吧！
    auto func_list=m_->get_functions();
    for (auto func:func_list){
        if (func->get_basic_blocks().size()==0){
            continue;
        }
        else{
            //fetch from the 1st instr to the end of the func
            //turning a var into a const may generate another const
            for (auto bb:func->get_basic_blocks()){
                for (auto instr:bb->get_instructions()){
                    if (instr->is_add() || instr->is_sub()|| instr->is_mul() || instr->is_div()){
                        auto r_val=cast_constantint(static_cast<StoreInst *>(instr)->get_rval());
                        auto l_val=cast_constantint(static_cast<StoreInst *>(instr)->get_lval());
                        if (r_val && l_val){
                            Value *tmp=fold->compute(instr->get_instr_type(),r_val,l_val);
                            instr->replace_all_use_with(tmp);
                            //删除指令
                            wait_delete.push_back(instr);
                        }
                    }
                    else if (instr->is_fadd()||instr->is_fdiv()||instr->is_fmul()||instr->is_fsub()){
                        auto r_val=cast_constantfp(static_cast<StoreInst *>(instr)->get_rval());
                        auto l_val=cast_constantfp(static_cast<StoreInst *>(instr)->get_lval());
                        if (r_val && l_val){
                            Value *tmp=fold->compute(instr->get_instr_type(),r_val,l_val);
                            instr->replace_all_use_with(tmp);
                            //删除指令
                            wait_delete.push_back(instr);
                        }
                    }
                    else if (instr->is_fp2si()){
                        auto l_val=cast_constantfp(static_cast<StoreInst*>(instr)->get_operand(0));
                        if (l_val){
                            Value *tmp=fold->compute(l_val->get_value());
                            instr->replace_all_use_with(tmp);
                            wait_delete.push_back(instr);
                        }
                    }
                    else if (instr->is_si2fp()){
                        auto l_val=cast_constantint(static_cast<StoreInst*>(instr)->get_operand(0));
                        if (l_val){
                            Value *tmp=fold->compute(l_val->get_value());
                            instr->replace_all_use_with(tmp);
                            wait_delete.push_back(instr);
                        }
                    }
                    else if (instr->is_zext()){
                        auto l_val=cast_constantint(static_cast<StoreInst*>(instr)->get_operand(0));
                        if (l_val){
                            Value *tmp=fold->zextcom(l_val->get_value());
                            instr->replace_all_use_with(tmp);
                            wait_delete.push_back(instr);
                        }
                    }
                    else if(instr->is_cmp()){
                        auto lval=cast_constantint(instr->get_operand(0));
                        auto rval=cast_constantint(instr->get_operand(1));
                        if(lval&&rval){
                            auto CmpOP=static_cast<CmpInst*>(instr)->get_cmp_op();
                            bool tmp=0;
                            if(CmpOP==CmpInst::EQ){
                                if(rval==lval) tmp=1;
                            }
                            else if(CmpOP==CmpInst::NE){
                                if(lval!=rval) tmp=1;
                            }
                            else if(CmpOP==CmpInst::GT){
                                if(lval>rval) tmp=1;
                            }
                            else if(CmpOP==CmpInst::GE){
                                if(lval>=rval) tmp=1;
                            }
                            else if(CmpOP==CmpInst::LT){
                                if(lval<rval) tmp=1;
                            }
                            else{
                                if(lval<=rval) tmp=1;
                            }
                            instr->replace_all_use_with(ConstantInt::get(tmp,m_));
                            //删除指令
                            wait_delete.push_back(instr);
                        }
                        
                    }
                    else if(instr->is_fcmp()){
                        auto lval=cast_constantfp(instr->get_operand(0));
                        auto rval=cast_constantfp(instr->get_operand(1));
                        if(lval&&rval){
                            auto CmpOP=static_cast<FCmpInst*>(instr)->get_cmp_op();
                            bool tmp=0;
                            if(CmpOP==FCmpInst::EQ){
                                if(rval==lval) tmp=1;
                            }
                            else if(CmpOP==FCmpInst::NE){
                                if(lval!=rval) tmp=1;
                            }
                            else if(CmpOP==FCmpInst::GT){
                                if(lval>rval) tmp=1;
                            }
                            else if(CmpOP==FCmpInst::GE){
                                if(lval>=rval) tmp=1;
                            }
                            else if(CmpOP==FCmpInst::LT){
                                if(lval<rval) tmp=1;
                            }
                            else{
                                if(lval<=rval) tmp=1;
                            }
                            instr->replace_all_use_with(ConstantInt::get(tmp,m_));
                            //删除指令
                            wait_delete.push_back(instr);
                        }
                        
                    }
                    //考虑全局变量
                    else if(instr->is_load()){
                        //全局变量
                        //如果IR中还有load指令，说明这时的全局变量已经是非常数了，将load指令从全局变量的y使用者链表中删除，保证后续操作
                        auto global_val=static_cast<LoadInst*>(instr)->get_lval();
                        global_val->remove_use(instr);
                        
                    }
                    else if(instr->is_store()){
                        //全局变量，如果store常数，则下次load时常量传播，将store语句删除
                        //store const, *a
                        auto rval=static_cast<StoreInst*>(instr)->get_rval(); //const
                        auto lval=static_cast<StoreInst*>(instr)->get_lval(); //a
                        //从全局变量的使用者链表中删除store指令，为了get_use_list().front总是当前判断的指令
                        lval->remove_use(instr);
                        if(cast_constantfp(rval)||cast_constantint(rval)){
                            auto tmp=static_cast<Instruction*>(lval->get_use_list().front().val_);
                            while(tmp->is_load()){
                                if (tmp->get_parent()!=bb) break;
                                auto tmp_lval=tmp->get_operand(0);
                                tmp->replace_all_use_with(rval);
                                wait_delete.push_back(tmp);
                                lval->remove_use(tmp);//从全局变量的使用者链表中删除load
                                tmp++;
                            }
                        }
                        
                    }
                    else if(instr->is_br()){
                        //条件跳转变成强制跳转
                        if(static_cast<BranchInst*>(instr)->is_cond_br()){
                            auto cond=cast_constantint(static_cast<BranchInst*>(instr)->get_operand(0));
                            // cond是常数
                            if(cond){
                                auto falseBB=bb->get_succ_basic_blocks().front();
                                auto trueBB=bb->get_succ_basic_blocks().back();
                                //强制跳转到trueBB
                                if(cond->get_value()==1){
                                    wait_delete.push_back(instr);
                                    auto br=BranchInst::create_br(trueBB, bb);
                                    //删除false分支的所有块
                                    //??? 不知道是否需要将false分支的后续块手动删除，需要的话将前面注释掉的代码还原
                                    //不需要，因为ifelse的结构其实是if-true-false-exit，所以后面必然是一个不需要删除的块。
                                    //func->remove(falseBB);
                                    bool flag=false;
                                    std::map<BasicBlock*,bool> bbflag;
                                    std::queue<BasicBlock*> que;
                                    que.push(trueBB);
                                    bbflag[trueBB]=true;
                                    while (!que.empty()){
                                        auto tmpbb=que.front();
                                        que.pop();
                                        if (tmpbb->get_name()==falseBB->get_name()){
                                            flag=true;
                                            break;
                                        }
                                        for (auto next:tmpbb->get_succ_basic_blocks()){
                                            if (!bbflag[next]){
                                                bbflag[next]=true;
                                                que.push(next);
                                            }
                                        }
                                    }
                                    if (!flag) func->remove(falseBB);
                                }
                                else if(cond->get_value()==0){
                                    wait_delete.push_back(instr);
                                    auto br=BranchInst::create_br(falseBB, bb);
                                    bb->add_instruction(br);
                                    //删除true分支的所有块
                                    func->remove(trueBB);
                                }
                            }
                        }
                    }
                }
                for (auto instr:wait_delete){
                    bb->delete_instr(instr);
                }
            }
        }
    }
}

