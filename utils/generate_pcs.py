import sys

text_file = open("in", "r")
#var_numbers = text_file.read().split(',')
var_numbers = text_file.read().split('-')
text_file.close()
# pcs line example: activity {0,1,2}[0]
# output format: v1 {0,1}[1] \n v2 {0,1}[1]
text_file = open("out", "w")
#for var_numb in var_numbers:
var_numb = int(var_numbers[0])
while var_numb <= int(var_numbers[1]):
    text_file.write("v"+str(var_numb)+" {0,1}[1]\n")
    var_numb=var_numb+1
text_file.close()

