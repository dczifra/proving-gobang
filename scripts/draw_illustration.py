import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle

def draw_lines(n,m):
    fig, ax = plt.subplots(figsize=(10,5))
    extra_lines = [((0,1),(1,0)),((m-2,0),(m-1,1)),
             ((0,2),(1,1),(2,0)),((m-3,0),(m-2,1),(m-1,2)),
             ((0,1),(1,2),(2,3)),((m-1,1),(m-2,2),(m-3,3))
    ]

    diagonal_lines = [
        ((i,0),(i+1,1),(i+2,2),(i+3,3)) for i in range(m-3)
    ] + [
        ((i,0),(i-1,1),(i-2,2),(i-3,3)) for i in reversed(range(3,m))
    ]

    side_lines = [
        ((0,i),(1,i),(2,i),(3,i)) for i in range(4)
    ] + [
        ((m-1,i),(m-2,i),(m-3,i),(m-4,i)) for i in range(4)
    ]
    for line in extra_lines:
        ax.plot([c[0]+0.5 for c in line],[n-c[1]-0.5 for c in line],color="r", alpha=0.7, linewidth=linewidth)
    
    for line in diagonal_lines:
        ax.plot([c[0]+0.5 for c in line],[n-c[1]-0.5 for c in line],color="g", alpha=0.7, linewidth=linewidth)
    
    for line in side_lines:
        ax.plot([c[0]+0.5 for c in line],[n-c[1]-0.5 for c in line],color="b", alpha=0.7, linewidth=linewidth)    

    for i in range(n):
        for j in range(m):
            ax.add_patch(Rectangle((j,n-1-i), 1, 1,color="black",
                edgecolor="black", linewidth=checkered_width, fill=False))

    ax.set_xlim([0-0.01,m+0.01])
    ax.set_ylim([0-0.01,n+0.01])
    
    plt.subplots_adjust(left=0.0, right=1.0, bottom=0.0, top=1.0)
    plt.axis("off")
    plt.show()

def draw_x(ax, x,y):
    eps = 0.6
    ax.plot([x-eps*0.5, x+eps*0.5], [y-eps*0.5, y+eps*0.5], color='b', linewidth=linewidth)
    ax.plot([x-eps*0.5, x+eps*0.5], [y+eps*0.5, y-eps*0.5], color='b', linewidth=linewidth)

def draw_o(ax,x,y):
    ax.add_patch(plt.Circle((x,y), 0.3, color='r', linewidth=linewidth, fill=False))
    
def draw_disproof(n,m):
    fig, ax = plt.subplots(figsize=(10,5))


    for i in range(n):
        for j in range(m):
            r = ax.add_patch(Rectangle((j,n-1-i), 1, 1,color="black",
                    edgecolor="black", linewidth=checkered_width, fill=False))
    
    draw_o(ax,0+0.5, n-1+0.5)
    draw_o(ax,0+0.5, 0+0.5)
    draw_x(ax, 0+0.5, n-2+0.5)
    
    
    ax.set_xlim([0-0.01,m+0.01])
    ax.set_ylim([0-0.01,n+0.01])
    plt.axis("off")
    plt.subplots_adjust(left=0.0, right=1.0, bottom=0.0, top=1.0)

    plt.show()

# === Params ===
linewidth = 4.0
checkered_width = 1.5

draw_lines(4,8)
draw_disproof(4,8)