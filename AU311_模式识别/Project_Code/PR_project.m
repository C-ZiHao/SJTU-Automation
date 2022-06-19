
% @author  C_zihao, DJ Chen ,Shu Tao
% @Address  chaizihao@sjtu.edu.cn
% @Group    1
% @Data     2020.12.21
%
% Function  main()
% 更多内容及文件说明请查看README.txt
%

clear;clc;

% load 检测标签
load('data/label2.mat')

% train('有标签data','标签','无标签数据') ,注释后也可直接运行test() 
train('data/labeled_data.mat','data/label.mat','data/unlabeled_data.mat');

% 输出结果 = test('测试数据')  
final_output=test('data/unlabeled_data.mat');

%计算准确率
acc=length(find(final_output == unnamed))/length(final_output)*100;
fprintf('!-------Finished ,Accuracy=%f--------!\n',acc);