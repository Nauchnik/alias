import os
import json
import subprocess
import uuid
import sys
import multiprocessing
import time
import logging 

import re
from shutil import copyfile

current_path = os.path.dirname(os.path.realpath(__file__))+"/"


def isInteger(s):
    try: 
        int(s)
        return True
    except ValueError:
        return False


#define dictionary for settings

settings = {}
#general settings
settings["solver"] = "genipainterval-glucose4"
settings["satisfiying_assignment_string"] = "formula is satisfiable"
settings["solverargs"] = {}
settings["sampler"]="sampler_static"
settings["cnf"] = ""
settings["wd"] = ""
settings["useallcores"] = True
settings["numproc"] =  multiprocessing.cpu_count()
settings["blocksize"] = 50
settings["maxnumfiles"] = settings["numproc"] * 10

settings["number_of_workunits"] = -1
settings["mode"] = ""

#sampling settings
settings["usesampler"] = True
settings["numdiap"] = 50
settings["diapsize"] = 10000
settings["numassumpts"] = 100
settings["maxtlpertask"] = 100
settings["multidiap"] = True
settings["estimout"] = "Result for SMAC: SUCCESS, 0, 0, {}, 0"
settings["bkv"] = -1


#solving settings
settings["wtlimitsolve"] = 3600 * 24
settings["solveout"] = "Solved! Cpu time: {} Wall time: {}"


#auxiliary storage
settings["diapason_winner"] = ""

settings["decomposition_set"] = []

settings["SS"] = []

settings["solving_started"] = 0
settings["interrupted"] = False

