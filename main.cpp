#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <queue>
#include <vector>
#include <semaphore.h>
#include <sstream>
#include <sys/stat.h>
#include <fstream>
#include <errno.h>
#include <vector>
#include <dirent.h>

using namespace std;
sem_t mutexqueue;int connection;
int number_of_threads=4;
pthread_mutex_t mutex;
char RootPath[100];
char loggingpath[200];const int no_of_t=100;
int debug,port,Time,logging,sched; char *dir,*logfile;bool busy;

class request         // Data Structure for the incoming requests contained in the ready queue
{
	public:
	int type,status,ip;
	char *IncomingTime, *ScheduleTime,*buffers;

	char *filename;
	string path;
	char *request_method;
	int filesize, connection;
	char *LastModifiedTime;
};
request incoming_request,debug_request;
class compareRequest {
    public:
        bool operator()(request &r1, request &r2)
        {
              
	      return r1.filesize > r2.filesize;
        }	
};

queue<request> readyQueue;
priority_queue<request,vector<request>,compareRequest> readyQueueP;


class socketprog               

{
	public:
	int socket_id; 
        struct sockaddr_in server,client;


		void initSocketprog()   // creates a socket and binds it to a port. note that it is a constructor function
		{				
			if((socket_id=socket(AF_INET,SOCK_STREAM,0))==-1)	
			{
				std::cout<<"socket error!\n";
				close(socket_id);
				exit(1);
			}		
			server.sin_family=AF_INET;
			server.sin_addr.s_addr=0; 
			server.sin_addr.s_addr = INADDR_ANY;
			server.sin_port = htons(port);
			bzero(&(server.sin_zero),8);

			if(bind(socket_id, (struct sockaddr *)&server, sizeof(struct sockaddr))==-1)
			{
				std::cout<<"Binding failed!";
				close(socket_id);
				exit(1);
			}
			
			if(listen(socket_id, 10)==-1)
			{
				std::cout<<"Listening failed!"; 
				close(socket_id);
				exit(1);
			}
				
			

		}
		
		~socketprog()
		{
			
		}
		

};
class thread               // class for all threads. Note that the constructor assings false as soon as a thread is created. Until the thread goes 
                           //to  process, the variable assigned will be false
{
	public:

        bool assigned;
	pthread_t idx;
	pthread_attr_t attributes;
	thread()
		{	
			assigned=false;
			pthread_attr_init(&attributes);
		}
	
};

socketprog current_socket;  
thread listening_thread, scheduling_thread;   
int signal_thread[no_of_t];
request process_array[no_of_t];
thread thread_pool[no_of_t];


