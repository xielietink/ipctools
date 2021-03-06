#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>

#include "allocator_shm.h"
#include "config.h"
#include "reactor.h"
#include "shared_queue.h"
#include "shared_in_list.h"

ipt_shared_queue_t *sq_ptr;
ipt_allocator_t *alloc_ptr;
struct my_message
{
	ipt_shared_queue_node_t node;
	char buf[256];
};

static void
test_1(void)
{

   	unsigned int count = 0;
   	struct my_message *ptr;
	int outer_loop = 0;

	/* Loop through 100 times. Stress the allocator and shared queue system. */
	while ( outer_loop++ < 100 )
	{
   		while (1)
   		{
     			ptr = (struct my_message *)alloc_ptr->malloc(alloc_ptr, sizeof(struct my_message));

      			if ( ptr == NULL )
      			{
         			break;
      			}

      			strcpy(ptr->buf,"123456789");

      			sq_ptr->enqueue(sq_ptr, (ipt_shared_queue_node_t *)ptr);
   
      			count++;

   		}

   		ipt_time_value_t tv = { 0, 100000 };

   		while (1)
   		{
         		ptr = (struct my_message *)sq_ptr->dequeue_timed(sq_ptr, &tv);

         		if ( ptr == NULL )
         		{
           			 break;
         		}

         		count--; 

         		assert ( !strcmp(ptr->buf,"123456789") );

         		alloc_ptr->free(alloc_ptr, (void *)ptr);

   		}

   		assert ( count == 0 );
	}
}

static void 
test_2()
{

	if ( fork() == 0 )	
   	{
      		ipt_time_value_t tv = {1,0};
      		int count=0;

      		while (1) 
      		{
         		void *ptr = sq_ptr->dequeue_timed(sq_ptr, &tv);
         		if (ptr == NULL )
         		{
            			break;
         		}
         		count ++;
      		}

      		assert( count == 10 );

      		exit( count );
	}
	else
	{
		int i =0;
      		int status = 0;

      		for (i=0; i < 10;i++)
      		{
         		struct my_message *ptr = (struct my_message * ) alloc_ptr->malloc(alloc_ptr,sizeof(struct my_message));

         		sprintf(ptr->buf,"message %d from parent",i);

         		sq_ptr->enqueue(sq_ptr, (ipt_shared_queue_node_t *)ptr);

		}

      		wait(&status);

      		assert( WEXITSTATUS(status) == 10 );
   	}
}
int main(int argc , char *argv[])
{
	alloc_ptr = ipt_allocator_shm_create(1024 *1024, IPT_TEST_ALLOCATOR_SHM_KEY);

	if ( alloc_ptr == NULL )
	{
		printf("Failed to create shared allocator \n");
		return -1;
	}

	sq_ptr = ipt_shared_queue_create("test_queue",alloc_ptr);

	if ( sq_ptr == NULL )
	{
		printf("Failed to create shared queue.\n");
		return -1;
	}

   	test_1();

	test_2();

	printf("%s completed successfully.\n",argv[0]);

	return 0;
}