def parse_commandline():
    if len(sys.argv) == 1:
        printusage()
        sys.exit()
    else: 
        if len(sys.argv) >= 2:         
            if sys.argv[1] == "-estimate" or sys.argv[1] == "-e":
                settings["mode"] = "estimate"
            else:
                if sys.argv[1] == "-solve" or sys.argv[1] == "-s":
                    settings["mode"] = "solve"
                else:
                    print ("Mode is not selected!")
                    #printusage()
                    sys.exit()
            for i in range(2, len(sys.argv)):        
                if (sys.argv[i] == '-solver'):
                    settings["solver"] = sys.argv[i+1]
                if (sys.argv[i] == '-sampler'):
                    settings["sampler"] = sys.argv[i+1]        
                if (sys.argv[i] == '-cnf'):
                    settings["cnf"] = sys.argv[i+1] 
                if (sys.argv[i] == '-wd'):
                    settings["wd"] = sys.argv[i+1]
                if (sys.argv[i] == '-numproc'):
                    settings["numproc"] = int(sys.argv[i+1])
                if (sys.argv[i] == '-blocksize'):
                    settings["blocksize"] = sys.argv[i+1]
                if (sys.argv[i] == '-maxnumfiles'):
                    settings["maxnumfiles"] = int(sys.argv[i+1])
                if (sys.argv[i] == '-nosampler'):
                    settings["usesampler"] = False
                if (sys.argv[i] == '-numdiap'):
                    settings["numdiap"] = int(sys.argv[i+1])
                if (sys.argv[i] == '-diapsize'):
                    settings["diapsize"] = int(sys.argv[i+1])
                if (sys.argv[i] == '-numassumpts'):
                    settings["numassumpts"] = int(sys.argv[i+1])
                if (sys.argv[i] == '-maxtlpertask'):
                    settings["maxtlpertask"] = int(sys.argv[i+1])
                if (sys.argv[i] == '-wtlimitsolve'):
                    settings["wtlimitsolve"] = float(sys.argv[i+1])
                if (sys.argv[i] == '-workunits'):
                    settings["number_of_workunits"] = int(sys.argv[i+1])
                if (sys.argv[i] == '-bkv'):
                    settings["bkv"] = float(sys.argv[i+1])        
                if sys.argv[i].startswith("-v"):                    
                    if sys.argv[i+1] == '1':                        
                        settings["decomposition_set"].append( int ( sys.argv[i].replace('-v','') ) )            
    #assimilate settings
    if settings["solver"] == "":
        print("Solver is not specified")
        sys.exit()

    if settings["solver"].find("/")==-1:
    #solver_fn=current_path + solver_fn
        settings["solver"] = "./" + settings["solver"] 


    if settings["sampler"] == "" and settings["usesampler"] == True:
        print("Sampler is not specified")
        sys.exit()

    if settings["sampler"] .find("/")==-1:
        settings["sampler"] = "./"  + settings["sampler"]

    if settings["cnf"] == "":
        print("No cnf file given")
        sys.exit()

    if settings["cnf"].find("/")==-1:
        settings["cnf"] = "./"  + settings["cnf"]

    if settings["wd"] =="":
        settings["wd"] = current_path

    os.chdir(settings["wd"])
        
    logging.basicConfig(filename="alias.log",level=logging.DEBUG, format = '%(asctime)s %(message)s',\
    datefmt = '%d/%m/%Y %I:%M:%S %p')

    if (len(settings["decomposition_set"])==0):
        if os.path.isfile("./d_set") == True:
            with open ('./d_set') as d_set:
                data = d_set.read().replace('\n','')
                settings["decomposition_set"] =  [int(u) for u in re.findall(r'\d+',data)]
        else:
            print("Decomposition set is not specified!")
            #logging.info("Decomposition set not specified")
            sys.exit()    
    dset_size = 2**len(settings["decomposition_set"])

    if settings["numproc"] <=0:
        if settings["useallcores"]==True: 
            settings["numproc"] = multiprocessing.cpu_count()
        else:
            settings["numproc"] = 1

    max_number_of_blocks_per_process = settings["maxnumfiles"]/settings["numproc"]
    
    if max_number_of_blocks_per_process < 1: 
        max_number_of_blocks_per_process = 1
    else:
        max_number_of_blocks_per_process = round(max_number_of_blocks_per_process)
    
    
    bkv_ttl = -1
    if settings["bkv"] > 0:
        bkv_ttl = settings["bkv"]*10/(2**len(settings["decomposition_set"]))
        if bkv_ttl < settings["maxtlpertask"]:
            settings["maxtlpertask"] = bkv_ttl
    

    #print("maxnumblocksperprocess = {}".format(max_number_of_blocks_per_process))

    if settings["number_of_workunits"] <= 0 :
        if settings["mode"] == "estimate":
            blocks_total = settings["numdiap"] * settings["numassumpts"] / settings["blocksize"]
            settings["number_of_workunits"] = 1

        if settings["mode"] == "solve":    
            blocks_total = round(dset_size / settings["blocksize"]) + 1        
            settings["number_of_workunits"] = round(blocks_total/max_number_of_blocks_per_process)
            
            min_wu_number = round(blocks_total/max_number_of_blocks_per_process)
            max_wu_number = blocks_total
            
            if 4*settings["numproc"] > min_wu_number and 4*settings["numproc"] < max_wu_number:            
                settings["number_of_workunits"] = 4 * settings["numproc"]            
            


    #print("blocks_total = {}".format(blocks_total))

    
    #if settings["number_of_workunits"] < settings["numproc"]:
    #    print("Increasing number of workunits to number of processes.")
    #    print("Please consider reevaluating sampling/solving parameters (maxnumfiles, numdiap, numassumpts)\n\
    #    to make more effective use of available computing power")
    #    settings["number_of_workunits"] = settings["numproc"]

