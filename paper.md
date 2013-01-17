%
% xiehuc

实验环境
=======

物理机
-----

CPU 信息

key          value
----------   ---------------------------------
vendor_id     AuthenticAMD
model name    AMD Opteron(TM) Processor 6272
cpu MHz       2100.108
cache size    2048KB
count         64
-----------------------------------------------
Table: Cpu Info

主板信息:

key            value
------------   -------------------------------
Manufacturer   SUGON
Product Name   Server
Serial Number  9800026100264555
UUID           03000200-0400-0500-0006-000700080009
Wake-up Type   Power Switch
----------------------------------------------
Table: Mother board Info

内存信息:

key            value
-----------    ------------------------------
Total Width    72 bits
Data Width     64 bits
Size           4096 MB
Form Factor    DIMM
Locator        DIMM11
Bank Locator   BANK11
Type           DDR3
Type Detail    Synchronous
Speed          1333 MHz
Count          32
Total          128GB
----------------------------------------------
Table: Memory Info

系统信息:

Linux root3-Server 3.2.0-29-generic #46-Ubuntu SMP Fri Jul 27 17:03:23 UTC 2012 x86_64 x86_64 x86_64 GNU/Linux

xen:

key                    value
-------------          -------------------------------------------
host                    root3-Server
release                 3.2.0-29-generic
version                 #46-Ubuntu SMP Fri Jul 27 17:03:23 UTC 2012
machine                 x86_64
nr_cpus                 64
nr_nodes                8
cores_per_socket        16
threads_per_core        1
cpu_mhz                 2100
virt_caps               hvm
total_memory            131054
free_memory             17
free_cpus               0
xen_major               4
xen_minor               1
xen_extra               .2
xen_scheduler           credit
xen_pagesize            4096
platform_params         virt_start=0xffff800000000000
xen_changeset           unavailable
xen_commandline         placeholder
cc_compiler             gcc version 4.6.3 (Ubuntu/Linaro 4.6.3-1ubuntu5) 
cc_compile_by           marc.deslaurier
cc_compile_domain       ubuntu.com
cc_compile_date         Tue Dec 11 16:32:07 UTC 2012
xend_config_format      4
----------------------------------------------------------
Table: xm info


虚拟机
------

5台虚拟机均使用同一规格:

* CPU: 同物理机.只分配一个核心
* Mem: 1024MB
* Mem Dynamic Range: <=2048MB
* System: Ubuntu Server
* uname: Linux ubuntu 3.5.0-17-generic #28-Ubuntu SMP Tue Oct 9 19:31:23 UTC 2012 x86_64 GNU/Linux
* 虚拟化: 全虚拟化

实验过程
=======

基本操作
-------

* 在虚拟机停机的时候设置好maxmem.
 可以用`xm mem-max`设置,或者是virt-manager.
 max值只能在停机的时候设置,在运行的时候设置下次启动有效.
 当分配的时候超过max值,虽然写入了分配的目标值,但是实际上没有分配.
 会造成数据的非一致性.检测方法见note一节

* 在虚拟机之中开启client
 因为有脚本.所以就很方便了.
 需要注意/dev/xen/xenbus文件或者是/proc/xen/xenbus
 自己看脚本内容修改.
	$ sudo service mmclientd start

* 在服务器上运行调节程序
	$ sudo src/mmserver
 之后可以在控制台中看到输出.并且在build文件夹中可以看到log文件.
 log文件的命名规则是:日期_次数_[虚拟机id].log
 次数在这一天之中做试验的次数.如果做了两次试验,那么
 次数有1和2.2是最近做的试验.所以log不会被覆盖.大胆放心的测吧.
 有数据了不管用什么方法.gnuplot,matlab,...或者是excel.
 就可以画图了.

mono测试
--------

* 在guest端安装deb.见上文.
* 在host开启mm_server调节程序.最好是在build文件夹用
   `sudo src/mm_server`启用.因为会在当前目录写log记录.
   千万不要用`sudo make install;sudo mm_server`
   这样是使用的/usr/local/bin目录下面的mm_server.
   那么log记录也会写乱.
* 在guest端使用`mm_test_mono low high`low,high是两个内存数值.
   如`mm_test_mono 50M 400M` M是MB.
* 观察host端的记录.可以看到某个domain id下面的free会变少.total会变多.
   这个是正在调节.
* 打开build文件夹下面的log记录.把一个试验里面的所有记录全部复制出来.
   推荐复制到dataset文件夹.然后设置好文件夹名称.例如date_test_param
   date是日期,test是测试项目名称.如mono,dacapo...param是测试参数
   然后再画图什么的...
   如果对mathematica比较熟悉.可以使用notebook文件夹里面已经写好的样本.
   里面也有已经画出来的图,注意不要覆盖了.用追加的方式贴记录.

