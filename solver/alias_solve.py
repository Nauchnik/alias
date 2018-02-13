from __future__ import division
import os
import json
import subprocess
import uuid
import sys
import multiprocessing
import time
import re


cur_version = sys.version_info
#print (cur_version)

#default parameters values

sampler_fn = "minisat_static"
solver_fn = "genipainterval-picosat961"
cnf_fn =""
wd = ""
mode =  "automatic"
multiple_diapasons = True
separate_diapasons = True
number_of_diapasons = 100
block_size = 100
diapason_size = 100000
number_of_assumptions = 1000  
use_all_cores = True
number_of_processes = 0
max_fraction_of_space = 0.1
timeout_reserve = 5
time_limit = 0
number_of_workunits = 100
max_number_of_files_total = 100
SS=''
#--------------------


def isFloat(x):
    try:
        float(x)
        return True
    except ValueError:
        return False

#multiprocessing functions
def mp_solve (input,event):    
    
    _index = input[0]
    _diapason_start = input[1]
    _diapason_end = input[2]    
    _res = 0
    if not event.is_set():    
        _pd= {}
        _pd['decomposition_set'] = decomposition_set
        _pd['mode'] = "manual_whole"
        _pd['diapason_start'] = _diapason_start
        _pd['diapason_end'] = _diapason_end    
        _pd["block_size"] = block_size
        json_specification = json.dumps(_pd)
        #print(json_specification)
        cur_ufn = "./" + str(_index)+"_"+str(uuid.uuid4())

        with open(cur_ufn,'w') as txtfile:
            txtfile.write(json_specification)
        #print (cnf_fn)
        runsampling = subprocess.run([sampler_fn, '-sampling', cur_ufn, cnf_fn, cur_ufn+"_out"],
        stdout=subprocess.PIPE, encoding = 'utf-8')

        lines = runsampling.stdout.split('\n')
        cur_assumption_files=[]
        for line in lines:
            #print(line)
            if (line.find(cur_ufn) != -1):    
                tmp = (line.rstrip().split(" "))[0]
                cur_assumption_files.append(tmp)    
        #print(str(assumption_files))

        t1 = time.perf_counter()

        for l in cur_assumption_files:   
            if not event.is_set():
                runsolve = subprocess.Popen([solver_fn, cnf_fn, l], stdout=subprocess.PIPE, encoding='utf-8')
                runsolve.wait()       

                flag = False
                for line in runsolve.stdout:          
                    if (line.find("formula is satisfiable")!=-1):
                        print("Satisfying assignment found.")            
                        event.set()
                    if flag==True:   
                        SS=line.rstrip()
                        print(SS)
                        flag=False
                    if (line.find("satisfying assignment")!=-1): 
                        flag=True
                    
        t2=time.perf_counter()      

        os.remove(cur_ufn)
        for f in cur_assumption_files:
            os.remove(f)
        if _res>=0:
            _res = t2-t1
        # print(res)
    return _res

def log_result(results, event, n_of_points):
    def lr(retval):
        results.append(retval)
        #if (len(results)%5==0):
        if not event.is_set():
            print ('{} % done'.format(100*len(results)/n_of_points))
    return lr
    

def mp_handler(solve_data, number_of_processes):    
    if __name__ == '__main__':
        p = multiprocessing.Pool(number_of_processes)
        m = multiprocessing.Manager()
        event = m.Event()
        results = []
        for item in solve_data:
            p.apply_async(mp_solve,(item,event),callback=log_result(results,event,len(solve_data)))        
            
        p.close()
        p.join()
        return results

#--------------------------

