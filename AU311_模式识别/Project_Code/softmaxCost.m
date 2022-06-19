function [cost, grad] = softmaxCost(theta, lambda, data, labels)

theta = reshape(theta, 10, 784);
numCases = size(data, 2);%样本数量 M
groundTruth = full(sparse(labels, 1:numCases, 1));
M = bsxfun(@minus,theta*data,repmat(max(theta*data, [], 1),10,1)); 
M = exp(M);
p = bsxfun(@rdivide, M, sum(M));
cost = -1/numCases * groundTruth(:)' * log(p(:)) + lambda/2 * sum(theta(:) .^ 2);   

thetagrad = -1/numCases * (groundTruth - p) * data' + lambda * theta;
grad = [thetagrad(:)];
end

