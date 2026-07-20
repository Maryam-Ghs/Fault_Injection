#!/usr/bin/env bash

# analyze_int_adad.sh
#/localdata/Mary/Workspace_2/SHOUT_Float/Code_source/SDC_Dataset/
BASE_DIR="/localdata/Mary/Workspace_2/SDC/"
SOURCE_DIR=$BASE_DIR"Code_source/test/test_float/"
EXT=".cpp"
OUTPUT=$BASE_DIR"Scripts/report_float_adad.csv"
ADADDIR=$BASE_DIR"adad/Adad_For_Bash/"
PAINDIR=$BASE_DIR"Output/Pain_Funcs/"
SuccessDIR=$BASE_DIR"Output/Codes_Compiled/"
SuccessOrigDIR=$BASE_DIR"Output/Codes_Orig/"

mkdir -p $PAINDIR
mkdir -p $SuccessDIR
mkdir -p $SuccessOrigDIR

echo "Starting analysis and inverting int to adad"
bash Shells_float/analyze_float_adaf_p.sh $SOURCE_DIR $EXT $OUTPUT $ADADDIR $PAINDIR $SuccessDIR $SuccessOrigDIR 2>&1 | tee Run_log_p1.log
echo "Completed analysis of int adad"

##### Inject_fault_adad.sh
LINEINFODIR=$BASE_DIR"Scripts/Shells_float/line_info.sh"
OUTPUT=$BASE_DIR"Scripts/report_FaultList.csv"
ADADDIR=$BASE_DIR"adad/Adad_For_Bash/"
SuccessDIR=$BASE_DIR"Output/Codes_Compiled/"
CompiledDIR=$BASE_DIR"Output/Compiled_Out/"
FaultReportDIR=$BASE_DIR"Output/Fault_Reports/"

mkdir -p $CompiledDIR
mkdir -p $FaultReportDIR

echo "Starting getting fault injection locations"
bash Shells_float/Inject_fault_adaf_p.sh $LINEINFODIR $OUTPUT $ADADDIR $SuccessDIR $CompiledDIR $FaultReportDIR 2>&1 | tee Run_log_p2.log
echo "Completed fault injection"


##### Gen_Fault_Report.sh
FIDIR=$BASE_DIR"Scripts/Shells_float/FI_counts_random.sh"
TRY=100
FALT_PROB=1.0
FaultReportDIR=$BASE_DIR"Output/Fault_Reports/"
FICSV=$BASE_DIR"Output/FI_Reports/"
OUTTXT=$BASE_DIR"Output/FI_TXT/"

mkdir -p $FICSV
mkdir -p $OUTTXT

echo "Starting generation of fault injection reports"
bash Shells_float/Gen_Fault_Report.sh $FIDIR $TRY $FALT_PROB $FaultReportDIR $FICSV $OUTTXT 2>&1 | tee Run_log_p3.log
echo "Completed generation of fault injection reports"
#find -L $OUTTXT -type f -name "*" -exec rm -f {} +
echo "Done"