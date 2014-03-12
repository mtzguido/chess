cat fairy_head - | fairymax | sed -une 's/move \([a-z][0-9][a-z][0-9]\)/\1/p
s/^1\/2-1\/2.*/&/p'
