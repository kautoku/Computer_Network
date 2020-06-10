 
第三方支付使用者對使用者小額付款系統
======
網路技術與應用 作業
-----
內容：實作server與多個clients，使用socket聯繫
此系統包含三大功能。
> 1. 第三方支付Server端對Client端（使用者）的統一管理，包含帳號管理、認證以及Client帳戶管理等。
> 2. Client間即時通訊
> 3. Client與Server以及Client間的通訊


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
