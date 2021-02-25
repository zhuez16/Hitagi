# Lab4 实验报告

队长：朱恩佐    学号：PB18111704

队员：孙宁        学号：PB18111779

## 实验要求

根据cminus-f的语义规则，运用LightIR核心类，实现Program, Var-Declaration...等16个语法分析树节点的翻译方案，实现自动IR产生的算法，使得产生的LLVM IR可以正确编译cminus-f程序。

## 实验难点

实现代码时的难点：

难点1：弄清此次试验的目的以及ast.hpp,cminus-f语法规则，LightIR之间的关系以及如何配合写出builder.cpp

难点2：scope中应该存放的哪些数据，弄清scope在整个程序中的功能并合理运用scope.push,scope.find。

难点3：容易混淆变量中存放的是数值还是地址。

难点4：在实验文档和c++官网上查找并正确运用合适的函数实现相应的功能。

难点5：容易专注于实现语义功能而忘记产生相应的IR指令，比如return函数里忘记写builder->create_ret

难点6：发现需要调用子类的函数，要对父类进行类型转换

难点7：函数类型在函数间的传递

难点8：BasicBlock的实际执行顺序与创建顺序不同的处理。

运行与调试的难点：

难点1：弄清logging具体怎么用

难点2：在编译时只显示segment fault时运用输入输出来找到问题所在。

难点3：人脑跑分析树，找到是具体哪个叶子节点上的问题

## 实验设计

#### 1.全局变量的设置

Value * ret: 存放了var, expression的值。当访问了var, Expression节点后，ret中是归约后节点的值的地址。

Function *currentfunc: 指向了当前运行的函数。

bool func_with_array: 标志当前变量是否为数组

#### 2.数组合法性判断机制

```C++
				if (!func_with_array) func_with_array=true;
        auto idx_illegal_block=BasicBlock::create(module.get(),"",currentfunc);
        auto idx_legal_block=BasicBlock::create(module.get(),"",currentfunc);
        auto icmp=builder->create_icmp_ge(num,Const_int(0));
        builder->create_cond_br(icmp,idx_legal_block,idx_illegal_block);
        builder->set_insert_point(idx_illegal_block);
        auto negid=scope.find("neg_idx_except");
        builder->create_call(negid,{});
```

在ASTVar结点中，当数组下标为负数时，这个数组是非法的。设立分支,当数组非法时，return 0

#### 3.对于Core Dumped的程序的处理

由于我们的环境中gdb无法正常执行，所以我们被迫选择在每一个函数的进入和输出位置设置输出命令，依次判断程序是在进入哪个函数的时候崩掉的。其实感觉效果并不好，经常需要编译好几次才能定位到出错语句。

#### 4.如何降低生成 IR 中的冗余

scope中只存入变量的变量名与数值，其余需要返回的数值都利用全局指针ret返回。可以在临时变量中进行的操作都不用生成IR命令来操作。

#### 5.对于expression的不同类型进行处理

expression语句从最基础的factor开始处理类型。我们选择在运算进行时处理类别。当运算开始时，首先检查两侧的变量是否有指针，如果是指针则将其转换为值；下一步判断是否有布尔类型变量参与了运算。如果有则先将其转换为整型；之后检查两侧的类型是否相同，如果相同则直接计算若否则转换为浮点类型。

我们以term代码为例：

```C++
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
```

由于只有get_type成功返回pointertype才有get_pointer_element_type，所以我们一定要确保如果输入了一个非pointer的东西就一定不要进入else（这么写代码是不是不太好？）。故我们需要先判断当前值是否是整形或浮点型，若是则ret直接获取。然后判断指针的类型，确定合适的load类型。

之后就是类型转换的处理。

