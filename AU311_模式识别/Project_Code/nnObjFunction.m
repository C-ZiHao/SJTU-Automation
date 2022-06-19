function [obj_val, obj_grad] = nnObjFunction(params, n_input, n_hidden, ...
                                    n_class, training_data,...
                                    training_label, lambda)
% nnObjFunction computes the value of objective function (negative log 
%   likelihood error function with regularization) given the parameters 
%   of Neural Networks, thetraining data, their corresponding training 
%   labels and lambda - regularization hyper-parameter.

% Input:
% params: vector of weights of 2 matrices w1 (weights of connections from
%     input layer to hidden layer) and w2 (weights of connections from
%     hidden layer to output layer) where all of the weights are contained
%     in a single vector.
% n_input: number of node in input layer (not include the bias node)
% n_hidden: number of node in hidden layer (not include the bias node)
% n_class: number of node in output layer (number of classes in
%     classification problem
% training_data: matrix of training data. Each row of this matrix
%     represents the feature vector of a particular image
% training_label: the vector of truth label of training images. Each entry
%     in the vector represents the truth label of its corresponding image.
% lambda: regularization hyper-parameter. This value is used for fixing the
%     overfitting problem.
       
% Output: 
% obj_val: a scalar value representing value of error function
% obj_grad: a SINGLE vector of gradient value of error function
% NOTE: how to compute obj_grad
% Use backpropagation algorithm to compute the gradient of error function
% for each weights in weight matrices.
% Suppose the gradient of w1 is 

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% reshape 'params' vector into 2 matrices of weight w1 and w2
% w1: matrix of weights of connections from input layer to hidden layers.
%     w1(i, j) represents the weight of connection from unit j in input 
%     layer to unit i in hidden layer.
% w2: matrix of weights of connections from hidden layer to output layers.
%     w2(i, j) represents the weight of connection from unit j in hidden 
%     layer to unit i in output layer.
w1 = reshape(params(1:n_hidden * (n_input + 1)), ...
                 n_hidden, (n_input + 1));

w2 = reshape(params((1 + (n_hidden * (n_input + 1))):end), ...
                 n_class, (n_hidden + 1));

% Normalize the training_data
training_data = double(training_data)/255;

% Find the number of Rows in N
N = size(training_data, 1);                         % 50000
x = ones(N, size(w1, 2));                           % 50000 X 720
x(1:size(training_data,1), 1:size(training_data,2)) = training_data;
aj = x * transpose(w1);                         % 50000 X 50
% Apply sigmoid on each weighted input
zj = 1 ./ (1 + exp(-aj));

% Add a column for bias
zj = horzcat(zj, ones(N,1));                    % Adding one column of ones

bk = zj * transpose(w2);

% Apply sigmoid on each output hence generated
yk = 1 ./ (1 + exp(-bk));

% tk is the 1-to-k encoding of each training input
tk = zeros(size(training_data,1), 10);

for i = 1 : size(training_data, 1)
    tk(i, (training_label(i,1) + 1)) = 1;
end

EnW = - sum(tk .* log(yk) + (1 - tk) .* log(1 - yk), 2);

% EW is the Error function.
% EW is a scalar => obj_val
EW = sum(EnW) / N;

% deltaK is the difference of the actual output and the true output
deltaK = yk - tk;                                   % 50000 X 10

% delEW2 is the derivative of error function with respect to the weights
% from hidden unit j to output unit k
delEW2 = transpose(deltaK) * zj;                    % 10 X 51

part1 = ((1-zj) .* zj);         %50000 X 51
part2 = deltaK * w2;            %50000 X 51
part3 = x;
part1 = part1 .* part2;         %50000 X 51

% delEW1 is the derivative of Error function with respect to the weights
% from input unit i = 1, 2, ..., D + 1 & 
%      hidden units j = 1, 2, ..., M + 1
delEW1 = transpose(part1) * part3;                  % 51 X 720
% EWtilda is revised error function which is calculated considering the
% regularization term lambda. This is done to control the parameters of the
% Neural Network
EWtilda = EW + (lambda / (2 * N)) * (sum(sum(w1 .* w1)) + sum(sum(w2 .* w2)));


% With the new objective function, delEWtilda1 is the partial derivative
% with respect to weight from hidden layer to output layer
delEWtilda1 = (delEW1(1:(size(delEW1,1) - 1), :) + lambda * w1) ./ N;%previously N*delEW1

% With the new objective function, delEWtilda2 is the partial derivative
% with respect to weight from input layer to hidden layer
delEWtilda2 = (delEW2 + lambda * w2) ./ N;% previously N*delEW2 

obj_val = EWtilda;
grad_w1 = delEWtilda1;
grad_w2 = delEWtilda2;

% Suppose the gradient of w1 and w2 are stored in 2 matrices grad_w1 and grad_w2
% Unroll gradients to single column vector
obj_grad = [grad_w1(:) ; grad_w2(:)];
end