dacapo测试
---------

dacapo:

>dacapo是一套用java写成的测试集.利用真实的程序.测试结果能够比较好的体现
>物理世界的真实负载.测试集有很多.有一些是CPU集中型的.一些是内存集中型的.
>以下是dacapo的测试表.

-----------------------------------------------------------------
     testset     description
------------     -------------------------------------------------
avrora           模拟一组程序在AVR微控制器上运行

batik            根据Apache的Batik产生一组SVG图像

eclipse          为Eclipse IDE执行一些非GUI的jdt性能测试

fop              分析和格式化XSL-FO文件,并生成PDF文件

h2               执行类似于JDBCbench的内存评估,执行一组和
                 银行程序模型相关的事务.代替了旧的hsqldb测试

jython           pybench 一个Python的评估解释器.

luindex          使用lucene来建立一些文件的索引;是莎士比亚
                 和King James Bible的作品

lusearch         使用lucene来做一些文本搜索;基于莎士比亚和
                 King James Bible作品中的一些词语

pmd              分析一些Java的Classes文件,检查一些源代码问题.

sunflow          使用光线追踪渲染一些图片

tomcat           运行一些Tomcat服务器的检索和验证网页结果的查询.

tradebeans       runs the daytrader benchmark via a Jave Beans to 
                 a GERONIMO backend with an in memory h2 as the 
                 underlying database

tradesoap        runs the daytrader benchmark via a SOAP to 
                 a GERONIMO backend with in memory h2 
                 as the underlying database

xalan            把XML转换成HTML
-------------------------------------------------------------------
Table: dacapo testset


dacapo测试,需要测以下的4组数据.其中每组数据都是时间.
 **低负载+无调节**,**低负载+开启调节**,**高负载+无调节**,**高负载+开启调节**
其中低负载是通过不执行负载程序来进行的.
高负载是通过开启负载程序来模拟的.
负载程序即是mm_test_static.
为了记录swap值的使用量.可以再开启mm_util_swap.
将所有的测试的消耗时间汇总之后画图.就可以得到workload.nb中的结果了.
因为dacapo的输出比较乱.推荐手工收集数据.

实验结果
======

实验1:mono
---------

**参数**:2台虚拟机,mono从50M到400M

![负载机(左)和空闲机(右)的内存分配](tex/workload_gr1.eps)

三条线依次是总内存分配(蓝),可用内存(黄),使用内存(红).
从图中可以得出:随着使用量的增大.调节程序从空闲机器中取走一部分内存.
给了负载机.而且取走的内存并不严格等于请求的内存.因为负载机的可用内存也在减小.
这个就是由$\tau$值控制的.

实验2:mono
----------

**参数**:5台虚拟机,mono从50M到400M

![负载机和空闲机的内存分配](tex/workload_gr2.eps)

为了能够更好的测量,将虚拟机台数增大到5台.看调节程序是否能正确的处理.
从结果可以看到.和2台时候类似,4台空闲机的总内存都有微小的下降.
这是因为.经过调节之后.从4台空闲机上均分了1台负载机的内存要求.

实验3:dacapo
-----------

**参数**:低负载,开启调节和不开启调节

![低负载调节效果对比](tex/workload_gr3.eps)

由图中得出.在低负载情况下.无论是否开启内存.对CPU影响都不大.
都能够保持高效的运算.

实验4:dacapo
------------

**参数**:高负载(800M),开启调节和不开启调节

![高负载调节效果对比](tex/workload_gr8.eps)

由图中看出.在高负载情况下.开启调节能够大幅度的减少执行时间.
和上图对比,完全来说.是在高负载情况下.程序执行时间大幅增加.而经过调节程序.
能够减缓这种增加趋势.在所有测试中.差距明显的是
eclipse,h2,trade beans,trade scope,xalan.
通过附录中对dacapo测试表格的描述可以知道.这些都是内存集中型的.
而CPU集中型的.差距都不是很明显.基本可以忽略.

为了更好的探究其中的原因.参见下图:

![开启调节和不开启调节SWAP占用率](tex/workload_gr7.eps)

其中蓝色的线条是不开启调节时候.SWAP的占用率.而下面不是非常明显的红色的线条
是开启调节时候的SWAP占用率.
该图有以下的解读:
* 红色的线条比蓝色的短:

>说明开启调节后测试所需要的时间比不开启的短.因为记录是相同的单位时间.
>记录是在测试执行完之后就关闭了.记录的数量少,则总时间就短.
>这个结论和上图中内存集中型测试耗时更少是一致的.

* 红色线条基本没有变化:

