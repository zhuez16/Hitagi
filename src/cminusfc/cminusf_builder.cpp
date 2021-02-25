#include "cminusf_builder.hpp"

// use these macros to get constant value
#define CONST_FP(num) \
    ConstantFP::get((float)num, module.get())
#define CONST_ZERO(type) \
    ConstantZero::get(var_type, module.get())


// You can define global variables here
// to store state
#define Const_int(num) \
    ConstantInt::get(num,module.get())
#define Const_fp(num) \
    ConstantFP::get(num,module.get())
Value *ret;
Function *currentfunc;
bool func_with_array;
/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope
 * scope.exit: exit current scope
 * scope.push: add a new binding to current scope
 * scope.find: find and return the value bound to the name
 */
//这是个测试

void CminusfBuilder::visit(ASTProgram &node) {
    //Program -> declaration-list
    for (auto decl: node.declarations){
        decl->accept(*this);
    }
}

void CminusfBuilder::visit(ASTNum &node) {
    if (node.type==CminusType::TYPE_INT){
        ret=Const_int(node.i_val);
    }
    else if (node.type==CminusType::TYPE_FLOAT){
        ret=Const_fp(node.f_val);
    }
}

void CminusfBuilder::visit(ASTVarDeclaration &node) {
    Type* int32type=Type::get_int32_type(module.get());
    Type* floattype=Type::get_float_type(module.get());
    if (!CminusfBuilder::scope.in_global()){
        if (node.num){ //local array declaration
            ArrayType* arraytype;
            if (node.type==CminusType::TYPE_INT)
                arraytype=ArrayType::get(int32type,node.num->i_val);
            else
                arraytype=ArrayType::get(floattype,node.num->f_val);
            auto localarray=builder->create_alloca(arraytype);
            scope.push(node.id,localarray);
        }
        else{
            if (node.type==CminusType::TYPE_INT){
                auto localvar=builder->create_alloca(int32type);
                scope.push(node.id,localvar);
            }
            else{
                auto localvar=builder->create_alloca(floattype);
                scope.push(node.id,localvar);
            }
        }
    }
    else{
        if (node.num){
            ArrayType* arraytype;
            auto initializer=ConstantZero::get(int32type,module.get());
            if (node.type==CminusType::TYPE_INT){
                arraytype=ArrayType::get(int32type,node.num->i_val);
            }
            else{
                arraytype=ArrayType::get(floattype,node.num->i_val);
                initializer=ConstantZero::get(floattype,module.get());
            }
            auto glarray=GlobalVariable::create(node.id,module.get(),arraytype,false,initializer);
            scope.push(node.id,glarray);
        }
        else{
            if (node.type==CminusType::TYPE_INT){
                auto initializer=ConstantZero::get(int32type,module.get());
                auto globalvar=GlobalVariable::create(node.id,module.get(),int32type,false,initializer);
                scope.push(node.id,globalvar);
            }
            else{
                auto initializer=ConstantZero::get(floattype,module.get());
                auto globalvar=GlobalVariable::create(node.id,module.get(),floattype,false,initializer);
                scope.push(node.id,globalvar);
            }
        }
    }
}

void CminusfBuilder::visit(ASTFunDeclaration &node) {
    //函数的作用域从参数声明开始.
    scope.enter();
    func_with_array=false;
    //fundeclaration -> type-specifier id (params) {}
    Type* int32type=Type::get_int32_type(module.get());
    Type* floattype=Type::get_float_type(module.get());
    Type* voidtype =Type::get_void_type (module.get());
    Type* int32ptrtype=Type::get_int32_ptr_type(module.get());
    Type* floatptrtype=Type::get_float_ptr_type(module.get());
    std::vector<Type *> argstype;
    if (node.params.size()>0){
        for (auto p:node.params){
            if (p->isarray){
                if (p->type==CminusType::TYPE_INT)
                    argstype.push_back(int32ptrtype);
                else
                    argstype.push_back(floatptrtype);
            }
            else{
                if (p->type==CminusType::TYPE_INT)
                    argstype.push_back(int32type);
                else
                    argstype.push_back(floattype);
            }
        }
    }
    Type* functype;
    if (node.type==CminusType::TYPE_INT){
        functype=int32type;
    }
    else if (node.type==CminusType::TYPE_FLOAT){
        functype=floattype;
    }
    else{
        functype=voidtype;
    }
    auto func=Function::create(FunctionType::get(functype,argstype),node.id,module.get());
    scope.exit();
    scope.push(node.id,func);
    scope.enter();
    auto bb=BasicBlock::create(module.get(),"",func);
    currentfunc=func;
    builder->set_insert_point(bb);
    //store parameters' value
    for (auto param:node.params){
        param->accept(*this);
    }
    std::vector<Value *>argsvalue;
    for (auto arg=func->arg_begin();arg!=func->arg_end();arg++)
        argsvalue.push_back(*arg);
    if (node.params.size() && argsvalue.size()){
        int i=0;
        for (auto arg:node.params){
            auto p=scope.find(arg->id);
            builder->create_store(argsvalue[i++],p);
        }
    }
    if (node.compound_stmt){
        node.compound_stmt->accept(*this);
    }
    scope.exit();
}

