/*
    FreeRTOS V8.2.0 - Copyright (C) 2015 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

	***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
	***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
	the FAQ page "My application does not run, what could be wrong?".  Have you
	defined configASSERT()?

	http://www.FreeRTOS.org/support - In return for receiving this top quality
	embedded software for free we request you assist our global community by
	participating in the support forum.

	http://www.FreeRTOS.org/training - Investing in training allows your team to
	be as productive as possible as early as possible.  Now you can receive
	FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
	Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/*
 * Creates all the demo application tasks, then starts the scheduler.
 * In addition to the standard demo tasks, the following tasks and tests are
 * defined and/or created within this file:
 *
 * "Check" task -  This only executes every five seconds but has the highest
 * priority so is guaranteed to get processor time.  Its main function is to 
 * check that all the standard demo tasks are still operational.  The check
 * task will write an error message to the console should an error be detected
 * within any of the demo tasks.  The check task also toggles the LED defined
 * by mainCHECK_LED every 5 seconds while the system is error free, with the
 * toggle rate increasing to every 500ms should an error occur.
 * 
 * "Reg test" tasks - These fill the registers with known values, then check
 * that each register still contains its expected value.  Each task uses
 * different values.  The tasks run with very low priority so get preempted very
 * frequently.  A register containing an unexpected value is indicative of an
 * error in the context switching mechanism.
 *
 * See the online documentation for this demo for more information on interrupt
 * usage.
 */

/*******************************************************************************
// Copyright (c) 2003-2015 Cadence Design Systems, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
--------------------------------------------------------------------------------
*/

#define TEST_MODE 1  // 1 for medium test, 2 for full test, 3 for Xtensa specific tests (cf. tests below).

// When using TEST_MODE 3, more Xtensa specific tests can be selected below (select one test and recompile)
const int u_xt_alloca                  = 0;
const int u_xt_coproc                  = 1;
const int u_xt_clib                    = 0;
const int u_xt_intr                    = 0;
const int u_example                    = 0;
const int u_perf_test                  = 0;

/* Standard includes. */
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "porttrace.h"
#include "portbenchmark.h"

/* Demo application includes. */
#include "partest.h"
#include "flash.h"
#include "blocktim.h"
#include "semtest.h"
#include "serial.h"
#include "comtest.h"
#include "GenQTest.h"
#include "QPeek.h"
#include "integer.h"
#include "PollQ.h"
#include "BlockQ.h"
#include "dynamic.h"
#include "countsem.h"
#include "recmutex.h"
#include "death.h"
#include "EventGroupsDemo.h"
#include "flop.h"

/*-----------------------------------------------------------*/

/* The rate at which the LED controlled by the 'check' task will toggle when no
errors have been detected. */
#define mainNO_ERROR_PERIOD	( 2000 )

/* The rate at which the LED controlled by the 'check' task will toggle when an
error has been detected. */
#define mainERROR_PERIOD 	( 100 )

/* FreeRTOS base timer period
 */
#define mainTIMER_PERIOD    ( mainNO_ERROR_PERIOD / 5)

/* The LED toggled by the Check task. */
#define mainCHECK_LED       ( 7 )

/* The first LED used by the ComTest tasks.  One LED toggles each time a 
character is transmitted, and one each time a character is received and
verified as being the expected character. */
#define mainCOMTEST_LED     ( 4 )

