
% @author  C_zihao, DJ Chen ,Shu Tao
% @Address  chaizihao@sjtu.edu.cn
% @Group    1
% @Data     2020.12.21
%
% Function  main()
% �������ݼ��ļ�˵����鿴README.txt
%

clear;clc;

% load ����ǩ
load('data/label2.mat')

% train('�б�ǩdata','��ǩ','�ޱ�ǩ����') ,ע�ͺ�Ҳ��ֱ������test() 
train('data/labeled_data.mat','data/label.mat','data/unlabeled_data.mat');

% ������ = test('��������')  
final_output=test('data/unlabeled_data.mat');

%����׼ȷ��
acc=length(find(final_output == unnamed))/length(final_output)*100;
fprintf('!-------Finished ,Accuracy=%f--------!\n',acc);