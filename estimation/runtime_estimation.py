import os
import json
import subprocess
import uuid
import sys
import multiprocessing
import time

cur_version = sys.version_info
#print (cur_version)

#default parameters values

sampler_fn = "minisat_static"
solver_fn = "genipainterval-picosat961"
wd = ""
mode =  "automatic",
multiple_diapasons = True,
separate_diapasons = True,
number_of_diapasons = 100,
diapason_size = 100000,
number_of_assumptions = 1000  
use_all_cores = True
number_of_processes = 0
max_fraction_of_space = 0.1
timeout_reserve = 5
time_limit = 0
#--------------------


def isFloat(x):
    try:
        float(x)
        return True
    except ValueError:
        return False

#multiprocessing functions
def mp_solve (input, event):
    res = 0
    solver_path = input[0]
    cnf_fn = input[1]
    assumptions_fn = input[2]    
    time_to_live = round(input[3] * timeout_reserve)
    if time_to_live<5:
        time_to_live = 5
    if time_to_live > 10000:
        time_to_live = 10000
    if time_limit!=0:
        time_to_live = time_limit
    #print("Time to live: ",time_to_live)
   # print("time to live: ",time_to_live)
    #print(solver_path)
    if not event.is_set():
        t1 = time.perf_counter()
        try:
        #    runsolve = subprocess.Popen([solver_path, cnf_fn, assumptions_fn], bufsize=-1, stdout=subprocess.PIPE, timeout = time_to_live)
            runsolve = subprocess.run([solver_path, cnf_fn, assumptions_fn], stdout=subprocess.PIPE, encoding='utf-8', timeout = time_to_live)
        except subprocess.TimeoutExpired:
            event.set()
            res = -1   

        t2=time.perf_counter()      
        if res!=-1:
            res = t2-t1            
       # print(res)
    return res
    

def mp_handler(assumptions_dict, number_of_processes):    
    
    p = multiprocessing.Pool(number_of_processes)
    m = multiprocessing.Manager()
    event = m.Event()
    #res =  p.map(mp_solve, assumptions_dict)    
    res = [p.apply_async(mp_solve,(i,event)) for i in assumptions_dict]
    output = [u.get() for u in res]
    #for i, r in enumerate(p.imap_unordered(mp_solve, (assumptions_dict,event), 1):    
    #    res.append(r)
    #    if r==-1:
            
    #        print('-1!')
    #        break
            
        #total_time+=r
        #sys.stdout.write('\rdone {}'.format(i/len(assumptions_dict)))           
    #p.close()
    #p.join()
    
    
    #print(str(output))
    return output

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
if (len(decomposition_set)==0):
    with open ('./d_set') as d_set:
        ds = json.load(d_set)
    #    print(ds)
        decomposition_set = ds['decomposition_set']

if (bkv>0):
    time_limit_per_task = bkv / 2**len(decomposition_set)
else:
    time_limit_per_task=time_limit
#print("Time limit per task :",time_limit_per_task)
#adjusting diapason_size and number_of_diapasons
#to take into account the size of the decomposition set.        
d_set_size = 2**len(decomposition_set)

print(decomposition_set)


if (number_of_diapasons*diapason_size)>(max_fraction_of_space*d_set_size):
    mode = "hybrid"
       
#print("Parameters after adjustment")
#print("Diapason size:",diapason_size)
#print("Number of diapasons:",number_of_diapasons)
#print("Number of assumptions:",number_of_assumptions)

pd= {}
pd['decomposition_set'] = decomposition_set
pd['mode'] = mode
pd['multiple_diapasons'] = multiple_diapasons
pd['number_of_diapasons'] = number_of_diapasons
pd['diapason_size'] = diapason_size
pd["number_of_assumptions"] = number_of_assumptions
pd["separate_diapasons"] = separate_diapasons

json_specification = json.dumps(pd)
uuid_string =str(uuid.uuid4()) 
ufn = "./" + uuid_string

with open(ufn,'w') as txtfile:
    txtfile.write(json_specification)

#runsampling = subprocess.Popen([sampler_fn, '-sampling', ufn, cnf_fn, ufn+"_out"],
#bufsize=-1, stdout=subprocess.PIPE)

runsampling = subprocess.run([sampler_fn, '-sampling', ufn, cnf_fn, ufn+"_out"],
stdout=subprocess.PIPE, encoding='utf-8')

#bufsize=-1, stdout=subprocess.PIPE, encoding='utf-8')
#print (runsampling.stdout)

assumption_files=[]
assumption_files_size=[]
lines = str(runsampling.stdout).split("\n")       
#while True:
#  line_ne = runsampling.stdout.readline()  
# line = format(line_ne.decode('utf-8'))
total_valid_points = 0
for line in lines:
  #print(line)
  if (line.startswith("Valid points:")):
      total_valid_points = int (line.replace("Valid points: ",""))
      d_set_size = total_valid_points
  if (line.startswith(ufn)):    
    tmp = line.rstrip().split(" ")
    assumption_files.append(tmp[0])
    if (len(tmp)>0):
        assumption_files_size.append(int(tmp[1]))
  if line =='':
    break

#print(assumption_files)
#print(assumption_files_size)

#the fun part will be here

#print("number_of_processes ",number_of_processes)

solve_data=[]
#print(time_limit_per_task)

for i in range(len(assumption_files)):
    s=assumption_files[i]
    tmp=[]
    tmp.append(solver_fn)
    tmp.append(cnf_fn)
    tmp.append(s) 
    tmp.append(time_limit_per_task*assumption_files_size[i])
    solve_data.append(tmp)   

#print(str(solve_data))
#f = mp_solve (solve_data[0])
#print (f)

if __name__ == '__main__':
    results = mp_handler(solve_data, number_of_processes)
    time_total = 0
    assumptions_total = 0

    for i in range(len(results)):        
        if results[i]!=-1:
            time_total = time_total + results[i]
            assumptions_total = assumptions_total +assumption_files_size[i]
        else:
            time_total = -1
            break
#        t = [float(s) for s in p.split(" ") if isFloat(s)]
#        if len(t)==2:
#            time_total = time_total + t[1]
#            assumptions_total = assumptions_total +t[0]
#        else:
#            time_total = time_total + t[0]
#            assumptions_total = assumptions_total + number_of_assumptions
#        #s.find("The input formula is satisfiable")>=0)
        
    if (time_total!=-1):
        re = (time_total / assumptions_total ) * d_set_size
    else:
        re=-1
#print(results)
    
#clean up
os.remove(ufn)
for f in assumption_files:
    os.remove(f)

    

print("Result for SMAC: SUCCESS, 0, 0, %f, 0" % re)
#print("Done!")
