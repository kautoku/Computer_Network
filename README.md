
* compile file  
`$ make`  
or  

* compile server 端   
`$ gcc -c server.c -lpthread`   
`$ gcc server.o -o server -lpthread`  
* compile client 端    
`$ gcc -c client.c -lpthread`
`$ gcc client.o -o client -lpthread`

* executable client  
`$ ./client`   

* executable server    
`$ ./server`   

  
* environment:  
ubuntu 18.04    
gcc version 7.4.0 (Ubuntu 7.4.0-1ubuntu1~18.04.1)   
GNU Make 4.1    
