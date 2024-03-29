// declare or define anything ONLY when needed 

#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ctime>
#include <semaphore.h>

using namespace std;


#define N 2
#define PORT 8787
#define PORT_2 8788
#define BUF_SIZE 64
#define W_Queue 5000
#define MAX_CLIENT_COUNT 5000

#define USERS 50

struct client_infos {
	int sockfd;
};

/*template code for LOG purpose */

template<typename T> 
void LOG(T const& t) { 
	std::cout << t; 
}
template<typename First, typename ... Rest> 
void LOG(First const& first, Rest const& ... rest) {
	std::cout << first<<" "; LOG(rest ...);
}
/*template code for log purpose */

vector<map<int,pair<int,pair<int,int>>>> LIST(MAX_CLIENT_COUNT);

int new_sockfd[MAX_CLIENT_COUNT];
char DELIM = ':';

sem_t pp;

int ITER = 0;
long int display_counter = 0;
long int T = 0;
void *handle_game(void *ptr) {

	client_infos *p = (client_infos *)(ptr);
	char buffer[BUF_SIZE];
	
	int times = 0;
	
	sem_wait(&pp);


	while(true) {

		memset(buffer,0,BUF_SIZE);
		int res = 0;
		int values[3] = {0};
		if((res = recv(p->sockfd,buffer,BUF_SIZE,0)) < 0) {
			perror("\t\tError location 1111:\n");
			exit(0);
		}else {

			//cout << " aaa = "<<res <<" "<<buffer<<endl;
			int num = 0,idx = 0; char c;
			for(int i=0;i<3;i++) {
				while((c = buffer[idx++]) != DELIM) {
					num = num * 10 + (c - '0');
				}
				values[i] = num;
				num = 0;
			}

			

			LIST[values[1]][values[2]].first += 1; // total match
			LIST[values[2]][values[1]].first += 1;// total match
			
			if(values[0] == 1) {
				//printf("1 wins\n");
				LIST[values[1]][values[2]].second.first += 1; // win
				LIST[values[2]][values[1]].second.second += 1; /// lose
			}else if(values[0] == 2) {
				//printf("2 wins\n");
				LIST[values[1]][values[2]].second.second += 1;
				LIST[values[2]][values[1]].second.first += 1;			

			}
			
			
		}

	
		int win_c  = 0;
		int tot_m = 0;
		int lost = 0;
		//printf("%d\n",p1);
		
		for(auto elem : LIST[values[1]]) {	
			tot_m += elem.second.first;
			win_c += elem.second.second.first;
			lost += elem.second.second.second;
		}
		memset(buffer,0,BUF_SIZE);
		sprintf(buffer,"%d:%d:%d:",tot_m,win_c,lost);
		int res1 = 0;
		if(  (res1 = send(p->sockfd,buffer,strlen(buffer),0)) < 0   ) {
			perror("\t\tError location 2222 \n");
			exit(0);
		}else {

			//cout <<" bbb = "<< res1 << endl;
			
		}
		memset(buffer,0,BUF_SIZE);
		bool go = false;
		int res2 = 0;
		if((res2 = recv(p->sockfd,buffer,BUF_SIZE,0)) < 0) {
			perror("\t\tError location 3333:\n");
			exit(0);
		}else {
			//LOG("loc-2 ",res2,"\n");
			if(buffer[0] == 1) go = true;
			if(buffer[0] != 1) cout << " ERRORRRRRRRRRRR "<<endl;
		}
		memset(buffer,0,BUF_SIZE);
		//printf("%d\n",p2);
		tot_m = win_c = lost = 0;
		for(auto elem : LIST[values[2]]) {
			tot_m += elem.second.first;
			win_c += elem.second.second.first;
			lost += elem.second.second.second;
		}
		sprintf(buffer,"%d:%d:%d:",tot_m,win_c,lost);
		if(go) {
			int res3 = 0;
			if(  (res3 = send(p->sockfd,buffer,strlen(buffer),0) )< 0) {
				perror("\t\tError location 4444\n");
				exit(0);
			}else {
				//cout <<" ccc = "<< res3<< endl;
				
			}
		}

		// display_counter++;
		// if(display_counter >= 1000) {
		// 	T += display_counter;
		// 	cout << T << endl;
		// 	display_counter = 0;
		// }
		sem_post(&pp);
	
		times++;
		if(times >= ITER) break;
	}
	//free(buffer);
	free(ptr);
	close(p->sockfd);
	pthread_exit(NULL);
}




int main(int argc,char **argv) {

	sem_init(&pp,0,1);

	ITER = atoi(argv[1]);

	printf("iter = %d\n",ITER);

	int sockfd_listen = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd_listen < 0) {
		perror("Error creating listening socket \n");
		exit(1);
	} else { 
		printf("Listening socket created in the server-2 side\n");	
	}

	int yes=1;
	if (setsockopt(sockfd_listen, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
		perror("setsockopt : ");
		exit(1);
	}

	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = PORT_2;
	if (bind(sockfd_listen, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("\t\tError: \n");
		exit(1);
	}else printf("\t\tBinding done with PORT : %d\n",PORT_2);
	printf("\t\tWaiting for a connection from server_1 side\n");
	listen(sockfd_listen, W_Queue);
	struct sockaddr_in cl_info_addr;
	int len = sizeof(cl_info_addr);
	int client_count = 0;
	bool LISTEN_FLAG_SERVER = true;

	pthread_t thread_pool[10000];
	
	int thread_count = 0;
	char buffer[BUF_SIZE];
	
	memset(buffer,0,BUF_SIZE);
	while(LISTEN_FLAG_SERVER) {



		new_sockfd[client_count] = accept(sockfd_listen, (struct sockaddr *) &cl_info_addr, (socklen_t *)&len);
		if(new_sockfd[client_count] < 0) {
			printf("Error accepting connection!\n");
			exit(1);
		}else {
			char clientAddr[INET_ADDRSTRLEN];
			inet_ntop(AF_INET,&(cl_info_addr.sin_addr),clientAddr,INET_ADDRSTRLEN);
			//printf("\t\tConnection accepted from server_1 side : %s\n", clientAddr); 	
		}
		client_infos *ptr = (client_infos *)malloc(sizeof(client_infos));
		ptr->sockfd = new_sockfd[client_count];
		if(pthread_create(&thread_pool[thread_count], NULL,handle_game, (void *)(ptr)) ) {
			perror("\t\tERROR: pthread failed inside server_1 exiting now !!\n");
			exit(1);
		}else {
			thread_count++;
			cout <<" Backend thread created "<<thread_count<<endl;
		}
		 if(pthread_detach(thread_pool[thread_count-1])){
			perror("\t\tERROR: pthread_detach failed inside server_1\n");
			exit(1); // try something else...not to completely end this server process
		}else {

		}

		client_count++;
		
		//if(client_count == MAX_CLIENT_COUNT) LISTEN_FLAG_SERVER = false;
		
		// if(client_count >= USERS) LISTEN_FLAG_SERVER = false;
	
	}


	// int cnt = 0;
	// for(int i=0;i<thread_count;i++) {
	// 	pthread_join(thread_pool[i],NULL);
	// 	cout <<" joined = "<<++cnt<<endl;
	// }
	
	return 0;
}