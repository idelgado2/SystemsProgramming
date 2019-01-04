To implement these programs correctly, You must execute the program in the following order: 	1) ping.c 
	2) pong.c
	3) pingpong.c

ping.c and pong.c should be run in the background as well. Once the you execute the last program (pingpong.c) all the output will print. To Terminate the program use Command-C or anything equivalent to send a manual Signal interrupt. This will cause the other programs (ping.c & pong.c) to terminate as well so there will be no need to manually go kill those processes.

Example Implementation:
$ ./ping &
[1] 2729
$ ./pong &
[2] 2731
$ ./pingpong
Ping
Pong
Ping
Pong
.
.
.
Ping
Pong
^C
Failed to send SIGUSR1 to pingpong or pingpong process was interrupted: No such process
[1]-  Interrupt               ./ping
[2]+  Interrupt               ./pong
