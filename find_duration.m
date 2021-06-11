clc;
filename = 'test09-1.csv'

T = readtable(filename)

% T(:,3) also work
n = unique(T.Var3)

list = zeros
for i = 1:height(n)
   num = sum(T.Var3 == n(i))
   if num >= 100
       list(end+1) = seconds(n(i))
   end
end

% count from 1
MIN = min(list(2:end))
MAX = max(list(2:end))