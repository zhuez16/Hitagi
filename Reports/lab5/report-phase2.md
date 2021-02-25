# Lab5 实验报告

组长 朱恩佐 PB18111704

组员 孙宁 PB18111779

## 实验要求

参考给定的pass，针对cminusfc的中间代码IR实现三个简单的IR优化pass。

## 实验难点

* 常量传播：

一、删除语句对遍历basicblock的iterator的影响。

​	如果在遍历basicblock的链表的时候直接删除语句会导致iterator丢失其后继的信息从而引发segmentation fault。最终我们采用一个todolist来保存所有需要删除的语句的指针，在遍历完成之后删除。

二、常量传播后导致的basicblock删除。

​	有的时候cmp语句会被常量传播掉。这样就会使得有的basicblock会因此而永远不会被进入，因此我们要将其删除。但是因为变成ir以后我们会丢失原本basicblock的结构信息：比如分支可能是if-else-exit也有可能是if-exit，我们不知道在单个br中不会到达的bb是不是exitbb。如果直接删除的话会导致exitbb丢掉，可能会引发错误。我们的解决方案是以另一个bb作为bfs的起点搜索，如果能够搜到这个等待被删除的bb就不删除，否则删除。

三、常量传播的store问题。

​	对于全局变量，由于需要考虑到块间、函数间并不支持常量传播（不然会把循环变量搞崩），所以对于store语句依然需要正常执行。

* 循环不变式外提：
* 活跃变量分析：

一、对于phi语句数据流的处理

​	由于phi语句调用的变量并不像其他语句一样会同时创建来自所有前驱bb的活跃性。所以他不直接满足OUT=IN的方程。最终我们的解决方案是给每一个bb维护一个活跃性链表。其中phi语句只创建一个bb的活跃性，其他语句会创建两个bb的活跃性。在迭代过程中IN=OUT会传递活跃性。

## 实验设计