def printusage():
    print("This is ALIAS main script.")
    print("To launch ALIAS in runtime estimation mode use ALIAS -estimate [args] or ALIAS -e [args]\n")
    print("To launch ALIAS in solving mode use ALIAS -solve [args] or ALIAS -s [args]\n")
    print("The following arguments are recognized from the command line.")
    print("Command line valued override that specified in settings.")
    print("General settings")
    print("\t -solver \"solvername\" to specify solver binary")    
    print("\t -sampler \"samplername\" to specify sampler binary")
    print("\t -cnf \"cnfname\" to specify cnf file path")
    print("\t -wd \"working directory\" to specify working directory")
    print("\t -numproc <int> to specify number of processes that the script can use")
    print("\t  by default it is equal to multiprocessing.cpu_count().")
    print("\t -blocksize <int> to specify the number of assumptions per file")
    print("\t -maxnumfiles <int> to specify maximum amount of temporary files in wd")
    print("Sampling settings")
    print("\t -nosampler to avoid using sampler")
    print("\t -numdiap <int> to specify the number of diapasons in random sample")
    print("\t -diapsize <int> to specify the size of a diapason in a random sample")
    print("\t -numassumpts <int> to specify the number of valid assumptions \n\t to be sampled from a diapason")
    print("\t -maxtlpertask <int> to specify the maximum time limit per assumption")
    print("Solving settings")
    print("\t -wtlimitsolve <int> to specify the time limit for solving in seconds")

    print("\nThe script by default loads backdoor from d_set file in working directory.")
    print("However, backdoor variables can also be specified in SMAC-like manner, i.e. ")
    print("ALIAS -e -v1 1 -v2 0 -v3 1 means that the decomposition set is {x1, x3}.")
    print("The entries specifying variables NOT included in backdoor (e.g. -v2 0) can be omitted.")
    

def dump_settings_to_file(filename_st):
    #forming a dictionary in beautiful format
    settings_json_dict = {}
    settings_json_dict["general_settings"]={}
    settings_json_dict["general_settings"]["solver"] = settings["solver"]
    settings_json_dict["general_settings"]["solver_settings"] = {}
    settings_json_dict["general_settings"]["solver_settings"]["satisfiying_assignment_string"]=\
    settings["satisfiying_assignment_string"]
    settings_json_dict["general_settings"]["solver_settings"]["solver_arguments"] = settings["solverargs"]
    settings_json_dict["general_settings"]["sampler_filename"] = settings["sampler"]
    settings_json_dict["general_settings"]["cnf_filename"] = settings["cnf"]
    settings_json_dict["general_settings"]["working_directory"] = settings["wd"]
    settings_json_dict["general_settings"]["use_all_cores"] = settings["useallcores"]
    settings_json_dict["general_settings"]["number_of_processes"] = settings["numproc"]
    settings_json_dict["general_settings"]["block_size"] = settings["blocksize"]
    settings_json_dict["general_settings"]["maximum_number_of_files_total"] = settings["maxnumfiles"]

    settings_json_dict["sampling_settings"]={}
    settings_json_dict["sampling_settings"]["use_sampler"] = settings["usesampler"] 
    settings_json_dict["sampling_settings"]["number_of_diapasons"] = settings["numdiap"]
    settings_json_dict["sampling_settings"]["diapason_size"] = settings["diapsize"]
    settings_json_dict["sampling_settings"]["number_of_assumptions"] = settings["numassumpts"] 
    settings_json_dict["sampling_settings"]["time_limit_per_task"] = settings["maxtlpertask"] 
    settings_json_dict["sampling_settings"]["multiple_diapasons"] = settings["multidiap"]
    settings_json_dict["sampling_settings"]["estimation_output_string"] = settings["estimout"]
    
    settings_json_dict["solving_settings"]={}
    settings_json_dict["solving_settings"]["time_limit_on_solving"] = settings["wtlimitsolve"] = 3600
    settings_json_dict["solving_settings"]["solving_output_string"] = settings["solveout"] = "Solved! Cpu time: {} Wall time: {}"
    
    #dumping settings to string
    json_export = json.dumps(settings_json_dict, indent=4)
    
    #writing the result to file
    with open(filename_st,'w') as txtfile:
        txtfile.write(json_export)


