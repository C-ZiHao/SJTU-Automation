function [softmaxModel] = softmaxTrain(lambda, inputData, labels, options)

inputSize =784 ;
lambda=1e-5;
theta = 0.005 * randn(7840, 1);

% Use minFunc to minimize the function
addpath minFunc/
options.Method = 'lbfgs'; 
[softmaxOptTheta, cost] = minFunc( @(p) softmaxCost(p, ...
                                   lambda,  ...
                                   inputData, labels), ...                                   
                                   theta, options);

softmaxModel.optTheta = reshape(softmaxOptTheta, 10, inputSize);
 end                          
