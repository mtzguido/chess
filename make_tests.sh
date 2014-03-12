rm -f SCORELOG FINISHLOG
n=0
total=100

rm -f wpipe bpipe
mkfifo wpipe bpipe

while [ $n -lt $total ]; do
	n=$((n+1))
	./fairy.sh <wpipe >bpipe & 
	pid=$!

	./chess w 2>&1 >wpipe <bpipe | tee full_log | grep -E '^RES:' | tee -a FINISHLOG

	kill $pid

	cp gamelog_w gamelog_$n
	echo "$n/$total games"
done

