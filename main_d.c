/*  main.c  - main */

#include <xinu.h>
#define BUFFER_SIZE 40
pid32 producer_id;
pid32 consumer_id;
pid32 timer_id;

int32 consumed_count = 0;
const int32 CONSUMED_MAX = 100;

/* Define your circular buffer structure and semaphore variables here */
/* */
sid32 produced, consumed, mutex;
int a[BUFFER_SIZE];							//circular buffer of length 40 
int head, tail;

/* Place your code for entering a critical section here */
void mutex_acquire(sid32 mutex)
{
	/* */
	wait(mutex);
}

/* Place your code for leaving a critical section here */
void mutex_release(sid32 mutex)
{
	/* */
	signal(mutex);
}

/* Place the code for the buffer producer here */
process producer(void)
{
	/* */
    int n= 1;
    while(1)
    {
        mutex_acquire(consumed); 
		n++;
		mutex_acquire(mutex); 
        a[head++] = n;          /* enqueue(n) */
		if (head >= BUFFER_SIZE)          
			head = 0;                    
        mutex_release(mutex); 
		kprintf("Number produced %d\n", n); 
		mutex_release(produced);
		
    }     
	return OK;
}

/* Place the code for the buffer consumer here */
process consumer(void)
{
	/* Every time your consumer consumes another buffer element,
	 * make sure to include the statement:
	 *   consumed_count += 1;
	 * this will allow the timing function to record performance */
	/* */
	int x;
	while(1)
	{
		mutex_acquire(produced); 
		mutex_acquire(mutex);
		x = a[tail++];          /* x = dequeue() */
		if (tail >= BUFFER_SIZE)          
		    tail = 0;            
		consumed_count += 1;
		mutex_release(mutex);
		kprintf("Number consumed %d\n", x);
		mutex_release(consumed); 
			 
		        	
	}

	return OK;
}


/* Timing utility function - please ignore */
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
		int cnt =0;
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
	
	/* Create the shared circular buffer and semaphores here */
	/* */
	produced = semcreate(consumed_count);
	consumed = semcreate(CONSUMED_MAX);
	mutex = semcreate(1);
	head = tail = 0;

	producer_id = create(producer, 4096, 50, "producer", 0);
	consumer_id = create(consumer, 4096, 50, "consumer", 0);
	timer_id = create(time_and_end, 4096, 50, "timer", 0);

	resched_cntl(DEFER_START);
	resume(producer_id);
	resume(consumer_id);
	/* Uncomment the following line for part 3 to see timing results */
	resume(timer_id); 
	resched_cntl(DEFER_STOP);

	return OK;
}