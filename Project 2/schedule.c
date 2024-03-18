/* schedule.c
 * This file contains the primary logic for the 
 * scheduler.
 */
#include "schedule.h"
#include "macros.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include <limits.h>
#include "privatestructs.h"

#define NEWTASKSLICE (NS_TO_JIFFIES(100000000))

/* Local Globals
 * rq - This is a pointer to the runqueue that the scheduler uses.
 * current - A pointer to the current running task.
 */
struct runqueue *rq;
struct task_struct *current;

/* External Globals
 * jiffies - A discrete unit of time used for scheduling.
 *			 There are HZ jiffies in a second, (HZ is 
 *			 declared in macros.h), and is usually
 *			 1 or 10 milliseconds.
 */
extern long long jiffies;
extern struct task_struct *idle;

/*-----------------Initilization/Shutdown Code-------------------*/
/* This code is not used by the scheduler, but by the virtual machine
 * to setup and destroy the scheduler cleanly.
 */
 
 /* initscheduler
  * Sets up and allocates memory for the scheduler, as well
  * as sets initial values. This function should also
  * set the initial effective priority for the "seed" task 
  * and enqueu it in the scheduler.
  * INPUT:
  * newrq - A pointer to an allocated rq to assign to your
  *			local rq.
  * seedTask - A pointer to a task to seed the scheduler and start
  * the simulation.
  */
void initschedule(struct runqueue *newrq, struct task_struct *seedTask)
{
	/*Assign the given newrq allocated in cpu.c to the *rq defined in the scheduler*/
	/*rq = newrq;*/
	seedTask->next = seedTask->prev = seedTask;
	newrq->head = seedTask;
	newrq->nr_running++;
}

/* killschedule
 * This function should free any memory that 
 * was allocated when setting up the runqueu.
 * It SHOULD NOT free the runqueue itself.
 */
void killschedule()
{
	return;
}


void print_rq () {
	struct task_struct *curr;
	
	printf("Rq: \n");
	curr = rq->head;
	if (curr)
		printf("%p", curr);
	while(curr->next != rq->head) {
		curr = curr->next;
		printf(", %p", curr);
	};
	printf("\n");
}

/*-------------Scheduler Code Goes Below------------*/
/* This is the beginning of the actual scheduling logic */

/* schedule
 * Gets the next task in the queue
 */
void schedule()
{
	struct task_struct *curr = NULL;
	unsigned long long tempBurstPred;

//	printf("In schedule\n");
//	print_rq();

	/*If rq->nr_running == 1 it means we just have init*/
	if(rq->nr_running == 1) {
		current->need_reschedule = 0;
		curr = rq->head;
		context_switch(curr);
	}
	else {
		if(current!=rq->head) {
			/*Assume it will lose the cpu, but keep temp values*/
			current->lastBurstTime += (sched_clock()-current->timeStarted);
			current->totalExecutionTime += (sched_clock()-current->timeStarted);
			tempBurstPred = current->nextBurstPred;
			current->nextBurstPred = (current->lastBurstTime + 0.5*current->nextBurstPred) / (1+0.5);
		}

		/*If process has been removed from the queue, calculate final burst prediction and reset burst counter*/
		if((current->next == NULL && current->prev == NULL) && current != rq->head) {
			current->nextBurstPred = (current->lastBurstTime + 0.5*current->nextBurstPred) / (1+0.5);
			current->lastBurstTime = 0; /*Reset the last burst counter, will start counting again next time it runs*/
		}

		current->need_reschedule = 0;

		/*Update WaitingInRQ*/
		updateWaitingInRQ();

		/*Get the task with the minimum goodness to run next*/
		curr = getMinGoodness();

		/*Comment the line above and uncomment the line below to disable min goodness selection mode and enable min exp burst selection mode*/
		/*curr = getMinExpBurst();*/

		/*Keep the start of this burst round*/
		/*If curr was null we would not reach this point, so curr can never be null.*/
		curr->timeStarted = sched_clock();

		/*If process switched*/
		if(curr != current) {			
			if(sched_clock() > current->lastRunOrInQueue) {
				current->lastRunOrInQueue = sched_clock();
			}
		}
		/*Else restore the temp calculated value*/
		else if (rq->nr_running != 1) {
			current->nextBurstPred = tempBurstPred;
		}
		context_switch(curr);
		
	}
}

void updateWaitingInRQ() {
	struct task_struct *tempTask;

	tempTask = rq->head->next;
	int i=0;
	while(i<rq->nr_running && tempTask != rq->head) {
		/*Current task always has 0 waitingInRQ*/
		if(tempTask == current) {
			tempTask->waitingInRQ = 0;
		}
		else {
			tempTask->waitingInRQ = sched_clock() - tempTask->lastRunOrInQueue;
		}
		tempTask = tempTask->next;
		i++;
	}
}

