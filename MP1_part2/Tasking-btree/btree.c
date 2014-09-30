#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#define SEED 0

struct tree{
	long int key;
	struct tree *left;
	struct tree *right;
	long int id;
};

void insert(struct tree *ptr, struct tree *p);
void myparent(struct tree *ptr, long int myid, struct tree **parent);
void free_mem(struct tree *p);
struct timespec start_time,end_time,slptime;
long int elapsed1,elapsed2=0,elapsed3,sleep_time;
double speedup;
struct tree *root,*p;
void search_tree(struct tree *p,long int slp);

int main(int argc, char *argv[])
{
	long int num,i=1;
	int threads;
    
	// read arguments
	if(argc!=4) { 
		printf("Invalid arguments"); 
		exit(1); 
	}
	threads=atoi(argv[1]);
	num=atol(argv[2]);   
    sleep_time=atol(argv[3]);
    
	// init data structure
	srand(SEED);
    root = (struct tree*) malloc(sizeof(struct tree));
	if(root==NULL) { 
		printf("Cannot allocate memory"); 
		exit(1); 
	}
	root->key = (long int) 1000000 * ((double) rand() / ((double) (RAND_MAX) + (double) (1)));
    root->left=NULL;
    root->right=NULL;
    root->id=i;
    i++;
	
	while(i<=num) {
		p = (struct tree*)malloc(sizeof(struct tree));
		if (p==NULL) { 
			printf("Cannot allocate memory"); 
			exit(1); 
		}
		p->key = (long int) 1000000 * ((double) rand() / ((double) (RAND_MAX) + (double) (1)));
		p->left=NULL;
		p->right=NULL;
		p->id=i++;
		insert(root,p); 
	}
   
	// start traversal sequentially (or in parallel)
	clock_gettime(CLOCK_MONOTONIC,&start_time);
    search_tree(root,sleep_time);
	clock_gettime(CLOCK_MONOTONIC,&end_time);
	elapsed1=((((end_time.tv_sec-start_time.tv_sec)*1000000000)+end_time.tv_nsec-start_time.tv_nsec)/num);
	printf("%ld,%d,%ld,%ld\n",num,threads,elapsed1,sleep_time);
   
	free_mem(root);
	return 0;
}   

void myparent(struct tree *ptr, long int myid, struct tree **parent) 
{
	if (ptr->id==(myid/2)) {
		*parent=ptr; 
		return;
	}
	if (ptr->left) 
		myparent(ptr->left,myid,parent);
	if (ptr->right) 
		myparent(ptr->right,myid,parent);
	return;
}

void insert(struct tree *ptr, struct tree *p)
{
	struct tree *parent;
	myparent(ptr,p->id,&parent);
	if ((p->id)%2) 
		parent->right=p;
	else 
		parent->left=p;
	return;
}

void free_mem(struct tree *p)
{
	if(p!=NULL) {
		free_mem(p->left);
		free_mem(p->right);
		free(p);
	}
	return;
}

// search_tree traverses the entire tree with depth-first search strategy
void search_tree(struct tree *p, long int slp)
{
	int i;
	if(p==NULL) 
		return;
	for(i=0;i<slp/4;i++) ; // iterate for slp/4 number of times
	
	// search both children
	search_tree(p->left,slp); 
	search_tree(p->right,slp); 
	
	return;
}
