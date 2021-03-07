#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <wiringPi.h>
#include <pthread.h>
#include <termios.h>

#define LED 4

int isQuit;

int compareRunTime(struct timeval, struct timeval, int);

void *t_receive_signal(void*);
void *t_analyze_signal(void*);
void *t_Led(void*);
void *t_getch(void*);

void sendBeacon();

// display menus
int display_menu() {
	int menu = 0;

	while(1) {
		system("clear");
		printf("\n\n\t\t\tSeclect Menu \n");
		printf("\t\t\t========================= \n");
		printf("\t\t\t1) Test Program\n");				// Start alarm test by distance
		printf("\t\t\t2) Start Receive\n");				// Start signal receiving
		printf("\t\t\t3) Start Send\n");				// Start signal sending
		printf("\t\t\t4) Exit Program\n");				// Exit program
		printf("\t\t\t========================= \n");
		printf("\t\t\tSelect : ");
		scanf("%d", &menu);
		if(menu < 1 || menu > 4) {
			continue;
		}
		else {
			return menu;
		}
	}

	return 0;
}
// Start alarm test by distance
int start_test() {
	double distance;
	struct timeval Tstart, Tend;
	// Default settings for LED control
	if(wiringPiSetup() == -1) {
		printf("wiringPiSetup() error!\n");
		exit(0);
	}
	// Set LED to OUTPUT mode
	pinMode(LED, OUTPUT);
	
	system("clear");
	printf("\n\n\t\t\tStart Test\n");
	printf("\t\t\t========================= \n");
	printf("\t\t\tEnter the Distance : ");
	scanf("%lf", &distance);		// Enter distance for testing
	printf("\t\t\t========================= \n");
	printf("\t\t\tDistance : %lf \n", distance);
	printf("\t\t\tStart LED processing...\n");
	// LED control start
	gettimeofday(&Tstart, NULL);
	while(1) {
		digitalWrite(LED, 1);
		delay((int)distance);

		digitalWrite(LED, 0);
		delay((int)distance);

		gettimeofday(&Tend, NULL);
		// Run for 3 seconds only
		if(compareRunTime(Tstart, Tend, 3))
			break;
	}
	printf("\t\t\tEnd............\n");
	sleep(3);

	return 1;
}
// Start signal reception
int start_receive() {
	pthread_t p_thread[3];

	int thr_id, status;
	char p1[] = "Signal_Reception_thread";
	char p2[] = "LED_thread";
	char p3[] = "Key_input_check_thread";
	char pM[] = "signal_analyze_thread";
	// Start beacon signal reception thread
	thr_id = pthread_create(&p_thread[0], NULL, t_receive_signal, (void *)p1);

	if(thr_id < 0) {
		perror("thread create error : ");
		exit(0);
	}
	// Start Led control thread
	thr_id = pthread_create(&p_thread[1], NULL, t_Led, (void *)p2);
	if(thr_id < 0) {
		perror("thread create error : ");
		exit(0);
	}
	// Start Key input check thread
	thr_id = pthread_create(&p_thread[2], NULL, t_getch, (void *)p3);
	if(thr_id < 0) {
		perror("thread create error : ");
		exit(0);
	}
	// Start beacon signal analysis thread
	t_analyze_signal((void *)pM);
	// Waiting for thread to terminate
	pthread_join(p_thread[0], (void **)&status);
	pthread_join(p_thread[1], (void **)&status);
	pthread_join(p_thread[2], (void **)&status);

	return 1;
}
// Start signal sending
int start_send() {
	pthread_t p_thread;

	int thr_id, status;
	char p[] = "Key_input_check_thread";

	system("clear");
	printf("\n\n\n\t\tSend Beacon......\n");
	printf("\t\t=================================\n");

	// Start Key input check thread
	thr_id = pthread_create(&p_thread, NULL, t_getchs, (void *)p);
	if(thr_id < 0) {
		perror("thread create error : ");
		exit(0);
	}

	isQuit = 0;
	while(1) {
		if(isQuit == 1)
			break;
		sendBeacon();
	}
	system("sudo hciconfig hci0 noleadv");

	pthread_join(p_thread, (void **)&status);

	return 1;
}

int main() {
	int menu = 1;

	while(menu) {
		menu = display_menu();

		if(menu == 1) {
			menu = start_test();
		}
		else if(menu == 2) {
			menu = start_receive();
		}
		else if(menu == 3) {
			menu = start_send();
		}
		else if(menu == 4) {
			system("clear");
			exit(0);
		}
	}

	return 0;
}
