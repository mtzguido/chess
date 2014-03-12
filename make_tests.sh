rm -f FINISHLOG
n=0
total=100

rm -f wpipe bpipe
mkfifo wpipe bpipe

while [ $n -lt $total ]; do
	n=$((n+1))
	( fairymax <wpipe >bpipe ) & disown
	pid=$!

	./chess w 2>&1 >wpipe <bpipe | tee full_log | grep -E '^RES:' | tee -a FINISHLOG

	kill -9 $pid

	cp gamelog_w gamelog_$n
	echo "$n/$total games"
done