/* Priority definitions for the tasks in the demo application. */
#define mainLED_TASK_PRIORITY		 ( tskIDLE_PRIORITY + 1 )
#define mainCREATOR_TASK_PRIORITY	 ( tskIDLE_PRIORITY + 3 )
#define mainCHECK_TASK_PRIORITY		 ( tskIDLE_PRIORITY + 4 )
#define mainQUEUE_POLL_PRIORITY		 ( tskIDLE_PRIORITY + 2 )
#define mainQUEUE_BLOCK_PRIORITY	 ( tskIDLE_PRIORITY + 3 )
#define mainCOM_TEST_PRIORITY		 ( tskIDLE_PRIORITY + 2 )
#define mainSEMAPHORE_TASK_PRIORITY	 ( tskIDLE_PRIORITY + 1 )
#define mainGENERIC_QUEUE_PRIORITY	 ( tskIDLE_PRIORITY )
#define mainREG_TEST_PRIORITY        ( tskIDLE_PRIORITY )
#define mainQUEUE_OVERWRITE_PRIORITY ( tskIDLE_PRIORITY )
#define mainFLOP_TASK_PRIORITY       ( tskIDLE_PRIORITY )

/* Misc. */
#define mainDONT_WAIT						( 0 )

/* The parameters passed to the reg test tasks.  This is just done to check
the parameter passing mechanism is working correctly. */
#define mainREG_TEST_1_PARAMETER    ( ( void * ) 0x12345678 )
#define mainREG_TEST_2_PARAMETER    ( ( void * ) 0x87654321 )

/*-----------------------------------------------------------*/

/*
 * Setup the processor ready for the demo.
 */
static void prvSetupHardware( void );

/*
 * Execute all of the check functions to ensure the tests haven't failed.
 */ 
static void prvCheckTask( void *pvParameters );

/*
 * The register test (or RegTest) tasks as described at the top of this file.
 */
static void prvFirstRegTestTask( void *pvParameters );
static void prvSecondRegTestTask( void *pvParameters );

/*-----------------------------------------------------------*/

/* Counters that are incremented on each iteration of the RegTest tasks
so long as no errors have been detected. */
volatile unsigned long ulRegTest1Counter = 0UL;
volatile unsigned long ulRegTest2Counter = 0UL;

/*-----------------------------------------------------------*/

void exit(int);
void puthex(unsigned long n);

// Select between Xtensa (0) or FreeRTOS (1) tests
#define F_ON     (TEST_MODE == 1 || TEST_MODE == 2)
#define F_FULL   (TEST_MODE==2)

// FreeRTOS tests: to enable or disable certain tests
#if F_ON
const int uStartIntegerMathTasks       = 1;
#endif

#if F_FULL
const int uStartLEDFlashTasks          = 1;
const int uStartPolledQueueTasks       = 1;
const int uStartBlockingQueueTasks     = 1;
const int uCreateBlockTimeTasks        = 1;
const int uStartSemaphoreTasks         = 1;
const int uStartDynamicPriorityTasks   = 1;  // This test fails if there are insufficient cycles to run its tasks; increase ticks per second
const int uStartQueuePeekTasks         = 1;
const int uStartGenericQueueTasks      = 1;
const int uStartCountingSemaphoreTasks = 1;
const int uStartRecursiveMutexTasks    = 1;
const int uStartEventGroupTasks        = 1;
const int uStartTimerDemoTask          = 1;
const int uStartQueueSetTasks          = 1;
const int uStartQueueOverwriteTask     = 1;
const int uStartMathTasks              = 1;
#endif

// The main main for all tests
int main(int argc, char *argv[])
{
	#if !F_ON
	if (u_xt_alloca)        main_xt_alloca(0, NULL);
	else if (u_xt_coproc)   main_xt_coproc(0, NULL);
	else if (u_xt_clib)     main_xt_clib(0, NULL);
	else if (u_xt_intr)     main_xt_intr(0, NULL);
	else if (u_example)     main_example(0, NULL);
	else if (u_perf_test)   main_perf_test(0, NULL);
	#else
	main_freertos(0, NULL);
	#endif
	return 0;
}

/*
 * Create FreeRTOS demo tasks then start the scheduler.
 */
