#!/usr/bin/env bash

set +e

#  Base directory
FIDIR="${1:-"/localdata/Mary/Workspace_2/SDC/Scripts/FI.sh"}"
TRY="${2:-10}"
FALT_PROB="${3:-0.5}"
FaultReportDIR="${4:-"/localdata/Mary/Workspace_2/SDC/Output/Fault_Reports/"}"
FICSV="${5:-"/localdata/Mary/Workspace_2/SDC/Output/FI_Reports/"}"
OUTTXT="${6:-"/localdata/Mary/Workspace_2/SDC/Output/FI_TXT/"}"

if [[ $# -gt 6 ]]; then
    echo "Usage: $0 [input_csv] [TRY] [FICSV] [OUTTXT]"
    exit 1
fi

#empty folders
find -L $FICSV -type f -name "*" -exec rm -f {} +
find -L $OUTTXT -type f -name "*" -exec rm -f {} +

# how long to give each program
TIMEOUT=10

cd $FaultReportDIR
rm -f terminal_out.txt

#NPROC=$(nproc)
#NC=$((NPROC/2))
NC=8

# for each C file
for FILE in *.csv; do
    echo "Processing $FILE"
    # run fault injection script
    #timeout --preserve-status ${TIMEOUT}s 
    # to print to txt: 2>&1 | tee -a terminal_out.txt
    bash $FIDIR $FILE $TRY $FALT_PROB $FICSV $OUTTXT &
    # increment the counter
    N=$((N+1))
    # if we are at the maximum number of jobs
    if [ $N -eq $NC ]; then
        # wait for all the background jobs to finish
        wait
        # reset the counter
        N=0
    fi
    status=$?
    echo "Fault injection for $FILE completed with status: $status"
done

# for FILE in *.csv; do
#     echo "Processing $FILE"
#     # run fault injection script
#     #timeout --preserve-status ${TIMEOUT}s 
#     # to print to txt: 2>&1 | tee -a terminal_out.txt
#     bash $FIDIR $FILE $TRY $FALT_PROB $FICSV $OUTTXT 
#     status=$?
#     echo "Fault injection for $FILE completed with status: $status"
# done

# go back to where we were
cd - > /dev/null
