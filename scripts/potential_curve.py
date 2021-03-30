import matplotlib.pyplot as plt
import numpy as np

potentials = [2.51562, 2.70312, 2.19531, 2.38281, 1.875, 2.125, 1.6875, 1.875, 1.25, 1.4375, 1, 0.8125]

plt.plot(range(len(potentials)), potentials)
plt.ylabel('potential')
plt.xlabel('step')
# plt.legend()
plt.savefig("potential.png")
