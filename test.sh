#!/bin/bash
make mrproper
make 


test(){
    n=0
    passed=0
    files=($1) 
    for f in ${files[@]}; do
        let n++
        printf "(%02d/%02d) - %s\n" $n ${#files[@]} "Testing $f..." 

        ./bin/tpcc < $f -n
        feedback=$PIPESTATUS

        echo $feedback >> ./test/feedback.txt 
        echo >> ./test/feedback.txt 

        if [ "$feedback" = "$2" ]; then
            let passed++
        fi
    done
    echo -e "$passed sources out of ${#files[@]} passed!" >> output.txt
    if [ $passed -eq ${#files[@]} ]; then
        echo -e "\033[0;32m$passed sources out of ${#files[@]} passed ! \033[0;37m" 
    elif [ $passed = "0" ]; then
        echo -e "\033[0;31m$passed sources out of ${#files[@]} passed ! \033[0;37m" 
    else
        echo -e "\033[0;33m$passed sources out of ${#files[@]} passed ! \033[0;37m" 
    fi
}

echo "=============== Valid tests  ===============" | tee -a output.txt
test "./test/good/*.tpc" 0 

echo "============== Syn-invalid tests ===============" | tee -a output.txt
test "./test/syn-err/*.tpc" 1

echo "============== Sem-invalids tests ===============" | tee -a output.txt
test "./test/sem-err/*.tpc" 2 

echo "============== Warn tests ===============" | tee -a output.txt
test "./test/warn/*.tpc" 0

echo "All tests done."

make clean