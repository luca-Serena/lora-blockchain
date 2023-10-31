###############################################################################################
#	Advanced RTI System, ARTÌS			http://pads.cs.unibo.it
#	Large Unstructured NEtwork Simulator (LUNES)
################################################################################################

#########################################################
#	CONFIGURATION FILES USED BY SCRIPTS
#########################################################
#
#	Common prefix for the output directories
PREFIX_DIRECTORY=/home/SenecaUrla/Downloads/ARTIS-2.1.2-x86_64/MODELS/LUNES-lora-blockchain
#
RESULTS_DIRECTORY=$PREFIX_DIRECTORY/results
#
mkdir -p  $RESULTS_DIRECTORY
#
#########################################################
#	EXECUTION VALUES
#########################################################
#
#	Number of working threads to be run for log processing
SOCKETS=`lscpu | grep "Socket(s)" | cut -d":" -f2 | cut -f14 -d" "`
COREPERSOCKET=`lscpu | grep "Core(s) per socket" | cut -d":" -f2 | cut -f5 -d" "`
CPUNUM=`echo $SOCKETS \\* $COREPERSOCKET | bc`
#
#	Reduce the I/O priority of stats workers
IONICE='ionice -c 3'
#
#	Number of LPs to be used:
#		1 = monolithic (sequential) simulation
#		> 1 parallel simulation
LPS=1
#
#	Turn on the simulation execution
EXECUTION=1
#
#	Delete all trace files after statistics calculation
NOTRACE=1
#
#	Number of time-steps in each simulation run
#	(after the building phase of the network is completed)
export END_CLOCK=1000

#########################################################
# File names definition, used for statistics purposes
#########################################################
#
#	output files
#
DELAYS="STAT_msg_delays.txt"
MEAN="STAT_mean_delay.txt"
MSGIDS="STAT_msg_ids.txt"
OUTPUT="STAT_coverage.txt"
DISTRIBUTION="STAT_missing_distribution.txt"
#
#	temporary files
#
RUNSCOVERAGEMEANTMP="STAT_tmp_runs_coverage_mean.txt"
RUNSCOVERAGETMP="STAT_tmp_runs_coverage.txt"
RUNSDELAYMEANTMP="STAT_tmp_runs_delay_mean.txt"
RUNSDELAYTMP="STAT_tmp_runs_delay.txt"
TMP="STAT_tmp_coverage.txt"
RUNSMESSAGETMP="STAT_tmp_runs_messages.txt"
RUNSMESSAGEMEANTMP="STAT_tmp_runs_messages_mean.txt"