#loading parameters from file
current_path = dir_path = os.path.dirname(os.path.realpath(__file__))+"/"
with open (current_path+'settings.ini') as settings_data:
    d = json.load(settings_data)
    #print(d)
    if ("solver_fn" in d.keys()):
        solver_fn = d['solver_fn']
    if ("sampler_fn" in d.keys()):
        sampler_fn = d['sampler_fn']
    if ("cnf_fn" in d.keys()):
        cnf_fn = d['cnf_fn']
    if ("working_path" in d.keys()):
        wd = d['working_path']
    if ("mode" in d.keys()):
        mode = d['mode']
    if ("multiple_diapasons" in d.keys()):
        multiple_diapasons = d['multiple_diapasons']
    if ("separate_diapasons" in d.keys()):
        separate_diapasons = d['separate_diapasons']
    if ("number_of_diapasons" in d.keys()):
        number_of_diapasons = d['number_of_diapasons']
    if ("diapason_size" in d.keys()):
        diapason_size = d['diapason_size']
    if ("number_of_assumptions" in d.keys()):
        number_of_assumptions = d['number_of_assumptions']
    if ("use_all_cores" in d.keys()):
        use_all_cores = d["use_all_cores"]
    if ("number_of_processes" in d.keys()):
        number_of_processes = d["number_of_processes"]
    if ("max_fraction_of_space" in d.keys()):
        max_fraction_of_space = d["max_fraction_of_space"]
    if ("time_limit" in d.keys()):
        time_limit = d["time_limit"]
    if ("block_size" in d.keys()):
        block_size = d["block_size"]
    if ("number_of_workunits" in d.keys()):
        number_of_workunits = d["number_of_workunits"]
    if ("max_number_of_files_total" in d.keys()):
        max_number_of_files_total = d["max_number_of_files_total"]
#


if solver_fn.find("/")==-1:
    #solver_fn=current_path + solver_fn
    solver_fn = "./" + solver_fn

if sampler_fn.find("/")==-1:
    sampler_fn = "./"  + sampler_fn

if cnf_fn.find("/")==-1:
    cnf_fn = "./"  + cnf_fn


if wd =="":
    wd = current_path
os.chdir(wd)

if use_all_cores==True:
    number_of_processes = multiprocessing.cpu_count()
else:
    if number_of_processes==0:
        number_of_processes = 1

#if we dont have decomposition set specified by argv
#we attempt to read it from cwd
decomposition_set =[]
#we also load best known value
bkv = 0
#print (str(sys.argv))
if len(sys.argv) > 2:
    for i in range(len(sys.argv)-1):
        if (sys.argv[i] == '-cnf'):
            cnf_fn = sys.argv[i+1]
        if (sys.argv[i] == '-solver'):
            solver_fn = sys.argv[i+1]
        if (sys.argv[i] == '-bkv'):
            bkv = float(sys.argv[i+1])
        if sys.argv[i+1] == '1':
            decomposition_set.append( int ( sys.argv[i].replace('-v','') ) )
if len(decomposition_set)==0:
    with open ('./d_set') as d_set:
        data = d_set.read().replace('\n','')
        decomposition_set =  [int(u) for u in re.findall(r'\d+',data)]
    #    print(ds)
        #decomposition_set = ds['decomposition_set']

#print(decomposition_set)
time_limit_per_task = bkv / 2**len(decomposition_set)
#print("Time limit per task :",time_limit_per_task)
#adjusting diapason_size and number_of_diapasons
#to take into account the size of the decomposition set.        
d_set_size = 2**len(decomposition_set)

#print(decomposition_set)


number_of_workunits = round((d_set_size *number_of_processes)/ (block_size*max_number_of_files_total))

milestones = []
milestones.append(0)
for i in range(number_of_workunits):
    milestones.append(round((i+1)*d_set_size/number_of_workunits))
milestones[len(milestones)-1] = d_set_size-1

diapason_delimiters = []

mask = '0'+str(len(decomposition_set))+'b'
#tmp = [int(t) for t in format(m, mask)]

milestones = [[int(t) for t in format(m, mask)] for m in milestones]



print("Splitting the decomposition space of {} assumptions into {} workunits with blocks of size {} and \
 processing them with {} processes. ".format(d_set_size, number_of_workunits, block_size, number_of_processes))


solve_data=[]
#print(time_limit_per_task)

for i in range(len(milestones)-1):    
    tmp=[]    
    tmp.append(i)
    tmp.append(milestones[i])
    tmp.append(milestones[i+1])
    solve_data.append(tmp)   





if __name__ == '__main__':
    solving_start = time.perf_counter()  
    results = mp_handler(solve_data, number_of_processes)
    solving_end = time.perf_counter()  
    
    time_total = 0    

    for i in range(len(results)):        
        if results[i]!=-1:
            time_total = time_total + results[i]            
        else:
            time_total = -1
            break
        
    if (time_total!=-1):
        re = time_total 
    else:
        re=-1

#print(results)
    
#clean up
#print("")
print("Solved! Cpu time: {} Wall time: {}".format(re,solving_end-solving_start))

#print("Done!")







