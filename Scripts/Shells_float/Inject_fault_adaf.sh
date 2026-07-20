#!/usr/bin/env bash
set +e
# set -x

# Base directory
LINEINFODIR="${1:-"/localdata/Mary/Workspace_2/SDC/Scripts/line_info.sh"}"
OUTPUT="${2:-"/localdata/Mary/Workspace_2/SDC/Scripts/report_FaultList.csv"}"
ADADDIR="${3:-"/localdata/Mary/Workspace_2/SDC/adad/Adad_For_Bash/"}"
SuccessDIR="${4:-"/localdata/Mary/Workspace_2/SDC/Output/Codes_Compiled/"}"
CompiledDIR="${5:-"/localdata/Mary/Workspace_2/SDC/Output/Compiled_Out/"}"
FaultReportDIR="${6:-"/localdata/Mary/Workspace_2/SDC/Output/Fault_Reports/"}"

#empty folders
find -L $CompiledDIR -type f -name "*" -exec rm -f {} +
find -L $FaultReportDIR -type f -name "*" -exec rm -f {} +

if [ -e $OUTPUT ]; then
    mv $OUTPUT $OUTPUT.old
fi

# how long to give each program
TIMEOUT=5

# Write CSV header
echo "CompiledGCCStatus,RunStatus,Runtime,FileName,Path,FileSize,OutLines" > "$OUTPUT"

cd $SuccessDIR
rm -f terminal.txt

# for each C file
for FILE in *.cpp; do
    echo $FILE
    OutName=$CompiledDIR${FILE%.cpp}
    # get number of lines in code
    file_size=$(wc -c < "$FILE")
    set +e
    # try to compile it
    # test: g++ -g -O0 -pedantic -Wall -I/localdata/Mary/Workspace_2/SDC/adad/Adad_For_Bash/ /localdata/Mary/Workspace_2/SDC/Output/Codes_Compiled/100-doors-1.c.cpp /localdata/Mary/Workspace_2/SDC/adad/Adad_For_Bash/main.cpp /localdata/Mary/Workspace_2/SDC/adad/Adad_For_Bash/adad.cpp /localdata/Mary/Workspace_2/SDC/adad/Adad_For_Bash/fault.cpp -lm
    g++ -g -O0 -pedantic -Wall -I${ADADDIR} $SuccessDIR$FILE ${ADADDIR}/main.cpp ${ADADDIR}/adaf.cpp ${ADADDIR}/fault.cpp -lm
    # get the exit status of the program 
    gcc_status=$?
    # turn stopping the shell script on errors back on
    # set -e
    status="N/A"
    compiled="No"
    out_lines=0
    runtime=0

    # was an executable created?
    if [ -e a.out ]; then
        mv a.out $OutName.out
        # turn off stopping the shell script on errors temporarily
        set +e
        # run it with a timeout
        start=$(date +%s.%N)
        echo "$LINEINFODIR $OutName.out $FaultReportDIR"
        # to print to txt: 2>&1 | tee -a terminal.txt
        timeout --preserve-status ${TIMEOUT}s bash $LINEINFODIR $OutName.out $FaultReportDIR 
        # get the exit status of the program 
        status=$?
        end=$(date +%s.%N)
        runtime=$(echo "$end - $start" | bc)
        # turn stopping the shell script on errors back on
        set -e
        # inform
        compiled="Yes"
        # get number of lines in the output
        out_lines=$(cat $OutName.list | wc -l)
        echo "    file $N $FILE compiles and runs producing $out_lines lines and exiting with a status of $status"
    else
        compiled="NO"
        # inform
        echo "    file $N $FILE did not compile, gcc returned $gcc_status"
    fi

    echo -n "\"$compiled\",\"$gcc_status\",\"$status\",\"$runtime\",\"$FILE\",\"$DIR\",\"$file_size\",\"$out_lines\"" >> "$OUTPUT"
    echo "" >> "$OUTPUT"
    rm -f a.out

done

# go back to where we were
cd - > /dev/null
