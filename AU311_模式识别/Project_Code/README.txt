PR_plus说明文件

@author  C_zihao, DJ Chen ,Shu Tao
@Address  
@Group    1
@Data     2020.12.21

基本算法思路
-----尝试算法：  
----------------预处理:   归一化，PCA降维 ，半监督
----------------传统分类算法：SVM, KNN, Softmax
----------------搭建分类网络：BP, LSTM, CNN
----------------其它： Ensemble vote
--------------------------------------------------------------------
------最终框架：
----------------半监督KNN生成伪标签，选取置信度高的数据形成新的训练数据，归一
----------------使用SVM，常规KNN，Softmax，Softmax网络，Lstm网络，CNN网络进行训练，模型存储
----------------测试，Bagging vote生成结果

效果测试（准确率）
--------算法-----------未使用半监督-----------融合半监督  
-------SVM-----------   79.4%        -----------    87.87%
-------KNN-----------    84%          -----------    88.13%
------Softmax--------  81.23%       -----------    87.33%
----Softmax网络--- 81.60%        -----------    84.93%
------Lstm网络-------  81.86%       -----------    87.47%
------CNN 网络------  73.73%       -----------    86.8%
----Ensemble vote-------    86%          ------    89.40%
       

文件说明：
---核心文件：
------------PR_project.m  --  主文件,运行train.m与test.m，数据集更换
------------train.m            --   集成七种算法主文件，可选择性执行,存储生成Model文件
                                         --   输入文件位置（'有标签data','标签','无标签数据'）
                                         --   数据格式 500*784，500*1，1500*784，mat文件
------------test.m              --   算法测试文件，
                                         --   输入文件位置（'测试数据'）,格式
                                         --   数据格式1500*784，mat文件
---------------------------------------------------------------------------------------------
---文件夹
------------ data文件夹         --数据存储 
------------ model文件夹      --训练模型存储，存储格式mat
                                             --命名规则，算法名_mdl.mat  ,算法名_PS.mat 
------------minFunc              --函数最小代价求解工具包，用于softmax求解
---------------------------------------------------------------------------------------------
---其它函数文件
------------KNN_train.m                --KNN训练文件，半监督KNN与常规KNN共用
------------my_classknn.m             --KNN类，魔改matlab封装后的KNN类以构建半监督算法
------------
------------convolutionalUnit.m    --CNN残差网络设定
------------
------------softmaxPredict.m         -- 普通softmax预测函数
------------softmaxCost.m              --普通softmax代价函数
------------softmaxTrain.m             --普通softmax训练函数
------------
------------nnObjFunction.m          --softmax网络搭建
------------softnet_fmincg.m          --softmax网络求解工具包
------------softnet_Predict.m          --softmax网络预测函数
------------softnetinit.m                  --softmax网络权重初始化
------------
------------SVMcg.m                        --SVM有优化的遍历调参，基于Libsvm工具包





