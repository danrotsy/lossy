import random

def find_closest_two(list_vals,val):
    '''
    if val is less than min(list_vals), returns min(list_vals) twice
    if val is greater than max(list_vals), returns max(list_vals) twice
    if an element in list_vals is equal to val, return val twice
    '''
    left, left_index = find_left(list_vals,val)
    right, right_index = find_right(list_vals,val)
    return left, right, left_index, right_index

def find_left(list_vals, val):
    '''
    there can only be one of each val in the list
    if left most val, returns val
    if val is in list, returns val
    list_vals does not need to be sorted
    '''
    # if first element
    if val <= min(list_vals):
        return min(list_vals),  list_vals.index(min(list_vals))
    # else
    else:
        less_than_list = []
        closest = None
        closest_dist = None
        for e in list_vals:
            if e <= val:
                less_than_list.append(e)
        for l in less_than_list:
            if closest_dist == None:
                closest = l
                closest_dist = abs(val -l)
            else:
                dist = abs(val-l)
                if dist < closest_dist:
                    closest = l
                    closest_dist = dist
        return closest, list_vals.index(closest)

def find_right(list_vals,val):
    '''
    there can only be one of each val in the list
    if right most val, returns val
    if val ins in list, returns val
    list_vals does not need to be sorted
    '''
    # if last element
    if val >= max(list_vals):
        return max(list_vals),  list_vals.index(max(list_vals))
    # else
    else:
        greater_than_list = []
        closest = None
        closest_dist = None
        for e in list_vals:
            if e >= val:
                greater_than_list.append(e)
        for g in greater_than_list:
            if closest_dist == None:
                closest = g
                closest_dist = abs(val -g)
            else:
                dist = abs(val-g)
                if dist < closest_dist:
                    closest = g
                    closest_dist = dist
        return closest, list_vals.index(closest)

def generate_random_list(length,min_val, max_val):
    index = 0
    rand_list = []
    while index != length:
        val = float(random.randint(min_val,max_val))
        if val not in rand_list:
            rand_list.append(val)
            index += 1
    return rand_list

def test():
    for i in range(0,100):
        min_val = random.randint(-10,-1)
        max_val = random.randint(1,10)
        done = False
        while not done:
            val = float(random.randint(-22,22))*0.5
            test_list = generate_random_list(max_val-min_val,min_val,max_val)
            done = True
            if (val > max(test_list)) or (val < min(test_list)):
                done = False
        left, right = find_closest_two(test_list,val)
        if (right < val) or (left > val) or (left not in test_list) or (right not in test_list):
            print 'Failed ' ,test_list, left, val, right
        else:
            print 'Success' ,test_list, left, val, right
