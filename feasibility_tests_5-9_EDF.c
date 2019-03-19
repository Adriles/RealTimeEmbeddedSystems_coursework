// Author: Adrian Unkeles
// ECEN 5623
// Exercise 2, Part 4
// feasibility_tests_5-9_EDF.c : This file contains the 'main' function. Program execution begins and ends there.
//

#include <math.h>
#include <stdio.h>
#include <stdbool.h>

#define TRUE 1
#define FALSE 0
#define U32_T unsigned int

// U=1
U32_T ex5_period[] = { 2, 5, 10 };
U32_T ex5_wcet[] = { 1, 2, 1 };
U32_T ex5_ttd[] = { 0, 0, 0 };

//U=0.9967
U32_T ex6_period[] = { 2, 5, 7, 13 };
U32_T ex6_wcet[] = { 1, 1, 1, 2 };
U32_T ex6_ttd[] = { 0, 0, 0, 0 };

//U=1
U32_T ex7_period[] = { 3, 5, 15 };
U32_T ex7_wcet[] = { 1, 2, 4 };
U32_T ex7_ttd[] = { 0, 0, 0 };

//U=0.9967
U32_T ex8_period[] = { 2, 5, 7, 13 };
U32_T ex8_wcet[] = { 1, 1, 1, 2 };
U32_T ex8_ttd[] = { 0, 0, 0, 0 };

//U=1
U32_T ex9_period[] = { 6, 8, 12, 24 };
U32_T ex9_wcet[] = { 1, 2, 4, 6 };
U32_T ex9_ttd[] = { 0, 0, 0, 0 };

/*
edf_feasibility() implements the EDF scheduler over a set of services (at the moment handles at most 4 services)
Determines the priority of the services that still need to be given CPU time at the beginning of each clock cycle
*/

int edf_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[], U32_T ttd[])
{
	int service_iter;

	int set_feasible = TRUE;

	int clk = 0;
	
	U32_T common_multiple = 1;
	for (service_iter = 0; service_iter < numServices; service_iter++)
		common_multiple = common_multiple * period[service_iter];

	int service_counter[] = { 0, 0, 0, 0 };
	bool missed_deadline[] = { FALSE, FALSE, FALSE, FALSE };
	int service_serviced[] = { 0, 0, 0, 0 };
	
	int highest_prio_srv_ttd;
	int highest_prio_srv_id = 0;

	bool service_completed[] = { FALSE, FALSE, FALSE, FALSE };

	for (clk = 0; clk < common_multiple; clk++)
	{
		if ((missed_deadline[0] == TRUE) || (missed_deadline[1] == TRUE) || (missed_deadline[2] == TRUE) || (missed_deadline[3] == TRUE))
			break;

		// initializing to a value that is guaranteed to be overwritten (longer than all known service periods in these sets)
		highest_prio_srv_ttd = 100;

		// initialize all the periods on the first clock
		if (clk == 0)
		{
			for (service_iter = 0; service_iter < numServices; service_iter++)
			{
				ttd[service_iter] = period[service_iter];
			}
		}

		// determine which is the currently highest priority task
		for (service_iter = 0; service_iter < numServices; service_iter++)
		{
			// skip over completed services in determining highest priority
			if (service_completed[service_iter] == TRUE)
				continue;

			if (ttd[service_iter] < highest_prio_srv_ttd)
			{
				highest_prio_srv_ttd = ttd[service_iter];
				highest_prio_srv_id = service_iter;
			}
		}
		
		// determine if the service is completed
		service_serviced[highest_prio_srv_id]++;
		if (service_serviced[highest_prio_srv_id] == wcet[highest_prio_srv_id])
			service_completed[highest_prio_srv_id] = TRUE;


		for (service_iter = 0; service_iter < numServices; service_iter++)
		{
			service_counter[service_iter]++;
			ttd[service_iter] = period[service_iter] - service_counter[service_iter];
			if (ttd[service_iter] == 0)
			{
				ttd[service_iter] = period[service_iter];
				service_counter[service_iter] = 0;
				service_serviced[service_iter] = 0;

				if (service_completed[service_iter] != TRUE)
					missed_deadline[service_iter] = TRUE;

				service_completed[service_iter] = FALSE;
			}
		}
		
	}

	if (clk != common_multiple)
		set_feasible = FALSE;
	else
		set_feasible = TRUE;

	return set_feasible;
}



int main()
{
	U32_T numServices;

	printf("******** Feasibility Test for EDF scheduler of Examples 5 through 9\n");


	printf("Ex-5 U=%4.2f (C1=1, C2=2, C3=1; T1=2, T2=5, T3=10; T=D): ",
		((1.0 / 2.0) + (2.0 / 5.0) + (1.0 / 10.0)));
	numServices = sizeof(ex5_period) / sizeof(U32_T);
	if(edf_feasibility(numServices, ex5_period, ex5_wcet, ex5_period, ex5_ttd) == TRUE)
		printf("Feasible\n");
	else
		printf("Infeasible\n");

	printf("Ex-6 U=%4.2f (C1=1, C2=1, C3=1, C4=2; T1=2, T2=5, T3=7, T4=13; T=D): ",
		((1.0 / 2.0) + (1.0 / 5.0) + (1.0 / 7.0) + (2.0 / 13)));
	numServices = sizeof(ex6_period) / sizeof(U32_T);
	if(edf_feasibility(numServices, ex6_period, ex6_wcet, ex6_period, ex6_ttd) == TRUE)
		printf("Feasible\n");
	else
		printf("Infeasible\n");

	printf("Ex-7 U=%4.2f (C1=1, C2=2, C3=4; T1=3, T2=5, T3=15; T=D): ",
		((1.0 / 3.0) + (2.0 / 5.0) + (4.0 / 15.0)));
	numServices = sizeof(ex7_period) / sizeof(U32_T);
	if (edf_feasibility(numServices, ex7_period, ex7_wcet, ex7_period, ex7_ttd) == TRUE)
		printf("Feasible\n");
	else
		printf("Infeasible\n");

	printf("Ex-8 U=%4.2f (C1=1, C2=1, C3=1, C4=2; T1=2, T2=5, T3=7, T4=13; T=D): ",
		((1.0 / 2.0) + (1.0 / 5.0) + (1.0 / 7.0) + (2.0 / 13)));
	numServices = sizeof(ex8_period) / sizeof(U32_T);
	if(edf_feasibility(numServices, ex8_period, ex8_wcet, ex8_period, ex8_ttd) == TRUE)
		printf("Feasible\n");
	else
		printf("Infeasible\n");

	printf("Ex-9 U=%4.2f (C1=1, C2=2, C3=4, C4=6; T1=6, T2=8, T3=12, T4=24; T=D): ",
		((1.0 / 6.0) + (2.0 / 8.0) + (4.0 / 12.0) + (6.0 / 24)));
	numServices = sizeof(ex9_period) / sizeof(U32_T);
	if(edf_feasibility(numServices, ex9_period, ex9_wcet, ex9_period, ex9_ttd) == TRUE)
		printf("Feasible\n");
	else
		printf("Infeasible\n");
}
