#include "ActiveVars.hpp"

void ActiveVars::run()
{
    std::ofstream output_active_vars;
    output_active_vars.open("active_vars.json", std::ios::out);
    output_active_vars << "[";
    for (auto &func : this->m_->get_functions()) {
        std::cout<<"current function is "<<func->get_name() << std::endl;
        if (func->get_basic_blocks().empty()) {
            continue;
        }
        else
        {
            func_ = func;  

            func_->set_instr_name();
            live_in.clear();
            live_out.clear();
            use.clear();
            def.clear();
            // 在此分析 func_ 的每个bb块的活跃变量，并存储在 live_in live_out 结构内
            for (auto bb:func_->get_basic_blocks()){
                std::cout<< "current bb is "<< bb->get_name() << std::endl;
                for (auto instr:bb->get_instructions()){
                    if (instr->is_add()
                     || instr->is_sub()
                     || instr->is_mul()
                     || instr->is_div()
                    ){
                        auto lval=static_cast<StoreInst*>(instr)->get_lval();
                        auto rval=static_cast<StoreInst*>(instr)->get_rval();
                        auto constant_ptr=dynamic_cast<ConstantInt *>(lval);
                        if (!constant_ptr){
                            if (def[bb].find(lval)==def[bb].end()) use[bb].insert(lval);
                            for (auto prevbb:bb->get_pre_basic_blocks()){
                                active[prevbb].insert(lval);
                            }
                        }
                        constant_ptr=dynamic_cast<ConstantInt *>(rval);
                        if (!constant_ptr){
                            if (def[bb].find(rval)==def[bb].end()) use[bb].insert(rval);
                            for (auto prevbb:bb->get_pre_basic_blocks()){
                                active[prevbb].insert(lval);
                            }
                        }
                        def[bb].insert(instr);
                    }
                    else if (instr->is_fadd()
                     || instr->is_fsub()
                     || instr->is_fmul()
                     || instr->is_fdiv()
                    ){
                        auto lval=static_cast<StoreInst*>(instr)->get_lval();
                        auto rval=static_cast<StoreInst*>(instr)->get_rval();
                        auto constant_ptr=dynamic_cast<ConstantFP *>(lval);
                        if (!constant_ptr){
                            if (def[bb].find(lval)==def[bb].end()) use[bb].insert(lval);
                            for (auto prevbb:bb->get_pre_basic_blocks()){
                                active[prevbb].insert(lval);
                            }
                        }
                        constant_ptr=dynamic_cast<ConstantFP *>(lval);
                        if (!constant_ptr){
                            if (def[bb].find(rval)==def[bb].end()) use[bb].insert(rval);
                            for (auto prevbb:bb->get_pre_basic_blocks()){
                                active[prevbb].insert(lval);
                            }
                        }
                        def[bb].insert(instr);
                    }
                    else if (instr->is_ret()){
                        if (instr->get_num_operand()){
                            auto tmp=instr->get_operand(0);
                            auto constant_fp_ptr=dynamic_cast<ConstantFP *>(tmp);
                            auto constant_int_ptr=dynamic_cast<ConstantInt *>(tmp);
                            if (!constant_fp_ptr && !constant_int_ptr){
                                if (def[bb].find(tmp)==def[bb].end()){
                                    use[bb].insert(tmp);
                                    for (auto prevbb:bb->get_pre_basic_blocks()){
                                        active[prevbb].insert(tmp);
                                    }
                                }
                            }
                        }
                        def[bb].insert(instr);
                    }
                    else if (instr->is_cmp()){
                        auto lval=static_cast<StoreInst*>(instr)->get_lval();
                        auto rval=static_cast<StoreInst*>(instr)->get_rval();
                        auto constant_ptr=dynamic_cast<ConstantInt *>(lval);
                        if (!constant_ptr){
                            if (def[bb].find(lval)==def[bb].end()) use[bb].insert(lval);
                            for (auto prevbb:bb->get_pre_basic_blocks()){
                                active[prevbb].insert(lval);
                            }
                        }
                        constant_ptr=dynamic_cast<ConstantInt *>(rval);
                        if (!constant_ptr){
                            if (def[bb].find(rval)==def[bb].end()) use[bb].insert(rval);
                            for (auto prevbb:bb->get_pre_basic_blocks()){
                                active[prevbb].insert(rval);
                            }
                        }
                        def[bb].insert(instr);
                    }
                    else if (instr->is_fcmp()){
                        auto lval=static_cast<StoreInst*>(instr)->get_lval();
                        auto rval=static_cast<StoreInst*>(instr)->get_rval();
                        auto constant_ptr=dynamic_cast<ConstantFP *>(lval);
                        if (!constant_ptr){
                            if (def[bb].find(lval)==def[bb].end()) use[bb].insert(lval);
                            for (auto prevbb:bb->get_pre_basic_blocks()){
                                active[prevbb].insert(lval);
                            }
                        }
                        constant_ptr=dynamic_cast<ConstantFP *>(lval);
                        if (!constant_ptr){
                            if (def[bb].find(rval)==def[bb].end()) use[bb].insert(rval);
                            for (auto prevbb:bb->get_pre_basic_blocks()){
                                active[prevbb].insert(rval);
                            }
                        }
                        def[bb].insert(instr);
                    }
                    else if (instr->is_zext()){
                        auto tmp=instr->get_operand(0);
                        if (def[bb].find(tmp)==def[bb].end()){
                            use[bb].insert(tmp);
                            for (auto prevbb:bb->get_pre_basic_blocks()){
                                active[prevbb].insert(tmp);
                            }
                        }
                        def[bb].insert(instr);
                    }
                    else if (instr->is_fp2si()){
                        auto tmp=instr->get_operand(0);
                        auto constant_fp_ptr=dynamic_cast<ConstantFP *>(tmp);
                        if (!constant_fp_ptr){
                            if (def[bb].find(tmp)==def[bb].end()){
                                use[bb].insert(tmp);
                            }
                        }
                        def[bb].insert(instr);
                    }
                    else if (instr->is_si2fp()){
                        auto tmp=instr->get_operand(0);
                        auto constant_int_ptr=dynamic_cast<ConstantInt *>(tmp);
                        if (!constant_int_ptr){
                            if (def[bb].find(tmp)==def[bb].end()){
                                use[bb].insert(tmp);
                                for (auto prevbb:bb->get_pre_basic_blocks()){
                                    active[prevbb].insert(tmp);
                                }
                            }
                        }
                        def[bb].insert(instr);
                    }
                    else if (instr->is_call()){
                        for (int i=1;i<instr->get_num_operand();i++){
                            auto tmp=instr->get_operand(i);
                            auto constant_int_ptr=dynamic_cast<ConstantInt *>(tmp);
                            auto constant_fp_ptr=dynamic_cast<ConstantFP *>(tmp);
                            if (!constant_int_ptr && !constant_fp_ptr){
                                if (def[bb].find(tmp)==def[bb].end()){
                                    use[bb].insert(tmp);
                                    for (auto prevbb:bb->get_pre_basic_blocks()){
                                        active[prevbb].insert(tmp);
                                    }
                                }
                            }
                        }
                        def[bb].insert(instr);
                    }
                    else if (instr->is_phi()){
                        for (int i=0;i<instr->get_num_operand();i+=2){
                            auto tmp=instr->get_operand(i);
                            auto constant_int_ptr=dynamic_cast<ConstantInt *>(tmp);
                            auto constant_fp_ptr=dynamic_cast<ConstantFP *>(tmp);
                            if (!constant_int_ptr && !constant_fp_ptr){
                                if (def[bb].find(tmp)==def[bb].end()){
                                    use[bb].insert(tmp);
                                    for (auto prevbb:bb->get_pre_basic_blocks()){
                                        if (prevbb->get_name()==instr->get_operand(i+1)->get_name())
                                            active[prevbb].insert(tmp);
                                    }
                                }
                            }
                        }
                        def[bb].insert(instr);
                    }
                    else if (instr->is_load()){
                        auto tmp=instr->get_operand(0);
                        auto constant_int_ptr=dynamic_cast<ConstantInt *>(tmp);
                        if (!constant_int_ptr){
                            if (def[bb].find(tmp)==def[bb].end()){
                                use[bb].insert(tmp);
                                for (auto prevbb:bb->get_pre_basic_blocks()){
                                    active[prevbb].insert(tmp);
                                }
                            }
                        }
                        def[bb].insert(instr);
                    }
                    else if (instr->is_gep()){
                        for (int i=0;i<instr->get_num_operand();i++){
                            auto tmp=instr->get_operand(i);
                            auto constant_int_ptr=dynamic_cast<ConstantInt *>(tmp);
                            auto constant_fp_ptr=dynamic_cast<ConstantFP *>(tmp);
                            if (!constant_int_ptr && !constant_fp_ptr){
                                if (def[bb].find(tmp)==def[bb].end()){
                                    use[bb].insert(tmp);
                                    for (auto prevbb:bb->get_pre_basic_blocks()){
                                        active[prevbb].insert(tmp);
                                    }
                                }
                            }
                        }
                        def[bb].insert(instr);
                    }
                    else if (instr->is_alloca()){
                        def[bb].insert(instr);
                    }
                }
                std::cout << "use ";
                for (auto &it:use[bb]){
                    std::cout << it->get_name() << " ";
                }
                std::cout << std::endl;
                std::cout << "def ";
                for (auto &it:def[bb]){
                    std::cout << it->get_name() << " ";
                }
                std::cout << std::endl;
            }
            live_out.clear();
            bool flag=true;
            while (flag){
                flag=false;
                for (auto bb:func_->get_basic_blocks()){
                    for (auto succbb:bb->get_succ_basic_blocks()){
                        for (auto it:live_in[succbb]){
                            if (live_out[bb].find(it)==live_out[bb].end()){
                                if (active[bb].find(it) != active[bb].end())
                                    live_out[bb].insert(it);
                            }
                        }
                    }
                    for (auto it:use[bb]){
                        if (live_in[bb].find(it)==live_in[bb].end()){
                            flag=true;
                            live_in[bb].insert(it);
                        }
                    }
                    for (auto it:live_out[bb]){
                        if (def[bb].find(it)==def[bb].end()){
                            if (live_in[bb].find(it)==live_in[bb].end()){
                                flag=true;
                                live_in[bb].insert(it);
                                for (auto prevbb:bb->get_pre_basic_blocks()){
                                    active[prevbb].insert(it);
                                }
                            }
                        }
                    }
                }
            }
            //analyzation ended
            output_active_vars << print();
            output_active_vars << ",";
        }
    }
    output_active_vars << "]";
    output_active_vars.close();
    return ;
}

