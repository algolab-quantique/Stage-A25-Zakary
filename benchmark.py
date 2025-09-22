import time
import matplotlib.pyplot as plt
import numpy as np

import benchmark_dpa
import benchmark_np

# n = 250000000
n = 50000
dim = 1000
reps = 1
labels = ['Random Generation', 'Tensor', 'Commutes', 'Compose', 'Identity', 'New Identity', 'Both Idendtities', 'Swap ZX']
mode = "all"  # all, tensor, commutes, compose



def make_normalized_graph(numpy_times, cpp_times):    
    x = np.arange(len(labels)) 
    width = 0.35
    

    fig, ax = plt.subplots()
    # rects1 = ax.bar(x - width / 2, numpy_times, width, label='NumPy')
    # rects2 = ax.bar(x + width / 2, dense_cpp_times, width, label='C++ (DensePauliArray)')
    newpa = []
    newcpp = []
    for i in range(len(numpy_times)):
        if numpy_times[i] < cpp_times[i]:
            newpa.append(numpy_times[i]/cpp_times[i]*100)
            newcpp.append(100)
        else:
            newcpp.append(cpp_times[i]/numpy_times[i]*100)
            newpa.append(100)
    
    rects1 = ax.bar(x - width / 2, newpa, width, label='PauliArray (Python)')
    rects2 = ax.bar(x + width / 2, newcpp, width, label='DensePauliArray (C++)')

    for i in range(len(x)):
        ax.text(x[i] - width / 2, newpa[i] + 1, f'{newpa[i]:.1f}%', ha='center', fontsize=8)
        ax.text(x[i] + width / 2, newcpp[i] + 1, f'{newcpp[i]:.1f}%', ha='center', fontsize=8)
    
    ax.set_ylabel('Time (ms)')
    ax.set_title(f"PauliArray voids V.S C++ DensePauliArray (n={n}, dim={dim})\n(Lower is better)")
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    ax.legend()

    fig.tight_layout()

def make_time_graph(numpy_times, cpp_times):
    
    x = np.arange(len(labels)) 
    width = 0.35
    

    fig, ax = plt.subplots()
    rects1 = ax.bar(x - width / 2, numpy_times, width, label='PauliArray (Python)')
    rects2 = ax.bar(x + width / 2, cpp_times, width, label='DensePauliArray (C++)')

    for i in range(len(x)):
        ax.text(x[i] - width / 2, numpy_times[i] + 1, f'{numpy_times[i]:.1f} ms', ha='center', fontsize=8)
        ax.text(x[i] + width / 2, cpp_times[i] + 1, f'{cpp_times[i]:.1f} ms', ha='center', fontsize=8)
    
    ax.set_ylabel('Time (ms)')
    ax.set_title(f"PauliArray voids V.S C++ DensePauliArray (n={n}, dim={dim})\n(Lower is better)")
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    ax.legend()

    fig.tight_layout()


def main():

    numpy_times = np.zeros(len(labels))
    dense_cpp_times = np.zeros(len(labels))
    for i in range(reps):
        print(f"Rep {i+1}/{reps}")
        numpy_times += benchmark_np.test_battery(n, dim, mode) # pyright: ignore[reportOperatorIssue]
        dense_cpp_times += benchmark_dpa.test_battery(n, dim, mode) # pyright: ignore[reportOperatorIssue]

    print("PauliArray (Voids) average times (ms):    ", numpy_times/reps)
    print("DensePauliArray (C++) average times (ms): ", dense_cpp_times/reps)


    make_normalized_graph(numpy_times, dense_cpp_times)
    make_time_graph(numpy_times/reps, dense_cpp_times/reps)
    plt.show()

    

if __name__ == "__main__":
    main()