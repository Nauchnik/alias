#!/bin/bash
filename="$1"
read str < "$filename"
IFS=', ' read -r -a array <<< "$str"
for element in "${array[@]}"
do
    echo "v$element {0,1}[1]"
done