void debugging_request()                 
{	

	
	        const char* c; 
		char *CurrentTime,CurrentPath[1024],LastModifiedTime[40]; time_t t;char header[1024],line[1024];FILE *f,*tmp;
		int fsize;
		memset(header,'\0', sizeof(header));
		ifstream outfile;
		stringstream filename;
		strcpy(CurrentPath, RootPath);
		time_t st;
		time(&st);
		debug_request.ScheduleTime=asctime(gmtime(&t));
		
		if(debug_request.status==404)
		{	
			time(&t);				
			CurrentTime=asctime(gmtime(&t));
			sprintf(header,"HTTP/1.1 404 FILE NOT FOUND\nDate: %sServer: FIFA\r\n\r\n",CurrentTime);
			write(debug_request.connection,header,strlen(header));		
		}
		else
		{
		switch(debug_request.type)
		{
			case 1: time(&t);				
				CurrentTime=asctime(gmtime(&t));



				



				

				for (int i = 0; i < (debug_request.path).length(); i++)
        			{
            				if ( debug_request.path[i] == '\n' || debug_request.path[i] == '\r' )
                				debug_request.path[i] = '\0';
        			}	
                                
                                c = (debug_request.path).c_str();
				strcat(CurrentPath,c);

 				tmp = fopen(CurrentPath,"r");
				if(tmp != NULL)
				{	
					fseek(tmp,0,SEEK_END);
					fsize=ftell(tmp);
				}
				fclose(tmp);


				sprintf(header,"HTTP/1.1 200 OK\nDate: %sServer: FIFA\nLast-Modified: %sContent-Type: text/html\nContent-Length: %d\r\n",CurrentTime,LastModifiedTime,debug_request.filesize);
				write(debug_request.connection,header,strlen(header));

					
				
				

								
				f=fopen(CurrentPath,"r");
				if (f != NULL) 
				{       
					while (fgets(line,sizeof(line), f ) != NULL )
					{
					write(debug_request.connection,line,strlen(line));

					}
				fclose(f);
				}
				else
				{
					cout<<strerror(errno);
				}				

				memset(line,'\0', sizeof(line));

				break;
			case 2: 
				
				time(&t);				
				CurrentTime=asctime(gmtime(&t));
				
				for (int i = 0; i < (debug_request.path).length(); i++)
        			{
            				if ( debug_request.path[i] == '\n' || debug_request.path[i] == '\r' )
                				debug_request.path[i] = '\0';
        			}	
                                c = (debug_request.path).c_str();

                                strcat(CurrentPath,c);
				tmp = fopen(CurrentPath,"r");
				if(tmp != NULL)
				{	
					fseek(tmp,0,SEEK_END);
					debug_request.filesize=ftell(tmp);
				}
				fclose(tmp);

				sprintf(header,"HTTP/1.1 200 OK\nDate: %sServer: FIFA\nLast-Modified: %sContent-Type: text/html\nContent-Length: %d\r\n",CurrentTime,LastModifiedTime,debug_request.filesize);
				write(debug_request.connection,header,strlen(header));
                                break; 
				
			default: write(debug_request.connection, "Invalid Request! Only HTTP GET and HEAD are served",sizeof("Invalid Request! Only HTTP GET and HEAD are served here!"));break;
		}
		}		
		printf("%d [%s] [%s] \"%s\" %d %d\n",debug_request.ip,debug_request.IncomingTime,debug_request.ScheduleTime, debug_request.buffers,debug_request.status,debug_request.filesize);
		
		close(debug_request.connection);
	
}

void* serve_request(void *ptr)                 
{	
        const char* d;

	int *index;
	index=(int*) ptr;
	while(1)
	{  
		char *CurrentTime,CurrentPath[1024],LastModifiedTime[40]; time_t t;char header[1024],line[1024];FILE *f,*tmp;
		memset(header,'\0', sizeof(header));
		ifstream outfile;
		stringstream filename;
		strcpy(CurrentPath, RootPath);
		while(signal_thread[*index]==0);
		if(process_array[*index].status == 404)
		{
			time(&t);				
			CurrentTime=asctime(gmtime(&t));
			sprintf(header,"HTTP/1.1 404 FILE NOT FOUND\nDate: %sServer: FIFA\r\n\r\n",CurrentTime);
			write(process_array[*index].connection,header,strlen(header));		
		}
		else
		{
		switch(process_array[*index].type)
		{
			case 1:  
				
				
				time(&t);				
				CurrentTime=asctime(gmtime(&t));
				
				for (int i = 0; i < (process_array[*index].path).length(); i++)
        			{
            				if ( process_array[*index].path[i] == '\n' || process_array[*index].path[i] == '\r' )
                				process_array[*index].path[i] = '\0';
        			}	
                                d = (process_array[*index].path).c_str();

                                strcat(CurrentPath,d);
 				sprintf(header,"HTTP/1.1 200 OK\nDate: %sServer: FIFA\nLast-Modified: %sContent-Type: text/html\nContent-Length: %d\r\n\r\n",CurrentTime,process_array[*index].LastModifiedTime,process_array[*index].filesize);
				write(process_array[*index].connection,header,strlen(header));

				
				

								
				f=fopen(CurrentPath,"r");
				if (f != NULL) 
				{      
					while (fgets(line,sizeof(line), f ) != NULL )
					{
					write(process_array[*index].connection,line,strlen(line));

					}
				fclose(f);
				}
				else
				{
					cout<<strerror(errno);
				}				

				memset(line,'\0', sizeof(line));

				break;
			case 2: 
				
				time(&t);				
				CurrentTime=asctime(gmtime(&t));
				
				for (int i = 0; i < (process_array[*index].path).length(); i++)
        			{
            				if ( process_array[*index].path[i] == '\n' || process_array[*index].path[i] == '\r' )
                				process_array[*index].path[i] = '\0';
        			}	
                                d = (process_array[*index].path).c_str();

                                strcat(CurrentPath,d);
				tmp = fopen(CurrentPath,"r");
				if(tmp != NULL)
				{	
					fseek(tmp,0,SEEK_END);
					process_array[*index].filesize=ftell(tmp);
				}
				fclose(tmp);
 				sprintf(header,"HTTP/1.1 200 OK\nDate: %sServer: FIFA\nLast-Modified: %sContent-Type: text/html\nContent-Length: %d\r\n\r\n",CurrentTime,process_array[*index].LastModifiedTime,process_array[*index].filesize);
				write(process_array[*index].connection,header,strlen(header));

                                break; 
				
			default: write(process_array[*index].connection, "Invalid Request! Only HTTP GET and HEAD are served",sizeof("Invalid Request! Only HTTP GET and HEAD are served here!"));break; 
		}
		}
		close(process_array[*index].connection);
		if(logging==true)
		{
			

			tmp = fopen(loggingpath,"a");
			if(tmp==NULL)
			{
				write(process_array[*index].connection, "Logfile Invalid",sizeof("Logfile Invalid"));
			}
			fprintf(tmp,"%d [%s] [%s] \"%s\" %d %d\n",process_array[*index].ip,process_array[*index].IncomingTime,process_array[*index].ScheduleTime,strtok((process_array[*index].buffers),"\n\n"),process_array[*index].status,process_array[*index].filesize);
			fclose(tmp);			
		}
		
		signal_thread[*index]=0;thread_pool[*index].assigned=false;busy=false;
	}
}

