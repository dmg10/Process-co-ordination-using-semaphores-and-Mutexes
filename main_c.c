/*  main.c  - main */

#include <xinu.h>
#define BUFFER_SIZE 40
pid32 producer_id;
pid32 consumer_id;
pid32 timer_id;

int32 consumed_count = 0;
const int32 CONSUMED_MAX = 100;


/* */
sid32 produced, consumed;
int a[BUFFER_SIZE];
int head, tail;


void mutex_acquire(sid32 mutex)
{
	/* */
	wait(mutex);
}


void mutex_release(sid32 mutex)
{
	/* */
	signal(mutex);
}


process producer(void)
{
	/* */
    int n= 0;
    while(1) 
    {
        n++;
		mutex_acquire(consumed);           
		a[head++] = n;          /* enqueue(n) */
		if (head >= BUFFER_SIZE)          
			head = 0;                    
        mutex_release(produced);
		kprintf("Number produced %d\n", n);            
    }     
	return OK;
}


process consumer(void)
{
	
	int x;
	while(1)
	{
		mutex_acquire(produced);                      
		x = a[tail++];          /* x = dequeue() */
		if (tail >= BUFFER_SIZE)          
		    tail = 0;            
		consumed_count += 1;
		mutex_release(consumed);  
		kprintf("Number consumed %d\n", x);        	
	}

	return OK;
}



process time_and_end(void)
{
	
	int32 times[5];
	int32 i;
	kprintf("Entered time and end\n");
	for (i = 0; i < 5; ++i)
	{
		times[i] = clktime_ms;
		yield();

		consumed_count = 0;
		while (consumed_count < CONSUMED_MAX * (i+1))
		{
			yield();
		}

		times[i] = clktime_ms - times[i];
	}

	kill(producer_id);
	kill(consumer_id);

	for (i = 0; i < 5; ++i)
	{
		kprintf("TIME ELAPSED (%d): %d\n", (i+1) * CONSUMED_MAX, times[i]);
	}
	
}

process	main(void)
{
	recvclr();
	
	/* Create the shared circular buffer and semaphores*/
	/* */
	produced = semcreate(0);
	consumed = semcreate(1);
	head = tail = 0;

	producer_id = create(producer, 4096, 50, "producer", 0);
	consumer_id = create(consumer, 4096, 50, "consumer", 0);
	timer_id = create(time_and_end, 4096, 50, "timer", 0);

	resched_cntl(DEFER_START);
	resume(producer_id);
	resume(consumer_id);
	
	resume(timer_id); 
	resched_cntl(DEFER_STOP);

	return OK;
}
