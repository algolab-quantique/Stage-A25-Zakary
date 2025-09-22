import time
import numpy as np
import densepaulicpp as dpc

def time_tensor(p1, p2):
    start_time = time.time()
    pr = p1.tensor(p2)
    end_time = time.time()
    duration = (end_time - start_time) * 1000 
    return duration

def time_commutes(p1, p2):
    start_time = time.time()
    pr = p1.commutes_batch(p2)
    end_time = time.time()
    duration = (end_time - start_time) * 1000 
    return duration

def time_compose(p1, p2):
    start_time = time.time()
    pr = p1.compose_batch(p2)
    end_time = time.time()
    duration = (end_time - start_time) * 1000 
    return duration

def time_identity(p1):
    start_time = time.time()
    pr = p1.is_identity()
    end_time = time.time()
    duration = (end_time - start_time) * 1000 
    return duration

def time_random_gen(n, shape):
    start_time = time.time()
    p = dpc.DensePauliArray.random(n, shape)
    end_time = time.time()
    duration = (end_time - start_time) * 1000 
    return p, duration

def time_new_identity(n, shape):
    start_time = time.time()
    p = dpc.DensePauliArray.identities(n, shape)
    end_time = time.time()
    duration = (end_time - start_time) * 1000 
    return p, duration

def time_both_identity(n, shape):
    start_time = time.time()
    p = dpc.DensePauliArray.identities(n, shape)
    pr = p.is_identity()
    end_time = time.time()
    duration = (end_time - start_time) * 1000 
    return duration

def time_swap_zx(p1):
    start_time = time.time()
    p1.swap_zx()
    end_time = time.time()
    duration = (end_time - start_time) * 1000 
    return duration

def time_traces(p1):
    start_time = time.time()
    tr = p1.traces()
    end_time = time.time()
    duration = (end_time - start_time) * 1000 
    return duration

def test_battery(n, dim, option="all"):
    shape = dim
    p1, random_duration = time_random_gen(n, shape)
    p2, random_duration2 = time_random_gen(n, shape)
    random_duration += random_duration2
    times_list = []
    times_list.append(random_duration)

    match option:
        case "all":
            tensor_duration = time_tensor(p1, p2)
            times_list.append(tensor_duration)

            commutes_duration = time_commutes(p1, p2)
            times_list.append(commutes_duration)

            compose_duration = time_compose(p1, p2)
            times_list.append(compose_duration)

            identity_duration = time_identity(p1)
            times_list.append(identity_duration)

            p_id, new_identity_duration = time_new_identity(n, shape)
            times_list.append(new_identity_duration)

            both_identity_duration = time_both_identity(n, shape)
            times_list.append(both_identity_duration)

            swap_duration = time_swap_zx(p1)
            times_list.append(swap_duration)

            # traces_duration = time_traces(p1)
            # times_list.append(traces_duration)
            return times_list
            
                    
        case "tensor":
            tensor_duration = time_tensor(p1, p2)
            times_list.append(tensor_duration)
            return times_list


        case "commutes":
            commutes_duration = time_commutes(p1, p2)
            times_list.append(commutes_duration)
            return times_list

        case "compose":
            compose_duration = time_compose(p1, p2)
            times_list.append(compose_duration)
            return times_list
        
        case "identity":
            identity_duration = time_identity(p1)
            times_list.append(identity_duration)
            return times_list
        
        
