import csv
import sys
import os
from typing import get_type_hints
import matplotlib.pyplot as plt
import numpy as np

test_cost = 126762

test_times = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
offline_time = [[0]*3 for i in range(5)]
gas_cost_ours = np.zeros(10)
gas_cost_bpreet = np.zeros(10)
for i in range(10):
    gas_cost_ours[i] = 1*test_cost
    gas_cost_bpreet[i] = test_times[i]*test_cost

plt.title('Gas cost of Test algorithm')
plt.xlabel('Number of executions needs of each request')
plt.ylabel('Gas cost')
plt.plot(test_times, gas_cost_ours, color='green', marker='o', markersize=3)
plt.plot(test_times, gas_cost_bpreet, color='blue', linestyle=':', marker='o', markersize=3)

plt.show()