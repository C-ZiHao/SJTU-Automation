function [pred] = softmaxPredict(softmaxModel, data)

theta = softmaxModel.optTheta;  % this provides a numClasses x inputSize matrix
pred = zeros(1, size(data, 2));
[nop, pred] = max(theta * data);
size(max(theta * data));

end

