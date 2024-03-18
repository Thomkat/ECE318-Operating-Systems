#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "macros.h"
#include "list.h"

struct thread_info;

/* ---------------- Do NOT Touch -------------- */
/* Sleep Types */
enum sleep_type
{
	SLEEP_NORMAL,
	SLEEP_NONINTERACTIVE,
	SLEEP_INTERACTIVE,
	SLEEP_INTERRUPTED
};

/* task_struct */
struct task_struct
{
	struct thread_info *thread_info;		/* Information about the thread */
	unsigned int time_slice;				/* Timeslice values */
	enum sleep_type sleep_type;				/* What type of sleep task is in */
	int need_reschedule;					/* Flag, set if task needs to
											   have schedule called */
	struct task_struct *prev, *next;	/* Used to link the task struct in the 
										   runqueue. Make sure to set them to
										   NULL when the process is not in the
										   runqueue */
	unsigned long long lastBurstTime; /*Last burst time of the task*/
	unsigned long long nextBurstPred; /*Final prediction of the next burst*/
	unsigned long long lastRunOrInQueue; /*Time when process last lost its CPU or was placed in the queue (keeping the greatest value)*/
	unsigned long long waitingInRQ; /*Waiting in RQ value calculated each round*/
	unsigned long long timeStarted; /*Time when process got its CPU*/
	unsigned long long goodness; /*Not needed for implementation, used for debugging*/
	unsigned long long totalProcessingTime; /*JUST FOR DEBUGGING / REPORT */
	unsigned long long totalWaitingTime; /*JUST FOR DEBUGGING / REPORT */
	unsigned long long totalExecutionTime; /*JUST FOR DEBUGGING / REPORT */
/* ---------------- Do NOT Touch END-------------- */
};

/* runqueue */
struct runqueue {
    unsigned long    nr_running;			/* number of runnable tasks */
	struct task_struct *head;
};

/*----------------------- System Calls ------------------------------*/
/* These calls are provided by the VM for your
 * convenience, and mimic system calls provided
 * normally by Linux
 */
void context_switch(struct task_struct *next); 
unsigned long long sched_clock();

/*------------------YOU MAY EDIT BELOW THIS LINE---------------------*/
/*------------------- User Defined Functions -------------------------*/
/*-------------These functions MUST be defined for the VM-------------*/
void initschedule(struct runqueue *newrq, struct task_struct *seedTask);
void killschedule();
void schedule();
void activate_task(struct task_struct *p);
void deactivate_task(struct task_struct *p);
void scheduler_tick(struct task_struct *p);
void sched_fork(struct task_struct *p);
void wake_up_new_task(struct task_struct *p);
struct task_struct* getMinExpBurst();
struct task_struct* getMaxWaitingInRQ();
struct task_struct* getMinGoodness();
void updateWaitingInRQ();

#endif