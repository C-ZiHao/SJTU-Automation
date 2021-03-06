
% @author  C_zihao, DJ Chen ,Shu Tao
% @Address  chaizihao@sjtu.edu.cn
% @Group    1
% @Data     2020.12.21
%
% Function  train()
% Input : File location of '有标签data','标签','无标签数据'
%Output: success
%
% 更多内容及文件说明请查看README.txt
%

function finish_flag= train(labeled_file,label_file,unlabeled_file)
   
    %加载 label, data ,etc
    load(labeled_file);
    load(label_file);
    load(unlabeled_file);
    
    %读取原始数据
    origin_train_data=train_x;
    origin_train_label=train_label;
    origin_untrain_data=untrain_x;
     
    %使用方法选择 ，使用 0 或1
    %可供选择：半监督knn，普通knn，softmax，softmax网络
    %                 LSTM网络，SVM，CNN网络
    %注： LSTM网络与CNN训练时间稍长，其余较快，简单跑建议关闭
    %        不建议关闭sknn，关闭后即半监督关闭。
     %  ---------sknn  knn  softmax  softnet  lstm  svm     cnn
    method=[      1          1       1           1           0         1      0];
    fprintf('------------------------------------------------------------------------\n');
    fprintf('------------------Using Method  :--------------------------------------\n');
    fprintf('---------sknn    knn    softmax    softnet    lstm     svm      cnn----------------\n');
    fprintf('---------%d           %d              %d             %d           %d          %d           %d----------------\n',method(1),method(2),method(3),method(4),method(5),method(6),method(7));
    fprintf('-------------------------------------------------------------------------\n');
    
    supervised_knn = method(1);
    normal_knn = method(2);
    softmax  = method(3);
    softnet = method(4);
    lstmnet=method(5);
    svm = method(6);
    cnn = method(7);
    
    %  PCA降维，之后发现没有用处，未使用
    %  [pc,score,latent,tsquare] = pca(origin_train_data);
    % p_d_ratio=cumsum(latent)./sum(latent);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % 半监督KNN，首先执行，为后续提供置信度较高的伪标签
    if(supervised_knn)
        fprintf('-------------Semi-supervised   knn traing  start ,please waiting--------\n'); 
        %数据读取
        sknn_train_data = origin_train_data;
        sknn_train_label = origin_train_label;
        sknn_test_data  = origin_untrain_data;

        %半监督训练控制，以置信度为依据划分，依次增长
        sknn_index=[0.18:0.01:0.4];
        for k=1:length(sknn_index)
            %归一，并存储归一模型
           [sknn_train_data,SKNN_PS] = mapminmax(sknn_train_data',0,1);
           sknn_train_data  =sknn_train_data';
           sknn_test_data = mapminmax('apply',sknn_test_data',SKNN_PS,0,1)';
            %调用KNN训练函数KNN_train
            sknn_mdl =KNN_train(sknn_train_data,sknn_train_label,'NumNeighbors',1, 'NSMethod','exhaustive','Distance','spearman',...
             'Standardize',1); 
            %打伪标签
            [sknn_output,~,~,distance]   = my_classknn.predict(sknn_mdl, sknn_test_data);
            add_data = zeros(1,784);
            add_label = zeros(1,1);
            add_num=0;
            %遍历提取可信标签
            for i=1:length(sknn_output)
                %当置信小于本次循环设定阈值，提取data与label放入训练数据
              if(distance(i,1)<sknn_index(k))
                  add_num=add_num+1;
                  add_data(add_num,:)=origin_untrain_data(i,:);
                  add_label(add_num,1)=sknn_output(i,1);
              end
            end
            %生成新的标签并进行下一步训练
           sknn_train_data =[origin_train_data;add_data];
           sknn_train_label=[origin_train_label;add_label];
           sknn_test_data=origin_untrain_data;
           if(mod(k,5)==0)
               fprintf('---sknn  epochs :%d    ||         index:  %2f,          add  %d\n',k,sknn_index(k),add_num);
           end
            if(k==1)
                sknn0_mdl = sknn_mdl;
                save('model/sknn0_mdl.mat','sknn0_mdl');
            end 
        end
        %存储参数mdl与归一mdl
        save('model/SKNN_PS.mat','SKNN_PS');
        save('model/sknn_mdl.mat','sknn_mdl');
        
         %使用训练数据进行测试
         train_acc_test =my_classknn.predict(sknn_mdl, sknn_train_data);
         train_accuracy =  length(find(train_acc_test == sknn_train_label))/length(sknn_train_label)*100;
         fprintf('----------Test  Using  train data ,Accuracy=%f---------\n',train_accuracy);
        
        fprintf('-------------Semi-supervised   knn traing  finished ,add data：%d--------\n',add_num); 
        
         % 之后训练的数据为KNN生成的
         origin_train_data=sknn_train_data;
         origin_train_label=sknn_train_label;
    end
    
    

    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    %SVM训练
    if(svm)
       fprintf('---------------SVM  training start ,please waiting--------\n');
       
       %数据读取与归一化
       svm_train_data = origin_train_data;
       svm_train_label = origin_train_label;
       [svm_train_data,SVM_PS] = mapminmax(svm_train_data',-1,1);
       svm_train_data=svm_train_data';
       save('model/SVM_PS.mat','SVM_PS');
        
       %使用Libsvm工具箱，由于这一工具箱需额外安装，最后舍去 
       % SVMcg可用于优化的遍历调参
%    [bestacc,bestc,bestg] = SVMcg(trainlabel,train_data,-8,8,-8,8,10,0.5,0.5,0.9);
%    cmd = ['-c',num2str(bestc),'-g',num2str(bestg),'-w1 2 -w0 0.5']
%    cmd=  ['-c 64 -g 0.0039063 -w1 1 -w9 1'];
%    svm_mdl =svmtrain(svm_train_label,svm_train_data,cmd);
%    train_acc_test = svmpredict(svm_train_label,svm_train_data,svm_mdl);
        
        %模型训练
        svm_mdl =fitcecoc(svm_train_data,svm_train_label);
        train_acc_test  = predict(svm_mdl,svm_train_data);
        
        save('model/svm_mdl.mat','svm_mdl');
        train_accuracy =  length(find(train_acc_test == origin_train_label))/length(origin_train_label)*100;

        fprintf('---------------SVM Trainng  finished ,model saved---------\n');
        fprintf('----------Test  Using  train data ,Accuracy=%f---------\n',train_accuracy);
    end
       
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    %Softmax求解
    if(softmax)
       fprintf('---------------Softmax  start ,please waiting--------\n');
       
       %数据读取与归一化
       softmax_train_data = origin_train_data;
       softmax_train_label = origin_train_label+1;
       [softmax_train_data,softmax_PS] = mapminmax(softmax_train_data',0,1);
       save('model/softmax_PS.mat','softmax_PS');
       
       %求解参数设定
       lambda=1e-5;
       options.maxIter = 1000;
       
       %使用softmaxTrain训练
       softmax_mdl = softmaxTrain( lambda,...
                            softmax_train_data, softmax_train_label, options);
       %模型存储
       save('model/softmax_mdl.mat','softmax_mdl')
        %使用训练数据集测试
       train_acc_test = softmaxPredict(softmax_mdl, softmax_train_data)'-1;
       train_accuracy =  length(find(train_acc_test == origin_train_label))/length(origin_train_label)*100;

       fprintf('---------------Softmax  finished ,model saved---------\n');
       fprintf('----------Test  Using  train data ,Accuracy=%f---------\n',train_accuracy);
    end
    
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
   %常规KNN算法
    if(normal_knn)
         fprintf('---------------Normal  KNN  start ,please waiting--------\n');
         
         %训练数据读取
         knn_train_data=origin_train_data;
         knn_train_label = origin_train_label;
         
         %使用KNN_train进行训练
         normal_knn_mdl =KNN_train(knn_train_data,knn_train_label,'NumNeighbors',1, 'NSMethod','exhaustive','Distance','spearman',...
        'Standardize',1);

        %  用于优化调参，运行时间较长
        %  mdl =fitcknn(train_data,trainlabel,'OptimizeHyperparameters','auto',...
        %      'HyperparameterOptimizationOptions',...
         %      struct('AcquisitionFunctionName','expected-improvement-plus'))
         
         %使用训练进行检测
         train_acc_test =my_classknn.predict(normal_knn_mdl, knn_train_data);
         train_accuracy =  length(find(train_acc_test == origin_train_label))/length(origin_train_label)*100;

         %存储模型
         save('model/normal_knn_mdl.mat','normal_knn_mdl')
         fprintf('---------------Normal  KNN  finished ,model saved---------\n');
         fprintf('----------Test  Using  train data ,Accuracy=%f---------\n',train_accuracy);
    end
    
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %LSTM网络
    if(lstmnet)
        fprintf('------------------Lstm-network  Begining----------------------\n');
        
        %训练数据输入
        lstm_train_data = origin_train_data';
        lstm_train_label = origin_train_label;
         %数据归一化
         [Train_matrix,LSTM_PS] = mapminmax(lstm_train_data,0,1);
         lstm_train_data = Train_matrix;
        
        %训练参数设定
        inputSize = 784;
        numHiddenUnits = 150;
        numClasses = 10;
        miniBatchSize=70;
        options = trainingOptions('adam', ...
            'MiniBatchSize',miniBatchSize, ...
            'MaxEpochs',1500, ...
            'InitialLearnRate',0.002, ...
            'LearnRateSchedule','piecewise', ...
            'LearnRateDropFactor',0.4, ...
            'LearnRateDropPeriod',200, ...
            'Shuffle','every-epoch', ...
            'Verbose',false, ...
            'Plots','training-progress');
        %'ValidationData',imdsValidation, ... 'ValidationFrequency',30, ...
        
        %基本网络架构
        layers = [ ...
            sequenceInputLayer(inputSize)
            lstmLayer(numHiddenUnits,'OutputMode','last')
            fullyConnectedLayer(numClasses)
            softmaxLayer
            classificationLayer]
        
        %进行训练
        lstm_train_data=mat2cell(lstm_train_data,[784],ones(1,length(lstm_train_label)));
        lstm_mdl = trainNetwork(lstm_train_data,categorical(lstm_train_label),layers,options);

        %存储模型
        save('model/lstm_mdl.mat','lstm_mdl');
        save('model/LSTM_PS.mat','LSTM_PS');
        
        %使用训练数据测试
        lstm_Pred = classify(lstm_mdl,lstm_train_data);
        for i =1:length(lstm_Pred)
            train_acc_test(i)=double(lstm_Pred(i))-1;
        end
        train_accuracy =  length(find(train_acc_test == origin_train_label))/length(origin_train_label)*100;
       
        fprintf('---------------Lstm-network  finished ,model saved---------\n');
        fprintf('----------Test  Using  train data ,Accuracy=%f---------\n',train_accuracy);
        
    end
    
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    %softmax使用网络构造
    if(softnet)
        fprintf('------------------Softmax-network  Begining----------------------\n');
        
        %训练数据输入
        softnet_train_data = origin_train_data;
        softnet_train_label = origin_train_label;

         n_input = size(softnet_train_data, 2);
         n_hidden = 50;
         n_class = 10;
        %继续/重新训练
        holdon=true;
        if holdon==true
           init_w1 = softnetinit(n_input, n_hidden);
           init_w2 = softnetinit(n_hidden, n_class);
        else
           load('model/softnet_mdl.mat');
           init_w1 =w1;
           init_w2 = w2;
        end
        
        %训练参数设定
        options = optimset('MaxIter', 1000);
        initWeights = [init_w1(:); init_w2(:)];
        lambda = 0.4;
        objFunction = @(params) nnObjFunction(params, n_input, n_hidden, ...
                               n_class, softnet_train_data, softnet_train_label, lambda);
                           
        %使用fmincg工具求解
        [nn_params, cost] = softnet_fmincg(objFunction, initWeights, options);
        w1 = reshape(nn_params(1:n_hidden * (n_input + 1)), ...
                         n_hidden, (n_input + 1));
        w2 = reshape(nn_params((1 + (n_hidden * (n_input + 1))):end), ...
                         n_class, (n_hidden + 1));
                     
        % 使用训练数据测试
        train_acc_test = softnet_Predict(w1, w2, softnet_train_data);
        train_accuracy =  length(find(train_acc_test == origin_train_label))/length(origin_train_label)*100;
        fprintf('----------Test  Using  train data ,Accuracy=%f---------\n',train_accuracy);        
        
        %存储模型
        save('model/softnet_mdl.mat', 'n_input', 'n_hidden', 'w1', 'w2', 'lambda');
        fprintf('---------------softmax-network  finished ,model saved---------\n');
    end
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %含有残差网络的CNN
    if(cnn)
         fprintf('------------------CNN-Network  Begining----------------------\n');
         
         %训练数据读取
         cnn_traindata = origin_train_data;
         cnn_train_label = origin_train_label;
         for i=1:length(cnn_train_label)
            cnn_train_data(:, :, 1,i)=reshape(cnn_traindata(i,:),28,28,1);
         end
        cnn_train_label = categorical(cnn_train_label);
        
        %训练参数设定
        netWidth = 16;
        layers = [
                imageInputLayer([28 28 1],'Name','input')
                convolution2dLayer(3,netWidth,'Padding','same','Name','convInp')
                batchNormalizationLayer('Name','BNInp')
                softmaxLayer('Name','reluInp')
                convolutionalUnit(2*netWidth,1,'S2U1')
                additionLayer(2,'Name','add21')
                softmaxLayer('Name','relu21')
                convolutionalUnit(4*netWidth,1,'S2U2')
                additionLayer(2,'Name','add22')
                softmaxLayer('Name','relu22')
                averagePooling2dLayer(2,'Name','globalPool')
                fullyConnectedLayer(10,'Name','fcFinal')
                softmaxLayer('Name','softmax')
                classificationLayer('Name','classoutput')
        ];


        lgraph = layerGraph(layers);
        skip1 = [
            convolution2dLayer(1,2*netWidth,'Stride',1,'Name','skipConv1')
            batchNormalizationLayer('Name','skipBN1')];
        lgraph = addLayers(lgraph,skip1);
        lgraph = connectLayers(lgraph,'reluInp','skipConv1');
        lgraph = connectLayers(lgraph,'skipBN1','add21/in2');

        skip2 = [
            convolution2dLayer(1,4*netWidth,'Stride',1,'Name','skipConv2')
            batchNormalizationLayer('Name','skipBN2')];
        lgraph = addLayers(lgraph,skip2);
        lgraph = connectLayers(lgraph,'relu21','skipConv2');
        lgraph = connectLayers(lgraph,'skipBN2','add22/in2');
        plot(lgraph)

        miniBatchSize = 128;
        learnRate = 0.1*miniBatchSize/128;
        % valFrequency = floor(size(XTrain,4)/miniBatchSize);
        options = trainingOptions('adam', ...
            'GradientDecayFactor',0.8,...
            'SquaredGradientDecayFactor',0.9, ...
            'InitialLearnRate',0.005, ...
            'MaxEpochs',50, ...
            'MiniBatchSize',miniBatchSize, ...
            'Shuffle','every-epoch', ...
            'Plots','training-progress', ...
            'Verbose',false, ...
            'LearnRateSchedule','piecewise', ...
            'LearnRateDropFactor',0.1, ...
            'LearnRateDropPeriod',30);
         cnn_mdl = trainNetwork(cnn_train_data,cnn_train_label,lgraph,options);
         
         % 使用训练数据进行测试
         CNNPredicted = classify(cnn_mdl,cnn_train_data);
         for i =1:length(CNNPredicted)
             train_acc_test(i,1)=double(CNNPredicted(i))-1;
         end
         train_accuracy =  length(find(train_acc_test == origin_train_label))/length(origin_train_label)*100;
        
         %存储模型
         save('model/cnn_mdl.mat', 'cnn_mdl');
         fprintf('---------------CNN-Network  finished ,model saved---------\n');
         fprintf('----------Test  Using  train data ,Accuracy=%f---------\n',train_accuracy);        

    end
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    %训练完成
    finish_flag = true;
    fprintf('---------------Training    Finished------------------\n');
end