#if F_ON
int main_freertos(int argc, char *argv[])
{
    /* Configure any hardware required for this demo. */
	prvSetupHardware();

	/* Create all the other standard demo tasks.  These serve no purpose other
    than to test the port and demonstrate the use of the FreeRTOS API. */
	if (uStartIntegerMathTasks)       vStartIntegerMathTasks( mainGENERIC_QUEUE_PRIORITY );

	#if F_FULL
	if (uStartLEDFlashTasks)          vStartLEDFlashTasks( tskIDLE_PRIORITY );
	if (uStartPolledQueueTasks)       vStartPolledQueueTasks( mainQUEUE_POLL_PRIORITY );
	if (uStartBlockingQueueTasks)     vStartBlockingQueueTasks( mainQUEUE_BLOCK_PRIORITY );
	if (uCreateBlockTimeTasks)        vCreateBlockTimeTasks();
	if (uStartSemaphoreTasks)         vStartSemaphoreTasks( mainSEMAPHORE_TASK_PRIORITY );
	if (uStartDynamicPriorityTasks)   vStartDynamicPriorityTasks();
	if (uStartQueuePeekTasks)         vStartQueuePeekTasks();
	if (uStartGenericQueueTasks)      vStartGenericQueueTasks( mainGENERIC_QUEUE_PRIORITY );
	if (uStartCountingSemaphoreTasks) vStartCountingSemaphoreTasks();
	if (uStartRecursiveMutexTasks)    vStartRecursiveMutexTasks();
	if (uStartEventGroupTasks)        vStartEventGroupTasks();
	if (uStartTimerDemoTask)          vStartTimerDemoTask(mainTIMER_PERIOD);
	if (uStartQueueSetTasks)          vStartQueueSetTasks();
	if (uStartQueueOverwriteTask)     vStartQueueOverwriteTask(mainQUEUE_OVERWRITE_PRIORITY);
	if (uStartMathTasks)              vStartMathTasks(mainFLOP_TASK_PRIORITY);
	#endif // F_FULL

	/* prvCheckTask uses sprintf so requires more stack. */
	xTaskCreate( prvCheckTask, "Check", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL );
    
    /* The RegTest tasks as described at the top of this file. */
    xTaskCreate( prvFirstRegTestTask, "Rreg1", configMINIMAL_STACK_SIZE, mainREG_TEST_1_PARAMETER, mainREG_TEST_PRIORITY, NULL );
    xTaskCreate( prvSecondRegTestTask, "Rreg2", configMINIMAL_STACK_SIZE, mainREG_TEST_2_PARAMETER, mainREG_TEST_PRIORITY, NULL );

	/* This task has to be created last as it keeps account of the number of tasks
	it expects to see running. */
	vCreateSuicidalTasks( mainCREATOR_TASK_PRIORITY );

    /* Finally start the scheduler. */
	vTaskStartScheduler();

	/* Will only reach here if there is insufficient heap available to start
	the scheduler. */
	puts("Error: scheduler did not start!");
	exit(-1);
}
/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
    /* Setup the digital IO for the LED's. */
    vParTestInitialise();
}
/*-----------------------------------------------------------*/