def load_settings_from_file(filename_st):
    s_in = {}
    with open (filename_st) as settings_input:
        s_in = json.load(settings_input)
    
    if ("general_settings") in s_in.keys():
        if "solver" in s_in["general_settings"].keys():
            settings["solver"] = s_in["general_settings"]["solver"]
        
        if "solver_settings" in s_in["general_settings"].keys():
        
            if "satisfiying_assignment_string" in s_in["general_settings"]["solver_settings"].keys():
               settings["satisfiying_assignment_string"] = s_in["general_settings"]["solver_settings"]["satisfiying_assignment_string"]
        
            if "solver_arguments" in s_in["general_settings"]["solver_settings"].keys():
                settings["solverargs"] = s_in["general_settings"]["solver_settings"]["solver_arguments"]
        
        if "sampler_filename" in s_in["general_settings"].keys():
            settings["sampler"] = s_in["general_settings"]["sampler_filename"]
        
        if "cnf_filename" in s_in["general_settings"].keys():
            settings["cnf"] = s_in["general_settings"]["cnf_filename"]            

        if "working_directory" in s_in["general_settings"].keys():
            settings["wd"] = s_in["general_settings"]["working_directory"]
    
        if "use_all_cores" in s_in["general_settings"].keys():
            settings["useallcores"] = s_in["general_settings"]["use_all_cores"]

        if "number_of_processes" in s_in["general_settings"].keys():
            settings["numproc"] = s_in["general_settings"]["number_of_processes"]

        if "block_size" in s_in["general_settings"].keys():
            settings["blocksize"] = s_in["general_settings"]["block_size"]
  
        if "maximum_number_of_files_total" in s_in["general_settings"].keys():
            settings["maxnumfiles"] = s_in["general_settings"]["maximum_number_of_files_total"]

    if ("sampling_settings") in s_in.keys():
        if "use_sampler" in s_in["sampling_settings"].keys():
            settings["usesampler"] = s_in["sampling_settings"]["use_sampler"]
  
        if "number_of_diapasons" in s_in["sampling_settings"].keys():
            settings["numdiap"] = s_in["sampling_settings"]["number_of_diapasons"]

        if "diapason_size" in s_in["sampling_settings"].keys():
            settings["diapsize"] = s_in["sampling_settings"]["diapason_size"]
    
        if "number_of_assumptions" in s_in["sampling_settings"].keys():
            settings["numassumpts"] = s_in["sampling_settings"]["number_of_assumptions"]
    
        if "time_limit_per_task" in s_in["sampling_settings"].keys():
            settings["maxtlpertask"] = s_in["sampling_settings"]["time_limit_per_task"]
    
        if "multiple_diapasons" in s_in["sampling_settings"].keys():
            settings["multidiap"] = s_in["sampling_settings"]["multiple_diapasons"]
    
        if "estimation_output_string" in s_in["sampling_settings"].keys():
            settings["estimout"] = s_in["sampling_settings"]["estimation_output_string"]
    
    if ("solving_settings") in s_in.keys():
        if "time_limit_on_solving" in s_in["solving_settings"].keys():
            settings["wtlimitsolve"] = s_in["solving_settings"]["time_limit_on_solving"]
        
        if "solving_output_string" in s_in["solving_settings"].keys():
            settings["solveout"] = s_in["solving_settings"]["solving_output_string"]

def print_state():
    if (settings["mode"] == "solve"):
        print("ALIAS launched in solving mode for cnf {}".format(settings["cnf"]))    
        logging.info("ALIAS launched in solving mode for cnf {}".format(settings["cnf"]))
    else:
        print("ALIAS launched in estimating mode for cnf {}".format(settings["cnf"]))        
        logging.info("ALIAS launched in estimating mode for cnf {}".format(settings["cnf"]))        
    print("Starting the computations using {} processes.".format(settings["numproc"]))
    print("Backdoor: "+str(settings["decomposition_set"]))
    logging.info("Starting the computations using {} processes.".format(settings["numproc"]))
    logging.info("Backdoor: "+str(settings["decomposition_set"]))
    if (settings["mode"]=="estimate"):
        print("Processing {} diapasons of {} assumptions divided into blocks of size {}".format(\
        settings["numdiap"], settings["numassumpts"],settings["blocksize"]))
        logging.info("Processing {} diapasons of {} assumptions divided into blocks of size {}".format(\
        settings["numdiap"], settings["numassumpts"],settings["blocksize"]))

    if (settings["mode"]=="solve"):        
        print("{} assumptions are split into {} workunits divided into blocks of size {}"\
        .format(str(2**len(settings["decomposition_set"])), settings["number_of_workunits"],\
        settings["blocksize"]))
        logging.info("{} assumptions are split into {} workunits divided into blocks of size {}"\
        .format(str(2**len(settings["decomposition_set"])), settings["number_of_workunits"],\
        settings["blocksize"]))
        if settings["wtlimitsolve"]>0:
            print("Wall time limit is set to {} seconds".format(settings["wtlimitsolve"]))
            logging.info("Wall time limit is set to {} seconds".format(settings["wtlimitsolve"]))

