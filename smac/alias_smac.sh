#!/bin/bash
if [ "$#" -ne 2 ]; then
    echo "Illegal number of parameters"
    echo "Usage: scriptname scenario_name out_name"
    exit 1
fi
echo "SMAC is starting"
scenario_name="$1"
out_name="$2"        
export SMAC_MEMORY=65536
nohup ./smac --scenario-file $scenario_name &> $out_name