void CminusfBuilder::visit(ASTParam &node) {
    // param -> type-specifier id ; type-specifier id []
    Type* int32type=Type::get_int32_type(module.get());
    Type* floattype=Type::get_float_type(module.get());
    Type* voidtype =Type::get_void_type (module.get());
    Type* int32ptrtype=Type::get_int32_ptr_type(module.get());
    Type* floatptrtype=Type::get_float_ptr_type(module.get());
    if(node.isarray){ //指针形参
        if (node.type==CminusType::TYPE_INT){
            auto param_ptr=builder->create_alloca(int32ptrtype);
            scope.push(node.id,param_ptr);
        }
        else{
            auto param_ptr=builder->create_alloca(floatptrtype);
            scope.push(node.id,param_ptr);
        }
    }
    else{
        if(node.type==CminusType::TYPE_INT){
            auto param=builder->create_alloca(int32type);
            scope.push(node.id, param);
        }
        else if(node.type==CminusType::TYPE_FLOAT){
            auto param=builder->create_alloca(floattype);
            scope.push(node.id, param);
        }
            
    }
}

void CminusfBuilder::visit(ASTCompoundStmt &node) {
    //compoundstmt->localdeclaration|statementlist
    scope.enter();
    for (auto decl:node.local_declarations)
        decl->accept(*this);
    for (auto stmt:node.statement_list){
        if (builder->get_insert_block()->get_terminator()==nullptr)
            stmt->accept(*this);
    }
    scope.exit();
}

void CminusfBuilder::visit(ASTExpressionStmt &node) {
    if (node.expression) node.expression->accept(*this);
}

void CminusfBuilder::visit(ASTSelectionStmt &node) {
    Type* int32type=Type::get_int32_type(module.get());
    Type* floattype=Type::get_float_type(module.get());
    node.expression->accept(*this);
    if (ret->get_type()->is_pointer_type()) ret=builder->create_load(ret);
    else if (ret->get_type()->is_array_type()) ret=builder->create_gep(ret,{Const_int(0),Const_int(0)});
    auto icmp=ret;
    if (icmp->get_type()==int32type) icmp=builder->create_icmp_gt(icmp,Const_int(0));
    else if (icmp->get_type()==floattype) icmp=builder->create_fcmp_gt(icmp,Const_fp(0));
    if (node.else_statement!=nullptr){
        auto trueBB=BasicBlock::create(module.get(),"",currentfunc);
        auto falseBB=BasicBlock::create(module.get(),"",currentfunc);
        auto exitBB=BasicBlock::create(module.get(),"",currentfunc);
        builder->create_cond_br(icmp,trueBB,falseBB);
        falseBB->erase_from_parent();
        exitBB->erase_from_parent();
        builder->set_insert_point(trueBB);
        node.if_statement->accept(*this);
        bool isreturned=true;
        if (builder->get_insert_block()->get_terminator()==nullptr){
            builder->create_br(exitBB);
            isreturned=false;
        }
        currentfunc->add_basic_block(falseBB);
        builder->set_insert_point(falseBB);
        node.else_statement->accept(*this);
        if (builder->get_insert_block()->get_terminator()==nullptr){
            builder->create_br(exitBB);
            isreturned=false;
        }
        if (!isreturned){
            currentfunc->add_basic_block(exitBB);
            builder->set_insert_point(exitBB);
        }
    }
    else{
        auto trueBB=BasicBlock::create(module.get(),"",currentfunc);
        auto falseBB=BasicBlock::create(module.get(),"",currentfunc);
        builder->create_cond_br(icmp,trueBB,falseBB);
        falseBB->erase_from_parent();
        builder->set_insert_point(trueBB);
        node.if_statement->accept(*this);
        currentfunc->add_basic_block(falseBB);
        if (builder->get_insert_block()->get_terminator()==nullptr)
            builder->create_br(falseBB);
        builder->set_insert_point(falseBB);
    }
}