```C++
        if (leftvalue->get_type()==int1type)
            leftvalue=builder->create_zext(leftvalue,int32type);
        if (rightvalue->get_type()==int1type)
            leftvalue=builder->create_zext(leftvalue,int32type);
        if (node.op==OP_MUL){
            if (leftvalue->get_type()==int32type && rightvalue->get_type()==int32type)
                ret=builder->create_imul(leftvalue,rightvalue);
            else{  //将其中一个整型变为浮点类型
                if(leftvalue->get_type()==int32type)
                    leftvalue=builder->create_sitofp(leftvalue, floattype);
                else rightvalue=builder->create_sitofp(rightvalue, floattype);
                ret=builder->create_fmul(leftvalue,rightvalue);
            }
        }
        else{
            if (leftvalue->get_type()==int32type && rightvalue->get_type()==int32type)
                ret=builder->create_isdiv(leftvalue,rightvalue);
            else{  //将其中一个整型变为浮点类型
                if(leftvalue->get_type()==int32type)
                    leftvalue=builder->create_sitofp(leftvalue, floattype);
                else rightvalue=builder->create_sitofp(rightvalue, floattype);
                ret=builder->create_fdiv(leftvalue,rightvalue);
            }
        }
    }
```

对于i1类型，由于表达式计算中两侧的值都至少是i32或float，为了偷懒就索性直接先把他们转换成int32类型。（毕竟但凡是个正常然应该不会写3+(1>2)这种阴间东西吧。。。）

之后判断语句是否存在float类型。如果有则这一步运算的返回值就一定是float类型。若没有则是int32类型。分别用对应的语句生成。

#### 6.Call语句对类型的转换

Call语句中需要从scope中读取函数所有参数的类型，但是scope.find这个语句只能给出一个value类型的值，value类型gettype获得的还只能是type类型，而访问get_param_type这个函数需要在Functiontype这个子类里面进行。所以我们进行一次强制类型转换，将type类型转换为FunctionType类型即可。

```C++
    Value* callfunc=scope.find(node.id);
    Type* calltype=callfunc->get_type();
    FunctionType* argstype=(FunctionType*)calltype;
```

这样argstype就可以调用get_param_type来获取各个返回值对应的参数了。

#### 7.BasicBlock的创建顺序与实际中间代码顺序不匹配的处理

由于助教撰写的CreateBlock函数会在创建之后会调用currentfunc的add_basic_block将其添加到bblist的后方。但是比如像如下代码：

```C
if (x==1){ //xtruebb
  if (y==2){//ytruebb
    return 1;
  }
  else{//yfalsebb
    return 3;
  }
}
else{//xfalsebb
  return 2;
}
```

我们在编译中应当先进入xtruebb来创建ytruebb和yfalsebb，然后进入xfalsebb进行翻译。但是如果只是createblock并br的话最终blocklist的组织顺序是xtruebb, xfalsebb, ytruebb, yfalsebb，是错误的。

我们最终经过缜密的思考和阅读我们进行了如下处理：

```c++
				auto trueBB=BasicBlock::create(module.get(),"",currentfunc);
        auto falseBB=BasicBlock::create(module.get(),"",currentfunc);
        auto exitBB=BasicBlock::create(module.get(),"",currentfunc);
        builder->create_cond_br(icmp,trueBB,falseBB);
        falseBB->erase_from_parent(); //将falseBB和trueBB从func的blocklist中先移除再插入。
        exitBB->erase_from_parent();
        builder->set_insert_point(trueBB);
        //truebb generation code
        currentfunc->add_basic_block(falseBB);
        builder->set_insert_point(falseBB);
        //falsebb generation code
```

在创建之后我们先将falseBB和exitBB从list中删除。这样list的结尾就是truebb。这时如果里面创建了新的bb就可以正确地接入list的结尾。在trueBB内部的语句全部处理完了之后也就应当是处理falsebb时再加入list。这样就可以让Selectionstmt正确地创建BasicBlock。

Iteration的处理大致相同。在这里不再单独讲解，仅放出代码。

```C++
		auto whileBB=BasicBlock::create(module.get(),"",currentfunc);
    auto trueBB=BasicBlock::create(module.get(),"",currentfunc);
    auto falseBB=BasicBlock::create(module.get(),"",currentfunc);
    builder->create_br(whileBB);
    builder->set_insert_point(whileBB);
    //while cmp generation block
    builder->create_cond_br(icmp,trueBB,falseBB);
    //while generation block
    builder->set_insert_point(trueBB);
    falseBB->erase_from_parent();
    node.statement->accept(*this);
    builder->create_br(whileBB);
    //falseBB
    currentfunc->add_basic_block(falseBB);
    builder->set_insert_point(falseBB);
```

