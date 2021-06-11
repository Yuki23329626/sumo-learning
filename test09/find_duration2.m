clc;
filename = 'D:\github\sumo-learning\2021-06-06-21-51-17\2021-06-06-21-51-17.csv'

T = readtable(filename);

% T(:,3) also work
n = unique(T.Var3);

keys = zeros;
values = zeros;
for i = 1:height(n)
   num = sum(T.Var3 == n(i));
   if num >= 100
       keys(end+1) = seconds(n(i));
       values(end+1) = num;
   end
end

% count from 1
MIN = min(keys(2:end))
MAX = max(keys(2:end))

map = containers.Map(keys,values)

map(1200)

for i = 1:height(T)
    if seconds(T{i,3}) >= 240
        T(i,:)
    end
end