void CminusfBuilder::visit(ASTIterationStmt &node) {
    //while(expression)statement
    Type* int32type=Type::get_int32_type(module.get());
    Type* floattype=Type::get_float_type(module.get());
    auto whileBB=BasicBlock::create(module.get(),"",currentfunc);
    auto trueBB=BasicBlock::create(module.get(),"",currentfunc);
    auto falseBB=BasicBlock::create(module.get(),"",currentfunc);
    builder->create_br(whileBB);
    //whileBB
    builder->set_insert_point(whileBB);
    node.expression->accept(*this);
    if (ret->get_type()->is_pointer_type()) ret=builder->create_load(ret);
    else if (ret->get_type()->is_array_type()) ret=builder->create_gep(ret,{Const_int(0),Const_int(0)});
    auto icmp=ret;
    if (icmp->get_type()==int32type) icmp=builder->create_icmp_gt(icmp,Const_int(0));
    else if (icmp->get_type()==floattype) icmp=builder->create_fcmp_gt(icmp,Const_fp(0));
    builder->create_cond_br(icmp,trueBB,falseBB);
    //trueBB
    builder->set_insert_point(trueBB);
    falseBB->erase_from_parent();
    node.statement->accept(*this);
    builder->create_br(whileBB);
    //falseBB
    currentfunc->add_basic_block(falseBB);
    builder->set_insert_point(falseBB);
}

void CminusfBuilder::visit(ASTReturnStmt &node) {
    //return;|return expression
    //返回值与函数声明不一样需要类型转换
    Type* int32type=Type::get_int32_type(module.get());
    Type* floattype=Type::get_float_type(module.get());
    auto returntype=currentfunc->get_return_type();  //currentfunc 的返回值类型
    if(node.expression!=nullptr){   //返回类型不为空
        node.expression->accept(*this);
        if (ret->get_type()->is_pointer_type()){
            if (ret->get_type()->get_pointer_element_type()==int32type)
                ret=builder->create_load(int32type,ret);
            else
                ret=builder->create_load(floattype,ret);
        }
        if(returntype==int32type&&ret->get_type()==floattype){ // 函数返回值类型为int但ret类型为float
            auto tmp=builder->create_fptosi(ret, int32type);
            builder->create_ret(tmp);
        }
        // 函数返回类型为float但ret类型为int
        else if(returntype==floattype&&ret->get_type()==int32type){
            auto tmp=builder->create_sitofp(ret, floattype);
            builder->create_ret(tmp);
        }
        else{
            builder->create_ret(ret);
        }
    }
    else{
        builder->create_void_ret();
    }
    //函数返回值类型为void  什么都不返回
}