#### 8.对于if语句中出现return语句的处理

if语句会生成truebb和falsebb两个basicblock。而LLVM要求一个block只允许有一个Terminator，所以需要处理一下保证当前block不会同时出现return和br exitbb两个Terminator。好在助教帮助我们实现了get_terminator()这个函数，可以简单快捷地获取当前函数的终结情况。我们只需要在create_br的时候判断一下函数是否已经返回了即可。

代码如下：

```c++
node.if_statement->accept(*this);
bool isreturned=true;
if (builder->get_insert_block()->get_terminator()==nullptr){
    builder->create_br(exitBB);
    isreturned=false;
}
```

#### 9.对于单独分号的处理

在某一次测试中我们发现对于这样的程序我们竟然无法编译：

```c
void main(void) {
    input();;
    return;
}
```

经过了仔细的检查~~和1w次cout调试~~发现我们的Expressionstmt是无条件访问Expression的。而显然这样就会访问一个nullptr，进而导致错误。

当然这也没什么不好处理的。

```c++
void CminusfBuilder::visit(ASTExpressionStmt &node) {
    if (node.expression) node.expression->accept(*this);
}
```

在访问之前判断一下存在性即可。

#### 10.return以后依然存在语句的处理

对于这样一个函数：

```c
int sbinala(){
    return 3;
    return 4;
  	return 6;
}
```

如果我们不进行任何处理，那么编译器就会在函数这个bb中创建三个ret函数，而显然这会爆炸。

处理方法就是我们在一个Compoundstmt中判断当前这个basicblock是否已经返回。如果返回了我们可以直接跳过当前的compundstmt，也就是说这些语句就不需要再翻译了。

```C++
void CminusfBuilder::visit(ASTCompoundStmt &node) {
    scope.enter();
    for (auto decl:node.local_declarations)
        decl->accept(*this);
    for (auto stmt:node.statement_list){
        if (builder->get_insert_block()->get_terminator()==nullptr)
            stmt->accept(*this);
    }
    scope.exit();
}
```

### 实验总结

更加深刻的明白了LR分析树和语法制导翻译的过程。理解了面向对象的编程语言的特点和编程思路，比较熟练的运用C++语言。

熟悉了git版本的管理方法和组队开发的流程，提升了队员们的工程实践能力。

对于各个Cminusf语言的文法，特别是作用域相关的内容进行了更加深刻的了解。~~可以更好地回答（怼）程序设计1课程同学们奇妙深刻的语法问题。~~

对C++语言class类别的构建和访问者模式有了一些了解。至少如果有人再甩给组内成员一段C++代码不至于还要google一下语法才能读得懂了。

另外本次试验也有一些设计上的失误和问题，希望助教解答：

* 我们对于factor中规约出来的var一直是用的时候才进行load/gep。但实际上我们这一次的语义似乎factor规约出来的var可以直接转换成数，而不用带着指针一直传。（不过考虑到Cminusf并不支持地址加减法，是不是如果我们要支持地址加减的话就需要在用的时候load/gep了？）
* 对于Selection和Iteration，我们选择了很暴力的pop and push来维护正确的bb顺序。是否更好的操作模式？
* 目前我们只需要处理i1、i32、float这三种类型的情况下我们的类型转化就已经很复杂了。考虑到可能后续加入char、double、long long等一系列类型，是否有一种更好的类型转化判断系统？

### 实验反馈 （可选 不会评分）

1、感觉Basicblock::create这个语句是不是不需要直接调用add_basic_block？而是create之后在add_insert_point()的时候加入？

2、似乎助教配的环境里面gdb有点问题？

3、测试数据也太弱了吧。。。希望助教至少能让测试数据涵盖所有的基本语法啊。。。这12个点连while循环都没有也太搞人心态了吧。。。

### 组间交流 （可选）

本次实验和哪些组（记录组长学号）交流了哪一部分信息
