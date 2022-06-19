
clearvars;
tic;


load('label.mat');
load('labeled_data.mat');
load('label2.mat');
load('unlabeled_data.mat');


train_data=[train_x];
trainlabel=[train_label];
test_data=untrain_x;
test_label=unnamed;


% set the number of nodes in input unit (not including bias unit)
n_input = size(train_data, 2);
% set the number of nodes in hidden unit (not including bias unit)
n_hidden = 50;
% set the number of nodes in output unit
n_class = 10;
holdon=true;

if holdon==true
% initialize the weights into some random matrices
  init_w1 = softnetinit(n_input, n_hidden);
  init_w2 = softnetinit(n_hidden, n_class);
else
  load('params.mat')
  init_w1 =w1;
  init_w2 = w2;
end

% unroll 2 weight matrices into single column vector
initWeights = [init_w1(:); init_w2(:)];
% set the maximum number of iteration in conjugate gradient descent
options = optimset('MaxIter', 1000);
% unroll 2 weight matrices into single column vector
initWeights = [init_w1(:); init_w2(:)];
% set the regularization hyper-parameter
lambda = 0.4;
% define the objective function
objFunction = @(params) nnObjFunction(params, n_input, n_hidden, ...
                       n_class, train_data, trainlabel, lambda);

% run neural network training with fmincg
[nn_params, cost] = softnet_fmincg(objFunction, initWeights, options);

% reshape the nn_params from a column vector into 2 matrices w1 and w2
w1 = reshape(nn_params(1:n_hidden * (n_input + 1)), ...
                 n_hidden, (n_input + 1));

w2 = reshape(nn_params((1 + (n_hidden * (n_input + 1))):end), ...
                 n_class, (n_hidden + 1));


% Test Neural Network with test data
softnet_output = softnet_Predict(w1, w2, test_data);
fprintf('\nTesting Set Accuracy: %f\n', ...
         mean(double(softnet_output == test_label)) * 100);

     
fprintf('\nTotal Cost: %f\n', sum(cost));
save('params2.mat', 'n_input', 'n_hidden', 'w1', 'w2', 'lambda');
fprintf('\nTotal Time (sec) for Neural Network: %f\n', toc);
