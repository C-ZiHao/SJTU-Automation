function label = softnet_Predict(w1, w2, data)
% nnPredict predicts the label of data given the parameter w1, w2 of Neural
% Network.

% Normalization
data = double(data)/255;
N = size(data, 1);                              
x = ones(N, size(w1, 2));                       
x(1:size(data,1), 1:size(data,2)) = data;
aj = x * transpose(w1);                         
zj = 1 ./ (1 + exp(-aj));
zj = horzcat(zj, ones(N,1));                    
bk = zj * transpose(w2);
yk = 1 ./ (1 + exp(-bk));

label = zeros(size(yk,1),1);

% Store the row wise maximum value of yk in the label
for i = 1 : size(yk,1)
    label (i,1) = find(yk(i,:) == max(yk(i,:)), 1 ) - 1;
end
