#!/bin/bash
solver="$1"
cnf="$2"
pcs=""
if [ "$#" -eq 3 ]; then
    pcs="$3"
    echo "pcsfile $pcs"
fi
solver_base=$(basename $solver);
cnf_base=$(basename $cnf)
echo "copy alias files for solver $solver_base and cnf $cnf_base"
folder="tmp_"$solver_base"_"$cnf_base;
echo "folder $folder"
rm -rf $folder
mkdir $folder
cp ./alias/igbfs ./$folder
cp ./alias/runtime_estimation.py ./$folder
cp ./alias/sampler_static ./$folder
cp ./alias/settings.ini ./$folder
cp ./alias/alias_solve2.py ./$folder
cp $solver ./$folder
cp $cnf ./$folder
if [ "$pcs" != "" ]; then
    echo "copy pcs $pcs"
    cp $pcs ./$folder
fi
echo "done"