void CminusfBuilder::visit(ASTVar &node) {
    Type* int32type=Type::get_int32_type(module.get());
    Type* floattype=Type::get_float_type(module.get());
    Type* int32ptrtype=Type::get_int32_ptr_type(module.get());
    Type* floatptrtype=Type::get_float_ptr_type(module.get());
    auto id=scope.find(node.id);
    if (node.expression==nullptr){
        ret=id;
    }
    else{
        node.expression->accept(*this);
        auto num=ret;
        if (num->get_type()==int32type) num=num;
        else if (num->get_type()==floattype) num=builder->create_fptosi(num,int32type);
        else if (num->get_type()->get_pointer_element_type()==int32type) num=builder->create_load(num);
        else if (num->get_type()->get_pointer_element_type()==floattype){
            num=builder->create_load(num);
            num=builder->create_fptosi(num,int32type);
        }
        else if (num->get_type()->get_pointer_element_type()->is_array_type()){
            num=builder->create_gep(num,{Const_int(0),Const_int(0)});
            if (num->get_type()==floattype) num=builder->create_fptosi(num,int32type);
        }
        else if (!num->get_type()->is_array_type()){
            num=builder->create_load(num);
            if (num->get_type()==floattype) num=builder->create_fptosi(num,int32type);
        }
        if (!func_with_array) func_with_array=true;
        auto idx_illegal_block=BasicBlock::create(module.get(),"",currentfunc);
        auto idx_legal_block=BasicBlock::create(module.get(),"",currentfunc);
        auto icmp=builder->create_icmp_ge(num,Const_int(0));
        builder->create_cond_br(icmp,idx_legal_block,idx_illegal_block);
        builder->set_insert_point(idx_illegal_block);
        auto negid=scope.find("neg_idx_except");
        builder->create_call(negid,{});
        if (currentfunc->get_return_type()==int32type) builder->create_ret(Const_int(0));
        else if (currentfunc->get_return_type()==floattype) builder->create_ret(Const_fp(0));
        else builder->create_void_ret();
        builder->set_insert_point(idx_legal_block);

        auto* idpload=builder->create_load(id);
        if (idpload->get_type()->is_array_type()){
            auto idgep=builder->create_gep(id,{Const_int(0),num});
            ret=idgep;
        }
        else{
            auto idgep=builder->create_gep(idpload,{num});
            ret=idgep;
        }
    }
}

void CminusfBuilder::visit(ASTAssignExpression &node) {
    //expression 的返回值都放在ret中
    //变量已经在声明的时候分配了内存空间
    //var = expression
    Type* int32type=Type::get_int32_type(module.get());
    Type* floattype=Type::get_float_type(module.get());
    node.var->accept(*this);
    auto assign_var=ret;
    node.expression->accept(*this);
    //如果赋值语句两边类型不同，进行类型转换
    // var is of int type
    if (ret->get_type()->is_pointer_type()){
        ret=builder->create_load(ret);
    }
    if(assign_var->get_type()->get_pointer_element_type()==ret->get_type())  // expression's type == var's type
        builder->create_store(ret, assign_var);
    else if (ret->get_type()==floattype){ //expression is of float type  强制转换类型
        ret=builder->create_fptosi(ret, int32type);
        builder->create_store(ret, assign_var);
    }
    else{
        ret=builder->create_sitofp(ret,floattype);
        builder->create_store(ret, assign_var);
    }
}

void CminusfBuilder::visit(ASTSimpleExpression &node) {
    //addexpr-l relop addexpr-r
    Type* int32type=Type::get_int32_type(module.get());
    Type* floattype=Type::get_float_type(module.get());
    Type* int1type=Type::get_int1_type(module.get());
    if (node.additive_expression_r==nullptr){
        node.additive_expression_l->accept(*this);
    }
    else{
        node.additive_expression_l->accept(*this);
        Value* lval;
        if (ret->get_type()==int32type || ret->get_type()==floattype || ret->get_type()==int1type){
            lval=ret;
        }
        else if (ret->get_type()->get_pointer_element_type()==int32type){
            lval=builder->create_load(int32type,ret);
        }
        else{
            lval=builder->create_load(floattype,ret);
        }
        node.additive_expression_r->accept(*this);
        Value* rval;
        if (ret->get_type()==int32type || ret->get_type()==floattype || ret->get_type()==int1type){
            rval=ret;
        }
        else if (ret->get_type()->get_pointer_element_type()==int32type){
            rval=builder->create_load(int32type,ret);
        }
        else{
            rval=builder->create_load(floattype,ret);
        }
    //二元运算两边类型不一样，强制类型转换  int 转 float
        if (lval->get_type()==int1type) lval=builder->create_zext(lval,int32type);
        if (rval->get_type()==int1type) rval=builder->create_zext(rval,int32type);
        if(lval->get_type()==floattype&&rval->get_type()==int32type)
            builder->create_sitofp(rval, floattype);
        else if(rval->get_type()==floattype&&lval->get_type()==int32type)
        builder->create_sitofp(lval, floattype);
    
    //判断ret的值
    /* <= OP_LE //< OP_LT,// > OP_GT,// >= OP_GE, // == OP_EQ, // !=  OP_NEQ*/
        if(lval->get_type()==int32type){// lval rval  int32type  icmp
            switch (node.op) {
                case OP_LE:
                    ret=builder->create_icmp_le(lval, rval);
                    break;
                case OP_LT:
                    ret=builder->create_icmp_lt(lval, rval);
                    break;
                case OP_GT:
                    ret=builder->create_icmp_gt(lval, rval);
                    break;
                case OP_GE:
                    ret=builder->create_icmp_ge(lval, rval);
                    break;
                case OP_EQ:
                    ret=builder->create_icmp_eq(lval, rval);
                    break;
                default:
                    ret=builder->create_icmp_ne(lval, rval);
            }
        }
        else { //lval rval  floattype  fcmp
            switch (node.op) {
                case OP_LE:
                    ret=builder->create_fcmp_le(lval, rval);
                    break;
                case OP_LT:
                    ret=builder->create_fcmp_lt(lval, rval);
                    break;
                case OP_GT:
                    ret=builder->create_fcmp_gt(lval, rval);
                    break;
                case OP_GE:
                    ret=builder->create_fcmp_ge(lval, rval);
                    break;
                case OP_EQ:
                    ret=builder->create_fcmp_eq(lval, rval);
                    break;
                default:
                    ret=builder->create_fcmp_ne(lval, rval);
                    break;
            }
        }
    }
}