static void prvCheckTask( void *pvParameters )
{
TickType_t xLastExecutionTime, ulTicksToWait = mainNO_ERROR_PERIOD;
unsigned long ulLastRegTest1 = 0UL, ulLastRegTest2 = 0UL;
	unsigned long counter = 0UL;
const char * pcMessage;

	// If benchmark code is enabled (cf. configBENCHMARK in FreeRTOSConfig.h)
	portbenchmarkReset();

	/* Initialise the variable used to control our iteration rate prior to
	its first use. */
	xLastExecutionTime = xTaskGetTickCount();

	for( ;; )
	{
		int error = 0;

		/* Wait until it is time to run the tests again. */
		vTaskDelayUntil( &xLastExecutionTime, ulTicksToWait);
		
		/* Have any of the standard demo tasks detected an error in their 
		operation? */
		if( xIsCreateTaskStillRunning() != pdTRUE )
		{
			error = 1;
			puts("Error: Suicidal Tasks.");
		}
		if( ulLastRegTest1 == ulRegTest1Counter )
        {
            /* ulRegTest1Counter is no longer being incremented, indicating
            that an error has been discovered in prvFirstRegTestTask(). */
            error = 1;
            puts("Error: Reg Test1.");
        }
        if( ulLastRegTest2 == ulRegTest2Counter )
        {
            /* ulRegTest2Counter is no longer being incremented, indicating
            that an error has been discovered in prvSecondRegTestTask(). */
            error = 1;
            puts("Error: Reg Test2.");
        }

        if( uStartIntegerMathTasks && xAreIntegerMathsTaskStillRunning() != pdTRUE )
		{
			error = 1;
			puts("Error: Integer Maths.");
		}

		#if F_FULL
		if( uStartGenericQueueTasks && xAreGenericQueueTasksStillRunning() != pdTRUE )
		{
			error = 1;
			puts("Error: GenQ.");
		}
		if( uStartBlockingQueueTasks && xAreBlockingQueuesStillRunning() != pdTRUE )
		{
			error = 1;
			puts("Error: BlockQ.");
		}
		if( uStartPolledQueueTasks && xArePollingQueuesStillRunning() != pdTRUE )
		{
			error = 1;
			puts("Error: PollQ.");
		}
		if( uStartQueuePeekTasks && xAreQueuePeekTasksStillRunning() != pdTRUE )
		{
			error = 1;
			puts("Error: PeekQ.");
		}
		if( uCreateBlockTimeTasks && xAreBlockTimeTestTasksStillRunning() != pdTRUE )
		{
			error = 1;
			puts("Error: Block Time.");
		}
		if( uStartSemaphoreTasks && xAreSemaphoreTasksStillRunning() != pdTRUE )
	    {
	        error = 1;
			puts("Error: Semaphore Test.");
	    }
		if( uStartDynamicPriorityTasks && xAreDynamicPriorityTasksStillRunning() != pdTRUE )
		{
			error = 1;
			puts("Error: Dynamic Priority.");
		}
		if( uStartCountingSemaphoreTasks && xAreCountingSemaphoreTasksStillRunning() != pdTRUE )
		{
			error = 1;
			puts("Error: Count Semaphore.");
		}
		if( uStartRecursiveMutexTasks && xAreRecursiveMutexTasksStillRunning() != pdTRUE )
		{
			error = 1;
			puts("Error: Recursive Mutex.");
		}
		if( uStartEventGroupTasks && xAreEventGroupTasksStillRunning() != pdTRUE )
		{
			error = 1;
			puts("Error: Event Group.");
		}
		if( uStartTimerDemoTask && xAreTimerDemoTasksStillRunning(mainNO_ERROR_PERIOD) != pdTRUE )
		{
			error = 1;
			puts("Error: Timer Demo.");
		}
		if( uStartQueueSetTasks && xAreQueueSetTasksStillRunning() != pdTRUE )
		{
			error = 1;
			puts("Error: Queue Set.");
		}
		if( uStartQueueOverwriteTask && xIsQueueOverwriteTaskStillRunning() != pdTRUE )
		{
			error = 1;
			puts("Error: Queue Overwrite.");
		}
		if( uStartMathTasks && xAreMathsTaskStillRunning() != pdTRUE )
		{
			error = 1;
			puts("Error: Flop Math Task.");
		}
		#endif // F_FULL

		portbenchmarkIntWait();
    	portbenchmarkPrint();

        /* Stop if errors are detected and report tasks status */
        if( error )
        {
        	portDISABLE_INTERRUPTS();
        	porttracePrint(-1);  // Empty macro if configUSE_TRACE_FACILITY_2 is not set
        	puts("ERRORS DETECTED; STOPPING");
        	puts("[Test Dynamic Priority fails if there are insufficient cycles to run the tasks; increase ticks per second]");
        	puthex(counter);
        	exit(-1);
        } else {
    		puts("CHECK PASSED --------------------");
        	puthex(counter);
        }
        
        /* Remember the counter values this time around so a counter failing
        to be incremented correctly can be spotted. */
        ulLastRegTest1 = ulRegTest1Counter;
        ulLastRegTest2 = ulRegTest2Counter;
        
        /* Provide visual feedback of the system status.  If the LED is toggled
        every 5 seconds then no errors have been found.  If the LED is toggled
        every 500ms then at least one error has been found. */
        vParTestToggleLED( mainCHECK_LED );

        counter++;
	}
}
/*-----------------------------------------------------------*/

