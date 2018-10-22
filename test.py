import socket # for socket 
import binascii
import sys  
from time import sleep
import random
import multiprocessing as mp
import random
import string


# a function I found that reverses bits
def reverse_bits( x ):
  size = 8 
  y = 0
  position = size - 1
  while position > 0:
    y += ( ( x & 1 ) << position )
    x >>= 1
    position -= 1
 
  return y

# this keeps the reversals in the right range for reversing
def send_strings( i ):
	# a small test array
	my_bytes = bytearray()
	for j in range(125):
		my_bytes.append(random.randint(0,127))
	try: 
	    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
	except socket.error as err: 
	    print "socket creation failed with error %s" %(err) 
	  
	
	# default port for socket 
	port = 8081
	  
	# assume that server code on the same host
	host_ip = socket.gethostbyname('localhost')
	  
	# connecting to the server 
	s.connect((host_ip, port)) 
	# send all the bytes
	s.sendall(my_bytes);
	  
	# collect the data
	data = s.recv(2048)
	# put into a byte array for comparasion
	ret_bytes = bytearray(data)
	# check to see if the first element is the reverse of the last element
	if (hex(ret_bytes[0]) == hex(reverse_bits(my_bytes[len(my_bytes)-1]))):
		print "Test " + str(i) + " Passed"
	else:
		print "Test " + str(i) + " Failed"
	s.close()

pool = mp.Pool(processes=10)
[pool.apply(send_strings, args=(x,)) for x in range(1,10000)]