void* listen_t(void *ptr)                    
                                             
{	sem_init(&mutexqueue,0,1);
	
	int n;
	socklen_t len;
	char *method, buff[1024];
	len=sizeof(current_socket.client);
	time_t it;

	
	incoming_request.type=0;
	while(1)
	{			
		
		char RequestPath[100]; FILE *tmp;struct stat attr;	
		incoming_request.connection=accept(current_socket.socket_id,(struct sockaddr *)& (current_socket.client),&len); 
		n=read(incoming_request.connection,buff,1024);

		char *z = buff;
		incoming_request.buffers = z;

		method=strtok(buff," ");
		incoming_request.path=strtok(NULL," ");
        	incoming_request.request_method=strtok(NULL,"\n");
		if(strcmp(method,"GET")==0)
			incoming_request.type=1;
		else if(strcmp(method,"HEAD")==0)
			incoming_request.type=2;
		else
			incoming_request.type=3;

		incoming_request.ip= (&current_socket.client)->sin_addr.s_addr;
		for (int i = 0; i < (incoming_request.path).length(); i++)
        	{
            		if (incoming_request.path[i] == '\n' || incoming_request.path[i] == '\r' )
                		incoming_request.path[i] = '\0';
        	}	
		strcpy(RequestPath,RootPath);
                const char* e = (incoming_request.path).c_str();
                strcat(RequestPath,e);
 		tmp = fopen(RequestPath,"r");
		if(tmp != NULL)
		{	
			fseek(tmp,0,SEEK_END);
			incoming_request.filesize=ftell(tmp);
			fclose(tmp);		
			incoming_request.status = 200;		
		}
		else
		{
			incoming_request.status = 404;incoming_request.filesize=0;					
		}				
		if(strcmp(method,"HEAD")==0)
		{
			incoming_request.filesize=0;
		}			
		time(&it);				
		stat(RequestPath, &attr);		
		incoming_request.LastModifiedTime=asctime(gmtime(&attr.st_mtime));
		incoming_request.IncomingTime=asctime(gmtime(&it));


				
		
		if(debug==1)
		{
			debug_request = incoming_request;			
			debugging_request();
		}

		else
		{

		
			sem_wait(&mutexqueue);
                        if (sched == 0) {
			readyQueue.push(incoming_request); 
                        }
                        else {
			readyQueueP.push(incoming_request); 
                        }
			sem_post(&mutexqueue);

		}
			       	
	}							
} 

