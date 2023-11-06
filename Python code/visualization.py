def visMatrix(mat):
    plt.matshow(mat)
    plt.colorbar()
    plt.show()


def rankPlot(lens, ranks):
    plt.scatter(lens, ranks)
    for (i, j) in zip(lens, ranks):
        plt.text(i, j, f'({i}, {j})')
    plt.show()


import matplotlib.pyplot as plt
import numpy as np


def plot3d(z, l):
    # set up the figure and axes
    fig = plt.figure()
    ax1 = fig.add_subplot(121, projection='3d')

    _x = np.arange(l)
    _y = np.arange(l)
    _xx, _yy = np.meshgrid(_x, _y)
    x, y = _xx.ravel(), _yy.ravel()
    print(x)
    print(y)
    print(x + y)
    top = z
    bottom = np.zeros_like(top)
    width = depth = 1

    colors = plt.cm.jet(z.flatten() / float(z.max()))
    ax1.bar3d(x, y, bottom, width, depth, top, shade=True, color=colors)
    ax1.set_title('Shaded')

    plt.show()
