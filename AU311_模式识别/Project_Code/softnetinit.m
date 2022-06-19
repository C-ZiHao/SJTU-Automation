function W = softnetinit(n_in, n_out)
% initializeWeights return the random weights for Neural Network given the
% number of node in the input layer and output layer
epsilon = sqrt(6) / sqrt(n_in + n_out + 1);
W = rand(n_out, n_in + 1) * 2* epsilon - epsilon;

end