>说明开启调节后SWAP分区基本没有使用增加.这个和图1进行参考.知道了因为调节
>程序为负载机分配了更多的内存.使得虽然负载机可用内存依然减少了.但是还完全
>不需要使用SWAP分区.

* 蓝色线条变化剧烈:

>说明不开启调节巨量的使用SWAP分区.因为负载程序的参数是800M.内存一共1000M,
>系统使用100多M.剩下的不足60M.也就是说,负载机已经完全没有什么内存空间执行
>dacapo测试了.所以这个时候只能全部的向SWAP分区申请空间来运行dacapo.

* 不开启调节大量使用SWAP:

>由上面3条结论.我们不难得出负载机器在高负载情况下.开启调节和不开启调节.
>对于内存集中型测试运行时间差异巨大的真正原因:
>不开启调节的时候大量使用SWAP分区.开启调节不用使用SWAP分区.
>SWAP分区的访问速度等于磁盘的访问速度是和内存的访问速度完全不是一个数量级.
>所以导致了不开启调节的时候.消耗时间巨幅增加.

为了探究在开启调节后.负载机没有使用SWAP分区的原因.
参见下图:

![开启调节后所有虚拟机的内存分配](tex/workload_gr9.eps)

该图是在调节时候调节程序记录下来的对所有内存的分配.
等效于实验2中.5个虚拟机的总内存变化曲线的集合.
其中非常突出的曲线是负载机的内存变化.非常的夸张.
剩下4条比较相近的是4台空闲机的内存变化.
中间的分割线是10^6KB,也就是1000MB.

该图时间序有3个阶段:

* 开启调节程序`mm_server`
  此时,经过短暂的震荡,内存数值由调节程序接管.
  调节程序通过各个虚拟机的使用量,计算并赋予新的内存分配.
  负载机内存变化到800M(因为此时刚刚运行完无调节的测试,
  系统内存全部释放完了.所以系统内存只使用了100MB,而其他空闲机.
  系统保持一定的缓存,均在200MB~300MB之间)
* 开启负载程序`mm_test_static 800M`
  此时.负载机器内存很快到达1400MB.并维持不变.
  其他各个机器内存使用均跌至950MB.
  负载机增量$1400-800=600$.
  空闲机负增量$(1100-950)*4=600$.所以也是平衡的.虽然从图上.
  有些视错觉.
* 开启dacapo测试程序.
  经过短暂的时间.是人工反应时间.是测试者从开启负载程序到开启
  dacapo测试程序的自然时间.然后在长时间的范围上.都是dacapo的运行
  时间.而且结合图3,内存分配图上.有一个非常明显的特点.
  就是开启调节时候总内存分配图趋势和不开启调节时候SWAP占用率.
  两者是惊人的一致.
  事实上开启调节时候.负载机总内存的分布有以下关系
  $$总内存=系统占用内存+负载程序内存+dacapo运行内存+剩余内存$$
  其中因为是高负载.系统把所有缓存都让出来了.系统占用内存约为100MB且不变.
  负载程序内存是固定的800M.
  剩余内存不多.大概为60M.dacapo运行内存就是如同图上的波动.随时间变化.
  不恒定.
  SWAP空间在开启调节时候基本没有使用.所以不作考虑.
  另外.我们也可以得出结论:
    + 不开启调节时候SWAP占用量和开启调节时候总内存分配趋势一致.
      换一种说法.总内存扣除负载程序内存和系统占用内存和剩余内存
      基本上等于不开启调节时候的SWAP占用量.
    + 开启调节以后因为调节程序为负载机分配了足量的内存.以至于负载机可以
      不必使用SWAP分区.亦即大幅度的抑制性能的损失.
      在调节过程中.4台空闲机的内存变化和负载机的内存变化基本反比一致.
  并且4台空闲机变化一致.这个和图1,图2的结论是一致的.
  综上:我们最终能够描绘内存的移动:
    + 不开启调节情况下:系统和负载程序基本把总内存占用完了.
      dacapo运行必须借助于SWAP分区.
    + 开启调节情况下:系统和负载程序基本把总内存占用完了的时候.
      调节程序又从其他空闲机器中抽走了内存.并补充给负载机器足
      量内存.所以在dacapo运行时候可以不用使用SWAP分区.所以可以
      减少性能的损失.
* dacapo测试运行完成
  此时,总内存分配再次回到1500MB水平并持续.至于为什么不是
  1400MB.因为负载机并不能完全回到之前的完全一致的条件.
  在开始的时候内存使用量为900MB,结束测试后,内存使用量为1200MB
* 关闭负载程序
  此时负载机内存使用量迅速跌至1050MB左右.同时,其他空闲机的内存
  也恢复至同样的水平.此后很长时间保持不变.