struct task_struct* getMinGoodness() {
	struct task_struct *curr = NULL; /*Used to hold currently selected task with minimum exp burst prediction*/
	struct task_struct *tempTask; /*Used to iterate through the loop*/
	struct task_struct *minBurstTask; /*Used to hold the task returned by getMinExpBurst*/
	struct task_struct *maxWaitingInRQTask; /*Used to hold the task returned by getMaxWaitingInRQ*/
	unsigned long long minGoodness = ULLONG_MAX; /*Set initial value to +infinity*/
	unsigned long long tempGoodness; /*Will hold the calculation for comparison each iteration*/
	

	tempTask = rq->head->next;
	int i=0;
	while(i<rq->nr_running && tempTask != rq->head) {
		/*Get the 2 tasks from the functions*/
		minBurstTask = getMinExpBurst();
		maxWaitingInRQTask = getMaxWaitingInRQ();

		tempGoodness = ((1+tempTask->nextBurstPred)/(1+minBurstTask->nextBurstPred)) * ((1+maxWaitingInRQTask->waitingInRQ) / (1+tempTask->waitingInRQ));
		
		tempTask->goodness = tempGoodness; /*Update current task's calculated goodness for debugging*/
		if(tempGoodness <= minGoodness) {
			minGoodness = tempGoodness;
			curr = tempTask;
		}
		tempTask = tempTask->next;
		i++;
	}

	return curr;
}

struct task_struct* getMinExpBurst() {
	struct task_struct *tempTask; /*Used to iterate through the loop*/
	struct task_struct *curr = NULL; /*Used to hold currently selected task with minimum exp burst prediction*/
	unsigned long long minExpBurst = ULLONG_MAX; /*Set initial value to +infinity*/

	tempTask = rq->head->next;
	int i=0;
	while(i<rq->nr_running && tempTask != rq->head) {
		/*Compare selected mininum exp burst with current tasks's exp burst*/
		if(tempTask->nextBurstPred < minExpBurst) {
			minExpBurst = tempTask->nextBurstPred;
			curr = tempTask;
		}

		tempTask = tempTask->next;
		i++;
	}

	return curr;
}

struct task_struct* getMaxWaitingInRQ() {
	struct task_struct *tempTask; /*Used to iterate through the loop*/
	struct task_struct *curr = NULL; /*Used to hold currently selected task with maximum waiting in RQ time*/
	unsigned long long maxWaitingInRQ = 0; /*Set initial value to smallest one, which is 0*/

	tempTask = rq->head->next;
	int i=0;
	while(i<rq->nr_running && tempTask != rq->head) {
		/*Compare the current maximum waiting in RQ value with the time the task has been waiting in the queue (current time minus when it was put in queue)*/
		if(tempTask->waitingInRQ >= maxWaitingInRQ) {
			maxWaitingInRQ = tempTask->waitingInRQ;
			curr = tempTask;
		}
		tempTask = tempTask->next;
		i++;
	}

	return curr;
}

/* sched_fork
 * Sets up schedule info for a newly forked task
 */
void sched_fork(struct task_struct *p)
{
	p->time_slice = 100;
	p->lastBurstTime = 0;
	p->lastRunOrInQueue = 0;
	p->nextBurstPred = 0;
	p->timeStarted = 0;
	p->goodness = 0;
	p->waitingInRQ = 0;
	p->totalProcessingTime = sched_clock();
	p->totalExecutionTime = 0;
	p->totalWaitingTime = 0;
}

/* scheduler_tick
 * Updates information and priority
 * for the task that is currently running.
 */
void scheduler_tick(struct task_struct *p)
{
	schedule();
}

/* wake_up_new_task
 * Prepares information for a task
 * that is waking up for the first time
 * (being created).
 */
void wake_up_new_task(struct task_struct *p)
{	
	p->next = rq->head->next;
	p->prev = rq->head;
	p->next->prev = p;
	p->prev->next = p;

	/*Update last run or in queue time*/
	if(sched_clock() > p->lastRunOrInQueue) {
		p->lastRunOrInQueue = sched_clock();
	}
	
	rq->nr_running++;
}

/* activate_task
 * Activates a task that is being woken-up
 * from sleeping.
 */
void activate_task(struct task_struct *p)
{
	p->next = rq->head->next;
	p->prev = rq->head;
	p->next->prev = p;
	p->prev->next = p;

	/*When the task is being placed in the queue, update the time it was returned to the queue, if needed.*/
	if(sched_clock() > p->lastRunOrInQueue) {
		p->lastRunOrInQueue = sched_clock();
	}
	
	rq->nr_running++;
}

/* deactivate_task
 * Removes a running task from the scheduler to
 * put it to sleep.
 */
void deactivate_task(struct task_struct *p)
{
	p->prev->next = p->next;
	p->next->prev = p->prev;
	p->next = p->prev = NULL; /* Make sure to set them to NULL *
							   * next is checked in cpu.c      */

	rq->nr_running--;
}
