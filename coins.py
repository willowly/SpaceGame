import random
import matplotlib.pyplot as plt
import numpy as np
import collections

results = []
distribution = {}

for i in range(1000):
    heads = 0
    flips = 0
    while heads < 10:
        if(random.random() < 0.25):
            heads += 1
        else:
            heads = 0
        flips += 1

    results.append(flips)

    average = 0
    for result in results:
        average += result
    average = average / len(results)

    print("got 10 heads in " + str(flips) + " flips \t avg:" + str(average))
    if int(flips/100000) in distribution:
        distribution[int(flips/100000)] += 1
    else:
        distribution[int(flips/100000)] = 1


fig, ax = plt.subplots(figsize=(5, 2.7), layout='constrained')

ordered_distribution = collections.OrderedDict(sorted(distribution.items()))

for key in ordered_distribution.keys():
    print(str(key) + ": " + str(ordered_distribution[key]))

ax.plot(ordered_distribution.keys(),ordered_distribution.values())
plt.show()





    