def initialize_settings():
    python_version = sys.version_info
    #add warning that if version is prior to 3.6 then the script is likely to fail and exit =)
    settings_fn = ""
    if os.path.isfile(current_path+"alias_settings") == True:
        settings_fn = current_path+"alias_settings"
    if (settings_fn == ""):
        print("alias_settings file not found. Dumping sample settings to " + current_path+"alias_settings")
        dump_settings_to_file(current_path+"alias_settings")
        sys.exit()
    else:
        load_settings_from_file(settings_fn)
    parse_commandline()
    print_state()
#parse command line arguments and print help
#printusage()

#Runtime estimation functions

def mp_launch_solver (input, event):
    res = 0
    solver_path = settings["solver"]
    cnf_fn = settings["cnf"]
    assumptions_fn = input[0]
    ttl = settings["maxtlpertask"]

    #scale ttl to number of tasks
    time_to_live = round(input[1] * ttl)
    if time_to_live < 1: 
        time_to_live = 10

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


def multiprocess_compute_estimations(assumptions_dict):    
    
    p = multiprocessing.Pool(settings["numproc"])
    m = multiprocessing.Manager()
    event = m.Event()    

    res = [p.apply_async(mp_launch_solver,(i,event)) for i in assumptions_dict]
    output = [u.get() for u in res]    

    return output

def run_sampler(sample_data):
    index = sample_data[0]
    sampler_pd = {}
    sampler_pd['decomposition_set'] = settings["decomposition_set"]
    sampler_pd['mode'] = sample_data[1]
    sampler_pd['multiple_diapasons'] = True
    sampler_pd['number_of_diapasons'] = sample_data[2]
    sampler_pd['diapason_size'] = settings["diapsize"]
    sampler_pd["number_of_assumptions"] = settings["numassumpts"]
    sampler_pd["separate_diapasons"] = True
    

    json_sampler_specification = json.dumps(sampler_pd)
    uuid_string =str(uuid.uuid4()) 
    sampler_ufn = "./" + str (index)+"_"+uuid_string
    with open(sampler_ufn,'w') as txtfile:
        txtfile.write(json_sampler_specification)    

    runsampling = subprocess.run([settings["sampler"], '-sampling', sampler_ufn, settings["cnf"], sampler_ufn+"_out"],
    stdout=subprocess.PIPE, encoding='utf-8')
    
    os.remove(sampler_ufn)
    
    assumption_files=[]    
    lines = str(runsampling.stdout).split("\n")       
    total_valid_points = 0 
    for line in lines:
    #print(line)    
        if (line.startswith("Valid points:")):
            total_valid_points = int (line.replace("Valid points: ",""))
            d_set_size = total_valid_points
        if (line.startswith(sampler_ufn)):    
            tmp = line.rstrip().split(" ")
            if (len(tmp)>0):
                tmp[1]=int(tmp[1])  
            assumption_files.append(tmp)
            
    return [total_valid_points, assumption_files]


def multiprocess_run_sampler(sampling_inputs):
    p = multiprocessing.Pool(settings["numproc"])    

    p_res = p.map(run_sampler,sampling_inputs)    
    all_files = []    
    for u in p_res:
        for h in u[1]:
            all_files.append(h)        
    
    return all_files


