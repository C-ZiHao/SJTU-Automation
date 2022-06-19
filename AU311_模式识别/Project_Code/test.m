
% @author  C_zihao, DJ Chen ,Shu Tao
% @Address  chaizihao@sjtu.edu.cn
% @Group    1
% @Data     2020.12.21
%
% Function  test()
% Input : File location of '����data'
%Output: ����label
%
% �������ݼ��ļ�˵����鿴README.txt
%

function final_output= test(test_file)
    
    load(test_file);
    %load(model_file);
    
    %ʹ�÷���ѡ�� ��ʹ�� 0 ��1
    %�ɹ�ѡ�񣺰�ලknn����ͨknn��softmax��softmax����
    %                 LSTM���磬SVM��CNN����
    %   test�������нϿ죬���ض��ٶ���Ӱ��
    %   ��������ѡ�����vote���㷨�����㵥������
                    % sknn  knn  softmax  softnet  lstm  svm    cnn
    method=[      1          1       1           1           1        1      1];
    
    fprintf('-----------------------------------------------------------------\n');
    fprintf('------------------Using Method  :------------------------------\n');
    fprintf('---------sknn    knn    softmax    softnet    lstm     svm       cnn --------\n');
    fprintf('---------%d           %d              %d             %d           %d          %d          %d--------\n',method(1),method(2),method(3),method(4),method(5),method(6),method(7));
    fprintf('-----------------------------------------------------------------\n');
    sknn = method(1);
    normal_knn = method(2);
    softmax  = method(3);
    softnet = method(4);
    lstmnet=method(5);
    svm = method(6);
    cnn = method(7);
    
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    %LSTM��Ԥ��
    if(lstmnet)
        fprintf('---------------LSTM   Test start ,Please waiting--------\n');
        load('model/lstm_mdl.mat');
        load('model/LSTM_PS.mat','LSTM_PS');
        lstm_test_data = mapminmax('apply',untrain_x',LSTM_PS,0,1);
        lstm_test_data=mat2cell(lstm_test_data,[784],ones(1,length(lstm_test_data)));
        lstm_Pred = classify(lstm_mdl,lstm_test_data);
        for i =1:length(lstm_Pred)
            lstm_output(i,1)=double(lstm_Pred(i))-1;
        end
    end    
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    %��ලKNN��Ԥ��
    if(sknn)
        fprintf('---------------Semi-supervised KNN  Test start ,Please waiting--------\n');
         load('model/SKNN_mdl.mat');
         load('model/SkNN_PS.mat');
         
         sknn_test_data=untrain_x;
         sknn_test_data = mapminmax('apply',sknn_test_data',SKNN_PS,0,1)';
         [sknn_output,~,~,~]   = my_classknn.predict(sknn_mdl, sknn_test_data);
         load('model/sknn0_mdl.mat');
         [~,~,~,distance]   = my_classknn.predict(sknn0_mdl, sknn_test_data);
    end
    
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    %SVMѵ��
    if(svm)
        fprintf('---------------SVM Test start ,Please waiting--------\n');
        
        %ģ�ͼ���
        load('model/SVM_mdl.mat');
        load('model/SVM_PS.mat');
        
        %���ݶ�ȡ���һ��
        svm_test_data=untrain_x;
        svm_test_data = (mapminmax('apply',svm_test_data',SVM_PS,-1,1))';
        
        %ʹ��Libsvm���߰�����ȥ
%     test_label=zeros(length(svm_test_data),1);
%      svm_output = svmpredict(test_label,svm_test_data,svm_mdl);
        
        %������
        svm_output = predict(svm_mdl,svm_test_data);      
    end
    
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    %Softmax���
    if(softmax)
       fprintf('---------------Softmax   Test start ,Please waiting--------\n');
       
       %ģ�ͼ���
       load('model/softmax_mdl.mat');
       load('model/softmax_PS.mat');
       
       %���ݶ�ȡ���һ��
       softmax_test_data=untrain_x;
       softmax_test_data = mapminmax('apply',softmax_test_data',softmax_PS,0,1);
       
       %���Ԥ��
       softmax_output = softmaxPredict(softmax_mdl, softmax_test_data)'-1;
    end
 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    %����KNN
    if(normal_knn)
         fprintf('---------------Normal  KNN  Test start ,Please waiting--------\n');
         %���ݶ�ȡ��ģ�ͼ���
         knn_test_data=untrain_x;
         load('model/normal_knn_mdl.mat')
         %ʹ��my_classknnԤ�����
         knn_output   =    my_classknn.predict(normal_knn_mdl, knn_test_data);
    end
    
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    %���������softmax
    if(softnet)
        fprintf('---------------Softmax  Network  Test start ,Please waiting--------\n');
        %ģ�ͼ��أ����ݶ�ȡ
        load('model/softnet_mdl.mat')
        softnet_test_data=untrain_x;
        %ʹ��softnet_PredictԤ�����
        softnet_output = softnet_Predict(w1, w2, softnet_test_data);
    end
    
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %���вв������CNN
    if(cnn)
        fprintf('---------------CNN  Test start ,Please waiting--------\n');
        cnn_testdata=untrain_x;
        load('model/cnn_mdl.mat')
        for i=1:length(cnn_testdata)
            cnn_test_data(:,:,1,i)=reshape(cnn_testdata(i,:),28,28,1);
        end
        CNNPredicted = classify(cnn_mdl,cnn_test_data);
        for i =1:length(CNNPredicted)
             cnn_output(i,1)=double(CNNPredicted(i))-1;
        end
    end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    %���յ�Vote
    fprintf('---------------VOTE    Begin------------------\n');
    %��ʼ��ͶƱ��
    vote_mat = zeros(length(untrain_x),10);
    final_output  =zeros(length(untrain_x),1);
    
    %���������㷨ͶƱȨ�أ����а�ල��Ȩ�ظ������Ŷȸı�
    svm_ratio = 1.2;
    softmax_ratio=0.7;
    softnet_ratio = 0.7;
    sknn_ratio = 1.8;
    knn_ratio = 1.5;
    lstm_ratio=0.7;
    cnn_ratio =1.5;
    
    for i=1:length(untrain_x)
        %�������Ŷȵ���Ȩ��
        if(method(1))
            if(distance(i,1)<0.4)
                  sknn_ratio = 4;
            elseif(distance(i,1)<0.5)
                  sknn_ratio = 2.8;
            elseif(distance(i,1)<0.6)
                  sknn_ratio = 1.8;
            elseif(distance(i,1)<0.7)
                  sknn_ratio = 1;
            else
                  sknn_ratio = 0;
            end
        end
        
        %����ͶƱ�������漰�����㷨�Ŀ��أ�if����һЩ
        if(method(1))
            vote_mat(i,sknn_output(i,1)+1) = vote_mat(i,sknn_output(i,1)+1)+1*sknn_ratio;
        end
        if(method(2))
            vote_mat(i,knn_output(i,1)+1) = vote_mat(i,knn_output(i,1)+1)+1*knn_ratio;
        end
        if(method(3))
            vote_mat(i,softmax_output(i,1)+1) = vote_mat(i,softmax_output(i,1)+1)+1 * softmax_ratio;
        end
        if(method(4))
            vote_mat(i,softnet_output(i,1)+1) = vote_mat(i,softnet_output(i,1)+1)+1* softnet_ratio;
        end
        if(method(5))
            vote_mat(i,lstm_output(i,1)+1) =vote_mat(i,lstm_output(i,1)+1)+1*lstm_ratio;
        end
        if(method(6))
            vote_mat(i,svm_output(i,1)+1) = vote_mat(i,svm_output(i,1)+1)+1*svm_ratio;
        end
        if(method(7))
            vote_mat(i,cnn_output(i,1)+1) = vote_mat(i,cnn_output(i,1)+1)+1*cnn_ratio;
        end
        %�鿴����ͶƱ���
        [~,election]=max(vote_mat(i,:));
        final_output(i,1)=election-1;
        
    end
    fprintf('---------------Test    Finished------------------\n');
    %���
    final_output = final_output;
    
end

