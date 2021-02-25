# lab3 实验报告
学号 姓名

## 问题1: cpp与.ll的对应
请描述你的cpp代码片段和.ll的每个BasicBlock的对应关系。描述中请附上两者代码。

```c++
//assign.cpp
int main()
{
    auto module=new Module("Cminus code");
    auto builder=new IRBuilder(nullptr,module);
    Type *Int32Type = Type::get_int32_type(module);
    
    //main function
    auto mainAssign = Function::create(FunctionType::get(Int32Type,{}),"main",module);
    auto bb=BasicBlock::create(module, "entry", mainAssign);
    builder->set_insert_point(bb);
    auto int_Array_a10 = ArrayType::get(Int32Type,10);
    //-->%1 = alloca [10 x i32], align 16
    auto a10 = builder->create_alloca(int_Array_a10);
    auto a10_0=builder->create_gep(a10,{CONST_INT(0),CONST_INT(0)});
    //%2 = getelementptr [10 x i32], [10 x i32]* %1, i64 0, i64 0 
    builder->create_store(CONST_INT(10),a10_0);
    //store i32 10, i32* %2, align 16
    auto temp1=builder->create_load(a10_0);
    //%3 = load i32, i32* %2, align 16
    auto temp2=builder->create_imul(temp1,{CONST_INT(2)});
    //%4 = mul nsw i32 %3, 2
    auto a10_1=builder->create_gep(a10,{CONST_INT(0),CONST_INT(1)});
    //%5 = getelementptr [10 x i32], [10 x i32]* %1, i64 1, i64 0
    builder->create_store(temp2,a10_1);
    //store i32 %4, i32* %5, align 4
    auto ret=builder->create_load(a10_1);
    //%6 = load i32, i32* %5
    builder->create_ret(ret);
    //ret i32 %6
    std::cout << module->print();
    delete module;
    return 0;
}
```

```c++
//fun.cpp
int main()
{
    auto module=new Module("Cminus code");
    auto builder=new IRBuilder(nullptr,module);
    Type *Int32Type = Type::get_int32_type(module);
    //callee function
    std::vector<Type *> Ints(1,Int32Type);
    auto callee=Function::create(FunctionType::get(Int32Type,Ints),"callee",module);
    auto bb=BasicBlock::create(module, "entry", callee);
    builder->set_insert_point(bb);
    std::vector<Value *>args1;
    for (auto arg=callee->arg_begin();arg!=callee->arg_end();arg++){
        args1.push_back(*arg);
    }
  	//define dso_local i32 @callee(i32 %0) #0{
  	//前面这一大串主要负责构建函数入口，for循环用于获取形式参数
    auto retAlloca=builder -> create_imul(args1[0],{CONST_INT(2)}); //%1 = mul i32 %0, 2
    builder->create_ret(retAlloca);//ret i32 %1
    
    //main function
    auto mainFun=Function::create(FunctionType::get(Int32Type,{}),"main",module);
    bb = BasicBlock::create(module,"entry",mainFun);
    builder->set_insert_point(bb); //define dso_local i32 @main() #0{
    auto call=builder->create_call(callee,{CONST_INT(110)}); //%1 = call i32 @callee(i32 110);
    builder->create_ret(call); //ret i32 %1
    std::cout << module->print();
    delete module;
    return 0;
}
```

```c++
//if.cpp
int main()
{
    auto module=new Module("Cminus code");
    auto builder=new IRBuilder(nullptr,module);
    Type *Int32Type = Type::get_int32_type(module);
    Type *Float32Type = Type::get_float_type(module);

    //main function
    auto mainAssign = Function::create(FunctionType::get(Int32Type,{}),"main",module);
    auto bb=BasicBlock::create(module, "entry", mainAssign);
    builder->set_insert_point(bb);
		//define dso_local i32 @main() #0{
    auto fl_a=builder->create_alloca(Float32Type); //%1 = alloca float, align 4
  	builder->create_store(CONST_FP(5.5),fl_a); //store float 0x40163851E0000000, float* %1, align 4
    auto compa=builder->create_load(fl_a); //%2 = load float, float* %1, align 4
    auto icmp=builder->create_fcmp_gt(compa,CONST_FP(1)); //%3 = fcmp ogt float %2, 0x3ff0000000000000
    auto truebb =BasicBlock::create(module, "truebb",mainAssign);
    auto falsebb=BasicBlock::create(module,"falsebb",mainAssign);
    //auto retbb  =BasicBlock::create(module,"",mainAssign);
    auto br=builder->create_cond_br(icmp,truebb,falsebb); //br i1 %3, label %4, label %5
    //true
    builder->set_insert_point(truebb); //4:
    builder->create_ret(CONST_INT(233)); //ret i32 233
    //false
    builder->set_insert_point(falsebb);//5:
    builder->create_ret(CONST_INT(0)); //ret i32 0
    std::cout << module->print(); 
    delete module;
    return 0;
}
```

