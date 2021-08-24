Syntax for 'repeat':

# repeat N COMMAND

	where N is the amount of times any mysh-supported COMMAND is executed.
	
	ex.
		
		# repeat 3 whereami
		/home
		/home
		/home
		# 
		
		# repeat 5 background /usr/bin/xterm -bg blue
		PID: 1
		PID: 2
		PID: 3
		PID: 4
		PID: 5
		# 

Syntax for 'exterminateall':

# exterminateall

	ex.
		
		# exterminateall
		Murdering 5 processes: 5, 4, 3, 2, 1
		# 