void* schedule(void *ptr)   
{      
	sleep(Time);      

	time_t st;
	request process_request;  
		
	while(1)
	{		

		sem_wait(&mutexqueue);

		if(! readyQueue.empty() || ! readyQueueP.empty())
		{	

			
			if (sched == 0){
			process_request=readyQueue.front(); 
			readyQueue.pop(); 
                        }
                        else {
			process_request=readyQueueP.top(); 
			readyQueueP.pop(); 
                        }
			sem_post(&mutexqueue);
			while(busy==true);			
			for(int i=0;i<number_of_threads;i++)
			{
				if(thread_pool[i].assigned==false)
				{	thread_pool[i].assigned=true;
					if(i==number_of_threads-1)
						busy=true;
					process_array[i]=process_request;
					time(&st);				
					process_array[i].ScheduleTime=asctime(gmtime(&st));
					signal_thread[i]=1;					
					break;
				}
			}		
		}
		else
		{

			sem_post(&mutexqueue);
		}

			
	}
  
}


	


int main(int argc, char **argv)

{
	
	//Default
	debug=0;Time=60;sched=0;logging=0;port=8080;
	
		
		
	//Command Line Parsing

	for(int i=1;i<argc;i++)
        {

                if(!strcmp(argv[i],"-d"))
		{
                        debug=1;
                }
                if(!strcmp(argv[i],"-h"))
                {
                        cout<<"myhttpd" << endl << "usage: ./myhttpd [-dh]"
                        << " [-l file] [-p port] [-r dir] [-t time] "
                        << "[-n threadnum] [-s sched]" << endl << endl
                        << "  -d  : Enter debugging mode." << endl
                        << "  −h  : Print a usage summary with all options and exit." 
                        << endl
                        << "  −l file       : Log all requests to the given file."
                        << endl 
                        << "  −p port       : Listen on the given port. If not provided,"
                        << endl << "                  myhttpd will listen on port 8080."
                        << endl
                        << "  −r dir        : Set the root directory for the http server"
                        << endl << "                  to dir."
                        << endl
                        << "  −t time       : Set the queuing time to time seconds." 
                        << endl << "                  The default is 60 seconds."
                        << endl
                        << "  −n threadnum  : Set number of threads waiting ready in the"
                        << endl << "                  execution thread pool to threadnum."
                        << endl << "                  The default is 4 execution threads."
                        << endl 
                        << "  −s sched      : Set the scheduling policy. It can be either"
                        << endl << "                  FCFS or SJF. The default will be FCFS"
                        << endl;
                        exit(0);
                }
                if(!strcmp(argv[i],"-l"))
                {
                        logfile = argv[i+1];
			strcpy(loggingpath,RootPath);
			strcat(loggingpath,logfile);
                        logging = 1;
                }
                if(!strcmp(argv[i],"-p"))
                {
                        port=atoi(argv[i+1]); 
                }
                if(!strcmp(argv[i],"-r"))
                {
                        strcpy(RootPath,argv[i+1]);cout<<RootPath;
                }
                if(!strcmp(argv[i],"-t")) {
                        Time = atoi(argv[i+1]);
                 }
                if(!strcmp(argv[i],"-n")){
                        number_of_threads = atoi(argv[i+1]);
                }
                if(!strcmp(argv[i],"-s"))
                {
                        if(!strcmp(argv[i+1],"SJF")){
                        sched = 1;
                        } 
                        
                }
        }

	current_socket.initSocketprog();
	if(debug==1)
	{
		listen_t(NULL);
	}

	else
	{	

	int pid;
	pid=fork();
	if(pid==0)
	{	
	
	pthread_create(&(listening_thread.idx),&(listening_thread.attributes),listen_t,NULL); 
        pthread_create(&(scheduling_thread.idx),&(scheduling_thread.attributes), schedule,NULL);
	int arg[number_of_threads];
	for(int i=0;i<number_of_threads;i++)
	{ 
		thread_pool[i].idx=i;
		arg[i]=i;
		pthread_create(&(thread_pool[i].idx),&(thread_pool[i].attributes), serve_request, &(arg[i]));
	}
	for(int i=0;i<number_of_threads;i++)
	{
		pthread_join(thread_pool[i].idx, NULL);
	}
	pthread_join(listening_thread.idx, NULL);
	pthread_join(scheduling_thread.idx, NULL);   
	return 0;
	}
	else
	{
	exit(0);
	}

	}    

}