// More testing tasks
static void prvFirstRegTestTask( void *pvParameters )
{
	uint32_t i = 0;
	TickType_t xLastExecutionTime = xTaskGetTickCount();

	float a = 4.001;
	float b = 2.0002;
    while (1)
    {
    	vTaskDelayUntil(&xLastExecutionTime, 41); // Prime number delay creates pseudo randomness
    	//vTaskPrioritySet(NULL, mainCHECK_TASK_PRIORITY);
    	puts("1");
    	//vTaskPrioritySet(NULL, mainREG_TEST_PRIORITY);
    	ulRegTest1Counter++;
    	b += a * 1.001 - 3;
    }
}

// More testing tasks
static void prvSecondRegTestTask( void *pvParameters )
{
	int i = 0, x = 10;

	float a = 10.0000;
	float b = 1.002;
	TickType_t xLastExecutionTime = xTaskGetTickCount();
	while (1)
	{
		vTaskDelayUntil(&xLastExecutionTime, 101); // Prime number delay creates pseudo randomness
    	//vTaskPrioritySet(NULL, mainCHECK_TASK_PRIORITY);
    	puts("2");
    	//vTaskPrioritySet(NULL, mainREG_TEST_PRIORITY);
		ulRegTest2Counter++;

		b += a * 10.111 - 3.22;
	}
}

// Print a number in hex
void puthex(unsigned long n)
{
	char str[17]; // Up to 16 characters (64 bits)
	int i = 16;   // Start filling from end (reverse order)
	str[i] = 0;   // Terminating null

	do {
		int x = n & 0xF;
		str[--i] = (x < 10) ? x + '0' : x + 'a';
		n >>= 4;
	} while(n);

	puts(str + i);
}

#endif // F_ON
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( void )
{
	/* Look at pxCurrentTCB to see which task overflowed its stack. */
	puts("Stack overflow. Stopping.");
	exit(0);
	for( ;; )
    {
		//asm( "break" );
    }
}
/*-----------------------------------------------------------*/

void _general_exception_handler( unsigned long ulCause, unsigned long ulStatus )
{
	/* This overrides the definition provided by the kernel.  Other exceptions
	should be handled here. */
	puts("General exception handler error. Stopping.");
	exit(0);
	for( ;; )
    {
		//asm( "break" );
    }
}

#if configUSE_TICK_HOOK
void vApplicationTickHook( void )
{
#if F_FULL
	/* Prodding periodically from the tick interrupt. */
	if (uStartTimerDemoTask)
		vTimerPeriodicISRTests(); // Defined in TimerDemo.c

	/* Call the periodic event group from ISR demo. */
	if (uStartEventGroupTasks)
		vPeriodicEventGroupsProcessing(); // Defined in EventGroupsDemo

	/* Call the periodic queue overwrite from ISR demo. */
	if (uStartQueueSetTasks)
		vQueueSetAccessQueueSetFromISR();

	/* Call the periodic queue overwrite from ISR demo. */
	if (uStartQueueOverwriteTask)
		vQueueOverwritePeriodicISRDemo(); // Defined in QueueOverwrite
#else
	if (u_perf_test)
		vPerfTestTickHook();
#endif
}
#endif