* 常量传播
    实现思路：
    
    通过判断单个语句的输入是否都是常量来判定当前语句是否需要被传播和删除。
    
    相应代码：
    
    一般语句：
    
    ```c++
    if (instr->is_add() || instr->is_sub()|| instr->is_mul() || instr->is_div()){
        auto r_val=cast_constantint(static_cast<StoreInst *>(instr)->get_rval());
        auto l_val=cast_constantint(static_cast<StoreInst *>(instr)->get_lval());
        if (r_val && l_val){
            Value *tmp=fold->compute(instr->get_instr_type(),r_val,l_val);
            instr->replace_all_use_with(tmp);
            wait_delete.push_back(instr);
        }
    }
    ```
    
    以对于运算的指令为例子：首先判定当前语句的左右值是否都是常数，如果是则将这个语句的值计算出来并将其传播。IR帮我们记录了这个语句的值在后续的使用情况，可以用replace_all_use_with来处理。最后将instr加入带删队列。
    跳转语句：
    
    ```c++
    else if(instr->is_br()){
        if(static_cast<BranchInst*>(instr)->is_cond_br()){
        auto cond=cast_constantint(static_cast<BranchInst*>(instr)->get_operand(0));
        if(cond){
            auto falseBB=bb->get_succ_basic_blocks().front();
            auto trueBB=bb->get_succ_basic_blocks().back();
            if(cond->get_value()==1){
                wait_delete.push_back(instr);
                auto br=BranchInst::create_br(trueBB, bb);
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
    ```
    
    首先检查br语句的条件是否已是定值。如果是的话就考虑删除。删除方法见上面难点。
    
    优化前后的IR对比（举一个例子）并辅以简单说明：
    
    优化前：
    
    ```assembly
    define i32 @max() {
    label_entry:
      %op0 = mul i32 0, 1
      %op1 = mul i32 %op0, 2
      %op2 = mul i32 %op1, 3
      %op3 = mul i32 %op2, 4
      %op4 = mul i32 %op3, 5
      %op5 = mul i32 %op4, 6
      %op6 = mul i32 %op5, 7 ;对于opa的大量计算
      store i32 %op6, i32* @opa
      %op7 = mul i32 1, 2
      %op8 = mul i32 %op7, 3
      %op9 = mul i32 %op8, 4
      %op10 = mul i32 %op9, 5
      %op11 = mul i32 %op10, 6
      %op12 = mul i32 %op11, 7
      %op13 = mul i32 %op12, 8
      store i32 %op13, i32* @opb
      %op14 = mul i32 2, 3
      %op15 = mul i32 %op14, 4
      %op16 = mul i32 %op15, 5
      %op17 = mul i32 %op16, 6
      %op18 = mul i32 %op17, 7
      %op19 = mul i32 %op18, 8
      %op20 = mul i32 %op19, 9
      store i32 %op20, i32* @opc
      %op21 = mul i32 3, 4
      %op22 = mul i32 %op21, 5
      %op23 = mul i32 %op22, 6
      %op24 = mul i32 %op23, 7
      %op25 = mul i32 %op24, 8
      %op26 = mul i32 %op25, 9
      %op27 = mul i32 %op26, 10
      store i32 %op27, i32* @opd
      %op28 = load i32, i32* @opa
      %op29 = load i32, i32* @opb
      %op30 = icmp slt i32 %op28, %op29
      %op31 = zext i1 %op30 to i32
      %op32 = icmp ne i32 %op31, 0
      br i1 %op32, label %label33, label %label39 ; if-exit类型的br
    label33:                                                ; preds = %label_entry
      %op34 = load i32, i32* @opb
      %op35 = load i32, i32* @opc
      %op36 = icmp slt i32 %op34, %op35
      %op37 = zext i1 %op36 to i32
      %op38 = icmp ne i32 %op37, 0
      br i1 %op38, label %label40, label %label46
    label39:                                                ; preds = %label_entry, %label46
      ret i32 0
    label40:                                                ; preds = %label33
      %op41 = load i32, i32* @opc
      %op42 = load i32, i32* @opd
      %op43 = icmp slt i32 %op41, %op42
      %op44 = zext i1 %op43 to i32
      %op45 = icmp ne i32 %op44, 0
      br i1 %op45, label %label47, label %label49
    label46:                                                ; preds = %label33, %label49
      br label %label39
    label47:                                                ; preds = %label40
      %op48 = load i32, i32* @opd
      ret i32 %op48
    label49:                                                ; preds = %label40
      br label %label46
    }
    define void @main() {
    label_entry:
      br label %label1
    label1:                                                ; preds = %label_entry, %label6
      %op15 = phi i32 [ 0, %label_entry ], [ %op9, %label6 ]
      %op3 = icmp slt i32 %op15, 200000000
      %op4 = zext i1 %op3 to i32
      %op5 = icmp ne i32 %op4, 0
      br i1 %op5, label %label6, label %label10
    label6:                                                ; preds = %label1
      %op7 = call i32 @max()
      %op9 = add i32 %op15, 1
      br label %label1
    label10:                                                ; preds = %label1
      %op11 = load i32, i32* @opa
      call void @output(i32 %op11)
      %op12 = load i32, i32* @opb
      call void @output(i32 %op12)
      %op13 = load i32, i32* @opc
      call void @output(i32 %op13)
      %op14 = load i32, i32* @opd
      call void @output(i32 %op14)
      ret void
    }
    ```
    
    优化后：
    
    ```assembly
    define i32 @max() {
    label_entry:
      store i32 0, i32* @opa
      store i32 40320, i32* @opb
      store i32 362880, i32* @opc
      store i32 1814400, i32* @opd
      br label %label33 ;可以看到大量的优化语句直接变成了一个值。跳转也变成了直接跳转。
    label33:                                                ; preds = %label_entry%label_entry
      %op34 = load i32, i32* @opb
      %op35 = load i32, i32* @opc
      %op36 = icmp slt i32 %op34, %op35
      %op37 = zext i1 %op36 to i32
      %op38 = icmp ne i32 %op37, 0
      br i1 %op38, label %label40, label %label46
    label39:                                                ; preds = %label_entry, %label46
      ret i32 0
    label40:                                                ; preds = %label33
      %op41 = load i32, i32* @opc
      %op42 = load i32, i32* @opd
      %op43 = icmp slt i32 %op41, %op42
      %op44 = zext i1 %op43 to i32
      %op45 = icmp ne i32 %op44, 0
      br i1 %op45, label %label47, label %label49
    label46:                                                ; preds = %label33, %label49
      br label %label39
    label47:                                                ; preds = %label40
      %op48 = load i32, i32* @opd
      ret i32 %op48
    label49:                                                ; preds = %label40
      br label %label46
    }
    define void @main() {
    label_entry:
      br label %label1
    label1:                                                ; preds = %label_entry, %label6
      %op15 = phi i32 [ 0, %label_entry ], [ %op9, %label6 ]
      %op3 = icmp slt i32 %op15, 200000000
      %op4 = zext i1 %op3 to i32
      %op5 = icmp ne i32 %op4, 0
      br i1 %op5, label %label6, label %label10
    label6:                                                ; preds = %label1
      %op7 = call i32 @max()
      %op9 = add i32 %op15, 1
      br label %label1
    label10:                                                ; preds = %label1
      %op11 = load i32, i32* @opa
      call void @output(i32 %op11)
      %op12 = load i32, i32* @opb
      call void @output(i32 %op12)
      %op13 = load i32, i32* @opc
      call void @output(i32 %op13)
      %op14 = load i32, i32* @opd
      call void @output(i32 %op14)
      ret void
    }
    ```
    
    


* 循环不变式外提
    实现思路：
    相应代码：
    优化前后的IR对比（举一个例子）并辅以简单说明：
    
* 活跃变量分析
    实现思路：
    
    按照书上的代码直接实现。
    
    相应的代码：
    
    ```c++
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
                        } //OUT=succIN(如果流自这个bb)
                        for (auto it:use[bb]){
                            if (live_in[bb].find(it)==live_in[bb].end()){
                                flag=true;
                                live_in[bb].insert(it);
                            }
                        } //IN=use
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
                        }  //IN=OUT-def
                    }
                }
    ```
    
    

### 实验总结

了解了优化pass的编写方法

对一些能在编译环节中使用到的算法有了一些了解。

### 实验反馈 （可选 不会评分）

希望测试数据多给两组。。。这个太难设计了。。。（比如活跃变量，我们真的得手算一遍每个bb的IN和OUT阿啊阿啊阿啊阿啊）

### 组间交流 （可选）

本次实验和哪些组（记录组长学号）交流了哪一部分信息