def ALIAS_estimate():
    
    #logging.info("Computing estimation for cnf {} and backdoor {}".format(str(settings["cnf"]),\
    #str(settings["decomposition_set"])))
    d_set_size = 2**len(settings["decomposition_set"])
    
    #print(str(d_set_size))
    #print(settings["numdiap"]*settings["diapsize"])
    sampling_mode = "automatic"
    if (settings["numdiap"]*settings["diapsize"])>(0.1*d_set_size):
        sampling_mode = "hybrid"
    
    files = []
    tvp = 0
    if sampling_mode == "hybrid":        
        sampling_dict = []
        sampling_dict.append(0)
        sampling_dict.append("hybrid")
        sampling_dict.append(settings["numdiap"])
        tvp, files = run_sampler(sampling_dict)
    
        
    else:        
        k = settings["numdiap"] // settings["numproc"]        
        r = settings["numdiap"] - k * settings["numproc"]        
        diap_sizes = [k]*settings["numproc"]
        for i in range (0,r):
            diap_sizes[i] = diap_sizes[i]+1
                    
        big_sampling_dict = []
        for i in range (0, settings["numproc"]):
            tmp = []
            tmp.append(i)
            tmp.append("automatic")
            tmp.append(diap_sizes[i])
            big_sampling_dict.append(tmp)
        files = multiprocess_run_sampler(big_sampling_dict)
    
    sampling_result = multiprocess_compute_estimations(files)
    for f in files:
        os.remove(f[0])
    runtime_estimation = 0
    sum = 0
    assumptions_total = 0
    for u in files:
        assumptions_total = assumptions_total + u[1]
    for v in sampling_result:
        if v == -1:
            runtime_estimation = -1
        sum = sum + v
    if runtime_estimation!=-1:
        runtime_estimation = (sum*d_set_size)/assumptions_total
    
    logging.info(settings["estimout"].format(runtime_estimation))
    print (settings["estimout"].format(runtime_estimation))
        
#---    
        

#Solve functions

def mp_process_workunit (input, SA_event, INTERRUPT_event, ERROR_event):        
    wu_index = input
    d_set_size = 2**len(settings["decomposition_set"])
    mask = '0'+str(len(settings["decomposition_set"]))+'b'

    wu_start_int = round((wu_index)*d_set_size/settings["number_of_workunits"])
    wu_end_int = round((wu_index+1)*d_set_size/settings["number_of_workunits"])
    
    wu_diapason_start =  [int(t) for t in format(wu_start_int, mask)]
    wu_diapason_end   =  [int(t) for t in format(wu_end_int, mask)]
    
    #tmp = [int(t) for t in format(m, mask)]
    wu_res = 0    
    if len(os.listdir(settings["wd"])) > settings["maxnumfiles"] * 1.5:        
        ERROR_event.set()
    if not SA_event.is_set() and not INTERRUPT_event.is_set() and not ERROR_event.is_set():                    
        
        wu_pd= {}
        wu_pd['decomposition_set'] = settings["decomposition_set"]
        wu_pd['mode'] = "manual_whole"
        wu_pd['diapason_start'] = wu_diapason_start
        wu_pd['diapason_end'] = wu_diapason_end    
        wu_pd["block_size"] = settings["blocksize"]
        wu_process_json = json.dumps(wu_pd)
        #print(json_specification)               
        wu_ufn = "./" + str(wu_index)+"_"+str(uuid.uuid4())        

        with open(wu_ufn,'w') as txtfile:
            txtfile.write(wu_process_json)
                
        #print (cnf_fn)
        wu_runsampling = subprocess.run([settings["sampler"], '-sampling', wu_ufn, settings["cnf"], wu_ufn+"_out"],
        stdout=subprocess.PIPE, encoding = 'utf-8')

        lines = wu_runsampling.stdout.split('\n')
        wu_assumption_files=[]
        wu_assumptions_total = 0
        for line in lines:
            #print(line)
            if (line.find(wu_ufn) != -1):    
                tmp = (line.rstrip().split(" "))[0]
                wu_assumption_files.append(tmp)    
                wu_assumptions_total = wu_assumptions_total + int ((line.rstrip().split(" "))[1])
        #print(str(assumption_files))


        t1 = time.perf_counter()

        for l in wu_assumption_files:   
            #check if wall time limit is not exceeded

            if settings["wtlimitsolve"]>0:
                if (time.perf_counter() - settings["solving_started"]) > settings["wtlimitsolve"]:                    
                    
                    INTERRUPT_event.set()

            if not SA_event.is_set() and not INTERRUPT_event.is_set():                    
                wu_runsolve = subprocess.Popen([settings["solver"], settings["cnf"], l], stdout=subprocess.PIPE, encoding='utf-8')
                wu_runsolve.wait()       

                flag = False
                for line in wu_runsolve.stdout:          
                    if (line.find(settings["satisfiying_assignment_string"])!=-1):
                        print("Satisfying assignment found.")            
                        settings["diapason_winner"] = l
                        copyfile(l,str(wu_index)+"_diap_ss")                        
                        SA_event.set()
                    if flag==True:   
                        settings["SS"]=line.rstrip()
                        print(settings["SS"])
                        flag=False
                    #totally dependent on ipasir now. 
                    #need a general enough way to move this functionality into settings
                    if (line.find("satisfying assignment")!=-1): 
                        flag=True
                    
        t2=time.perf_counter()      

        os.remove(wu_ufn)
        for f in wu_assumption_files:
            os.remove(f)
        
        
        wu_res = t2-t1
        # print(res)
    return wu_res, wu_assumptions_total

