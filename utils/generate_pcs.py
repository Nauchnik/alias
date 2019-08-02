print("enter number of variables")
var_count = int(input())
print("create pcs file for the %d first variables" % var_count)

pcs_file_name = "1-" + str(var_count) + ".pcs"
with open(pcs_file_name,'w') as ofile:
    for i in range(var_count):
        ofile.write("v" + str(i+1) + " {0,1}[1]\n")

print("pcs data was saved to " + pcs_file_name)