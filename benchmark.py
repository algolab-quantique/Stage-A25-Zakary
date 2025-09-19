import time
import qiskit
import matplotlib.pyplot as plt
import numpy as np

import pauliarray as pa
import paulicpp as pc
import densepaulicpp as dpc

# n = 250000000
n = 1000
dim = 100000
# n = 50000000

def pa_py(option="all"):
    start_time = time.time()
    shape = (dim,)
    p1 = pa.PauliArray.random(shape, n)
    p2 = pa.PauliArray.random(shape, n)
    random_gen_time = time.time()
    random_duration = (random_gen_time - start_time) * 1000
    match option:
        case "all":
            pr = p1.tensor(p2)
            tensor_time = time.time()
            tensor_duration = (tensor_time - random_gen_time) * 1000
            pr = p1.commute_with(p2)
            commutes_time = time.time()
            commutes_duration = (commutes_time - tensor_time) * 1000
            pr = p1.compose(p2)
            compose_time = time.time()
            compose_duration = (compose_time - commutes_time) * 1000
            
            return random_duration, tensor_duration, commutes_duration, compose_duration
            
        case "tensor":
            pr = p1.tensor(p2)
            tensor_time = time.time()
            tensor_duration = (tensor_time - random_gen_time) * 1000
            return random_duration, tensor_duration


        case "commutes":
            pr = p1.commute_with(p2)
            commutes_time = time.time()
            commutes_duration = (commutes_time - random_gen_time) * 1000
            return random_duration, commutes_duration

        case "compose":
            pr = p1.compose(p2)
            compose_time = time.time()
            compose_duration = (compose_time - random_gen_time) * 1000
            return random_duration, compose_duration


def pa_cpp():
    start_time = time.time()
    p1 = pc.PauliArray.random(n)
    p2 = pc.PauliArray.random(n)
    random_gen_time = time.time()
    results = p1.commutes_numpy(p2)
    end_time = time.time()
    return (random_gen_time - start_time) * 1000, (end_time - random_gen_time) * 1000

def dpa_cpp(option="all"):
    start_time = time.time()
    p1 = dpc.DensePauliArray.random(dim, n)
    p2 = dpc.DensePauliArray.random(dim, n)
    random_gen_time = time.time()
    random_duration = (random_gen_time - start_time) * 1000

    match option:
        case "all":
            pr = p1.tensor(p2)
            tensor_time = time.time()
            tensor_duration = (tensor_time - random_gen_time) * 1000
            pr = p1.commutes_batch(p2)
            commutes_time = time.time()
            commutes_duration = (commutes_time - tensor_time) * 1000
            pr = p1.compose_batch(p2)
            compose_time = time.time()
            compose_duration = (compose_time - commutes_time) * 1000
            
            
            return random_duration, tensor_duration, commutes_duration, compose_duration
            
        case "tensor":
            pr = p1.tensor(p2)
            tensor_time = time.time()
            tensor_duration = (tensor_time - random_gen_time) * 1000
            return random_duration, tensor_duration


        case "commutes":
            pr = p1.commutes_batch(p2)
            commutes_time = time.time()
            commutes_duration = (commutes_time - random_gen_time) * 1000
            return random_duration, commutes_duration

        case "compose":
            pr = p1.compose_batch(p2)
            compose_time = time.time()
            compose_duration = (compose_time - random_gen_time) * 1000
            return random_duration, compose_duration

def make_normalized_graph(numpy_times, cpp_times):
    labels = ['Random Generation', 'Tensor', 'Commutes', 'Compose']
    
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
    labels = ['Random Generation', 'Tensor', 'Commutes', 'Compose']
    
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
    reps = 10
    numpy_times = np.zeros(4)
    dense_cpp_times = np.zeros(4)
    for i in range(reps):
        print(f"Rep {i+1}/{reps}")
        numpy_times += pa_py("all")
        dense_cpp_times += dpa_cpp("all")
    print("PauliArray (Voids) average times (ms):    ", numpy_times/reps)
    print("DensePauliArray (C++) average times (ms): ", dense_cpp_times/reps)


    make_normalized_graph(numpy_times, dense_cpp_times)
    make_time_graph(numpy_times/reps, dense_cpp_times/reps)
    plt.show()

    

if __name__ == "__main__":
    main()