std::string ActiveVars::print()
{
    std::string active_vars;
    active_vars +=  "{\n";
    active_vars +=  "\"function\": \"";
    active_vars +=  func_->get_name();
    active_vars +=  "\",\n";

    active_vars +=  "\"live_in\": {\n";
    for (auto &p : live_in) {
        if (p.second.size() == 0) {
            continue;
        } else {
            active_vars +=  "  \"";
            active_vars +=  p.first->get_name();
            active_vars +=  "\": [" ;
            for (auto &v : p.second) {
                active_vars +=  "\"%";
                active_vars +=  v->get_name();
                active_vars +=  "\",";
            }
            active_vars += "]" ;
            active_vars += ",\n";   
        }
    }
    active_vars += "\n";
    active_vars +=  "    },\n";
    
    active_vars +=  "\"live_out\": {\n";
    for (auto &p : live_out) {
        if (p.second.size() == 0) {
            continue;
        } else {
            active_vars +=  "  \"";
            active_vars +=  p.first->get_name();
            active_vars +=  "\": [" ;
            for (auto &v : p.second) {
                active_vars +=  "\"%";
                active_vars +=  v->get_name();
                active_vars +=  "\",";
            }
            active_vars += "]";
            active_vars += ",\n";
        }
    }
    active_vars += "\n";
    active_vars += "    }\n";

    active_vars += "}\n";
    active_vars += "\n";
    return active_vars;
}