void CminusfBuilder::visit(ASTAdditiveExpression &node) {
    Type* int32type=Type::get_int32_type(module.get());
    Type* floattype=Type::get_float_type(module.get());
    Type* int1type=Type::get_int1_type(module.get());
    Type* int32ptrtype=Type::get_int32_ptr_type(module.get());
    Type* floatptrtype=Type::get_float_ptr_type(module.get());
    if (node.additive_expression==nullptr){
        //additive_expression -> term
        node.term->accept(*this);
    }
    else{
        //additive_expression -> additive_expression addop term
        node.additive_expression->accept(*this);
        Value* leftvalue;
        if (ret->get_type()==int32type || ret->get_type()==floattype || ret->get_type()==int1type){
            leftvalue=ret;
        }
        else if (ret->get_type()->get_pointer_element_type()==int32type){
            leftvalue=builder->create_load(int32type,ret);
        }
        else{
            leftvalue=builder->create_load(floattype,ret);
        }
        node.term->accept(*this);
        Value* rightvalue;
        if (ret->get_type()==int32type || ret->get_type()==floattype || ret->get_type()==int1type){
            rightvalue=ret;
        }
        else if (ret->get_type()->get_pointer_element_type()==int32type){
            rightvalue=builder->create_load(int32type,ret);
        }
        else{
            rightvalue=builder->create_load(floattype,ret);
        }
        if (leftvalue->get_type()==int1type) leftvalue=builder->create_zext(leftvalue,int32type);
        if (rightvalue->get_type()==int1type) rightvalue=builder->create_zext(rightvalue,int32type);
        if (node.op==OP_PLUS){
            if (leftvalue->get_type()==int32type && rightvalue->get_type()==int32type)
                ret=builder->create_iadd(leftvalue,rightvalue);
            else{ //整型和浮点数运算，整型变为浮点类型
                if(leftvalue->get_type()==int32type)   leftvalue=builder->create_sitofp(leftvalue, floattype);
                else rightvalue=builder->create_sitofp(rightvalue, floattype);
                ret=builder->create_fadd(leftvalue,rightvalue);
            }
        }
        else{
            if (leftvalue->get_type()==int32type && rightvalue->get_type()==int32type)
                ret=builder->create_isub(leftvalue,rightvalue);
            else{//整型和浮点数运算，整型变为浮点类型
                if(leftvalue->get_type()==int32type)
                    leftvalue=builder->create_sitofp(leftvalue, floattype);
                else rightvalue=builder->create_sitofp(rightvalue, floattype);
                ret=builder->create_fsub(leftvalue,rightvalue);
            }
        }
    }
}