```c++
//while.cpp
int main()
{
    auto module=new Module("Cminus code");
    auto builder=new IRBuilder(nullptr,module);
    Type *Int32Type = Type::get_int32_type(module);
    //main function
    auto mainAssign = Function::create(FunctionType::get(Int32Type,{}),"main",module);
    auto bb=BasicBlock::create(module, "entry", mainAssign);
    builder->set_insert_point(bb);
    auto i=builder->create_alloca(Int32Type);//%1 = alloca i32, align 4
    auto a=builder->create_alloca(Int32Type);//%2 = alloca i32, align 4
    builder->create_store(CONST_INT(10),a);  //store i32 10, i32* %1, align 4
    builder->create_store(CONST_INT(0),i);   //store i32 0, i32* %2, align 4
    auto whilebb=BasicBlock::create(module, "",mainAssign);
    auto ifbb=BasicBlock::create(module,"",mainAssign);
    auto endwhilebb=BasicBlock::create(module,"",mainAssign);
    auto br=builder->create_br(whilebb);     //br label %3
    //while judge
    builder->set_insert_point(whilebb);      //3:
    auto cmpi=builder->create_load(i);       //%4 = load i32, i32* %2, align 4
    auto icmp=builder->create_icmp_lt(cmpi,CONST_INT(10)); //%5 = icmp slt i32 %4, 10
    builder->create_cond_br(icmp,ifbb,endwhilebb); //br i1 %5, label %6, label %10
    //while
    builder->set_insert_point(ifbb); //6:
    auto itmp=builder->create_iadd(cmpi,CONST_INT(1)); //%7 = add nsw i32 %4, 1
    builder->create_store(itmp,i); //store i32 %7, i32* %2, align 4
    auto atmp=builder->create_load(a); //%8 = load i32, i32* %1, align 4
    auto newa=builder->create_iadd(atmp,itmp); //%9 = add nsw i32 %7, %8
    builder->create_store(newa,a);//store i32 %9, i32* %1, align 4
    auto whilebr=builder->create_br(whilebb);//br label %3
    //endwhile
    builder->set_insert_point(endwhilebb);//10:
    auto ret=builder->create_load(a);//%11 = load i32, i32* %1, align 4
    builder->create_ret(ret);//ret i32 %11
    std::cout << module->print();
    delete module;
    return 0;
}
```

## 问题2: Visitor Pattern

请指出visitor.cpp中，`treeVisitor.visit(exprRoot)`执行时，以下几个Node的遍历序列:numberA、numberB、exprC、exprD、exprE、numberF、exprRoot。  
序列请按如下格式指明：  
exprRoot->numberF->exprE->numberA->exprD

exprRoot->exprD->exprC->numberA->numberB->exprD->numberB->numberA->numberF
(这是该树的前序遍历)

## 问题3: getelementptr

请给出`IR.md`中提到的两种getelementptr用法的区别,并稍加解释:
  - `%2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 %0` 
      - 2这个变量赋值为数组1第0项的指针的地址值+%0
  - `%2 = getelementptr i32, i32* %1 i32 %0` 
      - 2这个变量赋值为指针1的地址值+%0

## 实验难点
描述在实验中遇到的问题、分析和解决方案

## 实验反馈
吐槽?建议?

assign那个函数好像因为下标取的太简单导致有个坑？