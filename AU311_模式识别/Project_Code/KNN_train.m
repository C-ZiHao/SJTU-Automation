function this = KNN_train(X,Y,varargin)

    internal.stats.checkNotTall(upper(mfilename),0,X,Y,varargin{:});
    Y = convertStringsToChars(Y);
    [varargin{:}] = convertStringsToChars(varargin{:});
    [~, RemainingArgs] = classreg.learning.paramoptim.parseOptimizationArgs(varargin);
    % ʹ�� my_classknnѵ��knn model
    this = my_classknn.fit(X,Y,RemainingArgs{:});
end