void CminusfBuilder::visit(ASTTerm &node) {
    Type* int32type=Type::get_int32_type(module.get());
    Type* floattype=Type::get_float_type(module.get());
    Type* int1type=Type::get_int1_type(module.get());
    Type* int32ptrtype=Type::get_int32_ptr_type(module.get());
    Type* floatptrtype=Type::get_float_ptr_type(module.get());
    if (node.term==nullptr){
        //additive_expression -> term
        node.factor->accept(*this);
    }
    else{
        //additive_expression -> additive_expression addop term
        node.term->accept(*this);
        Value* leftvalue;
        if (ret->get_type()==int32type || ret->get_type()==floattype || ret->get_type()==int1type){
            leftvalue=ret;
        }
        else if (ret->get_type()->get_pointer_element_type()==int32type){
            leftvalue=builder->create_load(int32type,ret);
        }
        else{
            leftvalue=builder->create_load(floattype,ret);
        }
        node.factor->accept(*this);
        Value* rightvalue;
        if (ret->get_type()==int32type || ret->get_type()==floattype || ret->get_type()==int1type){
            rightvalue=ret;
        }
        else if (ret->get_type()->get_pointer_element_type()==int32type){
            rightvalue=builder->create_load(int32type,ret);
        }
        else{
            rightvalue=builder->create_load(floattype,ret);
        }
        if (leftvalue->get_type()==int1type) leftvalue=builder->create_zext(leftvalue,int32type);
        if (rightvalue->get_type()==int1type) leftvalue=builder->create_zext(leftvalue,int32type);
        if (node.op==OP_MUL){
            if (leftvalue->get_type()==int32type && rightvalue->get_type()==int32type)
                ret=builder->create_imul(leftvalue,rightvalue);
            else{  //将其中一个整型变为浮点类型
                if(leftvalue->get_type()==int32type)   leftvalue=builder->create_sitofp(leftvalue, floattype);
                else rightvalue=builder->create_sitofp(rightvalue, floattype);
                ret=builder->create_fmul(leftvalue,rightvalue);
            }
        }
        else{
            if (leftvalue->get_type()==int32type && rightvalue->get_type()==int32type)
                ret=builder->create_isdiv(leftvalue,rightvalue);
            else{  //将其中一个整型变为浮点类型
                if(leftvalue->get_type()==int32type)   leftvalue=builder->create_sitofp(leftvalue, floattype);
                else rightvalue=builder->create_sitofp(rightvalue, floattype);
                ret=builder->create_fdiv(leftvalue,rightvalue);
            }
        }
    }
}

void CminusfBuilder::visit(ASTCall &node) {
    Value* callfunc=scope.find(node.id);
    Type* int32type=Type::get_int32_type(module.get());
    Type* floattype=Type::get_float_type(module.get());
    Type* int32ptrtype=Type::get_int32_ptr_type(module.get());
    Type* floatptrtype=Type::get_float_ptr_type(module.get());
    Type* calltype=callfunc->get_type();
    FunctionType* argstype=(FunctionType*)calltype;
    std::vector<Value *> callargs;
    if (node.args.size()){
        int i=0;
        for (auto args:node.args){
            args->accept(*this);
            if (ret->get_type()==int32type){
                //return an integer type
                if (argstype->get_param_type(i)==floattype)
                    ret=builder->create_sitofp(ret,floattype);
                callargs.push_back(ret);
            }
            else if (ret->get_type()==floattype){
                if (argstype->get_param_type(i)==int32type)
                    ret=builder->create_sitofp(ret,int32type);
                //return an floattype
                callargs.push_back(ret);
            }
            else if (ret->get_type()->get_pointer_element_type()==int32type){
                //return a pointer to int
                ret=builder->create_load(ret);
                if (argstype->get_param_type(i)==floattype)
                    ret=builder->create_sitofp(ret,floattype);
                callargs.push_back(ret);
            }
            else if (ret->get_type()->get_pointer_element_type()==floattype){
                //return a pointer to float
                ret=builder->create_load(ret);
                if (argstype->get_param_type(i)==int32type)
                    ret=builder->create_fptosi(ret,int32type);
                callargs.push_back(ret);
            }
            else if (ret->get_type()->get_pointer_element_type()->is_array_type()){
                //return a array
                callargs.push_back(builder->create_gep(ret,{Const_int(0),Const_int(0)}));
            }
            else if (!ret->get_type()->get_pointer_element_type()->is_array_type()){
                //return a array's pointer
                callargs.push_back(builder->create_load(ret));
            }
            i++;
        }
    }
    ret=builder->create_call(callfunc,callargs);
}