def log_result(results, index, event1, event2, event3, n_of_points):
    def lr(retval):
        results.append(retval[0])
        #if (len(results)%5==0):
        if not event1.is_set() and not event2.is_set() and not event3.is_set():
            logging.debug("Workunit {} [{} assumptions total] done in time {} ({}/{}) ".format(str(index), str(retval[1]), str(retval), str(len(results)),str(settings["number_of_workunits"])))
            if len(results) % 10 == 0:
                time_elapsed = time.perf_counter() - settings["solving_started"]
                estimation = (time_elapsed / len(results)) * n_of_points
                logging.debug("Elapsed time: {}. Estimated total time: {}.".format(str(time_elapsed),str(estimation)))
            print ('{} % done'.format(100*len(results)/n_of_points))
    return lr
    

def multiprocess_solve(solve_data):    
    if __name__ == '__main__':
        p = multiprocessing.Pool(settings["numproc"])
        m = multiprocessing.Manager()
        SA_event = m.Event()
        INTERRUPT_event = m.Event()
        ERROR_event = m.Event()
        results = []
        
        for item in solve_data:
            p.apply_async(mp_process_workunit,(item,SA_event, INTERRUPT_event, ERROR_event),\
            callback=log_result(results, item, SA_event,INTERRUPT_event,ERROR_event, len(solve_data)))                                
        p.close()
        p.join()
        if SA_event.is_set() == True:
            print("SA found!")
            logging.info("Satisfying assignment found")
        if INTERRUPT_event.is_set()== True:            
            settings["interrupted"] = True
            logging.info("Interrupted solving due to time limit")
        if ERROR_event.is_set()== True:                        
            logging.info("Interrupted solving due to ERROR (total number of files 1.5 times larger than should be")
    return results
   

def ALIAS_solve():
       
    solve_data = [i for i in range(0,settings["number_of_workunits"])]
    #print(str(solve_data))
    solving_start = time.perf_counter()  
    
    #logging.info("Starting solving for cnf {} with backdoor {}, time limit {}".format(\
    #settings["cnf"],str(settings["decomposition_set"]),str(settings["wtlimitsolve"])))

    #workunit_size = 2**len(settings["decomposition_set"]) / settings["number_of_workunits"]
    
    #logging.info("The search space is split into {} workuints of size {}".format(\
    #str(settings["number_of_workunits"]), str(workunit_size)))

    settings["solving_started"] = solving_start
    
    results = multiprocess_solve(solve_data)
    
    solving_end = time.perf_counter()  

    walltime = solving_end-solving_start    
    cputime = 0 

    for i in range(len(results)):             
        cputime = cputime + results[i]                    
    

    #print(results)
        
    #clean up
    #print("")
#    print(str(settings))
    if settings["interrupted"] == True:
        print("Solving interrupted due to the time limit.")
        print("Wall time spent: {}".format(walltime))
        print("CPU time spent: {}".format(cputime))
    else:
        print(settings["solveout"].format(cputime,walltime))


#---

initialize_settings()
if settings["mode"] == "estimate":
    ALIAS_estimate()
if settings["mode"] == "solve":
    ALIAS_solve()


    
