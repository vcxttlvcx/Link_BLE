
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>
#include <math.h>
#include <wiringPi.h>
#include <sys/time.h>
#include <termios.h>

#define BUFF_SIZE 1024
#define PI 3.14159265358979323846
#define LED 4		// GPIO 23

int isLed = 0;
int isQuit = 0;
double distance = 0;
// Receive real-time key input for sendBeacon
int getch() {
	int ch;
	struct termios buf;
	struct termios save;

	tcgetattr(0, &save);
	buf = save;
	buf.c_lflag &= ~(ICANON | ECHO);
	buf.c_cc[VMIN] = 1;
	buf.c_cc[VTIME] = 0;
	tcsetattr(0, TCSAFLUSH, &buf);
	ch = getchar();
	tcsetattr(0, TCSAFLUSH, &save);

	return ch;
}
// A thread that terminates when 'q' is input
void *t_getch(void *data) {
	pid_t pid;
	pthread_t tid;

	pid = getpid();
	tid = pthread_self();

	char* thread_name = (char*)data;

	int ch;

	while(1) {
		ch = getch();
		if(ch == 'q' || ch == 'Q')
			break;
	}

	isQuit = 1;
	printf("\n\t\t\tShutting down.......\n");
}
// Calculate distance using gps signal
double Distance(double lat1, double lon1, double lat2, double lon2, char unit) {
        double deg2radMultiplier = PI / 180;
        lat1 = lat1 * deg2radMultiplier;
        lon1 = lon1 * deg2radMultiplier;
        lat2 = lat2 * deg2radMultiplier;
        lon2 = lon2 * deg2radMultiplier;

        double radius = 6378.137; // earth mean radius defined by WGS84
        double dlon = lon2 - lon1;
        double distance = acos(sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(dlon)) * radius;

        if (unit == 'K') {
                return (distance);
        }
        else if (unit == 'M') {
                return (distance * 0.621371192);
        }
        else if (unit == 'N') {
                return (distance * 0.539956803);
        }
        else {
                return 0;
        }
}
// Thread 1
// receiving beacon signal
void *t_receive_signal(void *data) {
	pid_t pid;
	pthread_t tid;

	pid = getpid();
	tid = pthread_self();

	char* thread_name = (char*)data;
	int i = 0;

	char buff[BUFF_SIZE];

	sleep(2);
	FILE *fp;
	FILE *fp2;

	fp2 = popen("sudo hciconfig hci0 reset", "r");
	if(NULL == fp2) {
		perror("popen() fail");
		exit(0);
	}
	pclose(fp2);

	fp = popen("sudo hcitool -i hci0 lescan --duplicates 1> /dev/null", "r");

	if(NULL == fp) {
		perror("popen() fail");
		exit(0);
	}

	sleep(1);
	while(1) {
		if(isQuit == 1)
			break;
	}
}

void removeRSN(char *str) {
	int len = strlen(str);
	str[len-1] = '\0';
}
// Remove whitespace from a URL
void removeSpace(char *str, char *result, int start, int end) {
	int i, j = 0;

	for(i = start; i < end; i++) {
		if(str[i] != ' ') {
			result[j] = str[i];
			j++;
		}
	}
	result[j] = 0;
}
// Change to UUID format
void makeUUID(char *uuid, char *result) {
	char temp[BUFF_SIZE];
	int i, j = 0;

	for(i = 0; i < strlen(uuid); ) {
		switch(j) {
			case 8 :
				temp[j] = '-';
				j++;
				break;
			case 13 :
				temp[j] = '-';
				j++;
				break;
			case 18 :
				temp[j] = '-';
				j++;
				break;
			case 23 :
				temp[j] = '-';
				j++;
				break;
			default :
				temp[j] = uuid[i];
				j++;
				i++;
				break;
		}
	}
	temp[j] = '\0';

	for(i = 0; i < sizeof(temp); i++)
		result[i] = temp[i];
}
// Bring a GPS signal
void bringGps(double *lat, double *lon) {
	FILE *fp;
    	double real;

	int count;

	system("./gps/gps.sh");

	if (fp = fopen("./gps/gps.data", "r")) {
		count = 0;
		while (0 < fscanf(fp, "%lf", &real)){
			if (count == 0)
				*lat = real;
			else
				*lon = real;

			count++;
		}
	}

	fclose(fp);
}
// Separate gps data from url
void receivedGps(char *url, double *lat_in, double *lon_in) {
	int i = 0, j = 0, k = 0;
	int lat_sign, lon_sign;
	int lat_dec, lon_dec;

	char lat_tmp[17];
	char lon_tmp[17];

	char lat_s[14];
	char lon_s[14];

	for(i = 0; i < 16; i++) {
		lat_tmp[i] = url[i];
		lon_tmp[i] = url[i + 18];
	}
	lat_tmp[i] = '\0';
	lon_tmp[i] = '\0';

	lat_sign = lat_tmp[1] - '0';
	lat_dec = lat_tmp[3] - '0';

	lon_sign = lon_tmp[1] - '0';
	lon_dec = lon_tmp[3] - '0';

	j = 0;
	i = 4;
	k = 4;
	for(j = 0; j < 16; j++) {
		if(j == lat_dec) {
			lat_s[j] = '.';
		} else {
			lat_s[j] = lat_tmp[i];
			i++;
		}
		if(j == lon_dec) {
			lon_s[j] = '.';
		} else {
			lon_s[j] = lon_tmp[k];
			k++;
		}
	}

	*lat_in = atof(lat_s);
	*lon_in = atof(lon_s);
}
// Show information
void print_status(char* packet, double lat, double lon, double lat_in, double lon_in, double distance) {
	system("clear");
	printf("\n\n\n");
	printf("\t\t\t========================= \n");
	printf("\t\t\tPacket : %s\n", packet);
	printf("\t\t\t========================= \n");
	printf("\t\t\tLAT    : %lf || LON    : %lf\n", lat, lon);
	printf("\t\t\tLAT_IN : %lf || LON_in : %lf\n", lat_in, lon_in);
	printf("\t\t\t========================= \n");
	printf("\t\t\tDistance : %lf\n", distance);
	printf("\t\t\t========================= \n");
	printf("\t\t\tQuit Key is 'q' \n");
}
// thread
// beacon signal processing
void *t_analyze_signal(void *data) {
	pid_t pid;
	pthread_t tid;

	pid = getpid();
	tid = pthread_self();

	char* thread_name = (char*)data;
	int i = 0;

	FILE *fp;
	char buff[BUFF_SIZE];
	char buff2[BUFF_SIZE];
	char packet[BUFF_SIZE];
	char c;
	char ch;
	// beacon singal format
	const char ibeacon[] = "^> 04 3E 2A 02 01 .{26} 02 01 .{14} 02 15";				// ibeacon
	const char eddystone[] = "^> 04 3E .{2} 02 01 00 00 .{2} A3 3E 95 01 00";		// eddystone
	char *ptr;
	// Regular expression variation
	regex_t ext_regex;
	// Gps variation
	double lat = 0, lon = 0;			// This deivce gsp data
	double lat_in = 0, lon_in = 0;		// Received gps data
	// Show basic information
	print_status("", lat, lon, lat_in, lon_in, distance);
	// Start beacon signal analysis
	isQuit = 0;
	while(1) {
		if(isQuit == 1)
			break;

		fp = popen("sudo hcidump --raw", "r");
		if(NULL == fp) {
			perror("popen() fail");
			exit(0);
		}

		do {
			if(isQuit == 1)
				break;

			c = fgetc(fp);

			if (c != '\n') {
				buff[i] = c;
				i++;
			} else {
				if(buff[0] == '>') {
					strcpy(packet, buff2);
					(char *)memset(buff2, '\0', sizeof(buff));
					strncpy(buff2, buff, sizeof(buff) - 1);
				} else {
					strncat(buff2, (buff+2), sizeof(buff) - 1);
				}

				if(packet[0] != '\0') {
					// Filtering beacon signals using regular expression
					regcomp(&ext_regex, eddystone, REG_EXTENDED);

					if(!regexec(&ext_regex, packet, 0, NULL, 0)) {
						// beacon signal variation
						int size;
						char url[BUFF_SIZE], cSize[4];

						FILE *file;

						file = fopen("Log", "a");
						if(NULL == file) {
							perror("fopen() error");
							exit(0);
						}

						// remove spaces from url and char to int
						removeSpace(packet, cSize, 41, 43);
						size = (int)strtol(cSize, NULL, 16);

						removeSpace(packet, url, 86, (43 + (size * 3)));
						// receive gps signal
						bringGps(&lat, &lon);
						receivedGps(url, &lat_in, &lon_in);
						// calculate distance and show information
						distance = Distance(lat, lon, lat_in, lon_in, 'K');
						print_status(packet, lat, lon, lat_in, lon_in, distance);
						// save results
						fprintf(file, "URL : %s LAT : %lf LON : %lf LAT_IN : %lf LON_IN : %lf DISTANCE : %lf \n", url, lat, lon, lat_in, lon_in, distance);
						// Initialize variable
						(char *)memset(packet, '\0', sizeof(packet));

						fclose(file);
						// LED processing on
						isLed = 1;
					} else {
						// LED processing off
						isLed = 0;
					}
				}
				(char *)memset(buff, '\0', sizeof(buff));
				i = 0;
			}
		} while(c != EOF);
		pclose(fp);
	}
}
// Compare Run Time
int compareExecuteTime(struct timeval Tstart, struct timeval Tend, int sec) {
	Tend.tv_usec = Tend.tv_usec - Tstart.tv_usec;
	Tend.tv_sec = Tend.tv_sec - Tstart.tv_sec;
	Tend.tv_usec += (Tend.tv_sec * 1000000);

	return ((Tend.tv_usec / 1000000.0) > sec) ? TRUE : FALSE;
}
// LED control thread
void *t_Led(void *data) {
	pid_t pid;
	pthread_t tid;

	pid = getpid();
	tid = pthread_self();

	char* thread_name = (char *)data;
	int i = 0;

	struct timeval Tstart, Tend;
	// Default settings for LED control
	if(wiringPiSetup() == -1) {
		printf("wiringPi Setup error!!\n");
		exit(0);
	}
	// Set LED to OUTPUT mode
	pinMode(LED, OUTPUT);
	// Start led control
	while(1) {
		if(isQuit == 1)
			break;
		if(isLed == 1) {
			gettimeofday(&Tstart, NULL);
			while(1) {
				digitalWrite(LED, 1);	// LED ON
				delay(distance);

				digitalWrite(LED, 0);	// LED OFF
				delay(distance);
				// Run for 4 seconds only
				gettimeofday(&Tend, NULL);
				if(compareExecuteTime(Tstart, Tend, 4))
					break;
			}
		}
		else {
			digitalWrite(LED, 0);
		}
	}
	digitalWrite(LED, 0);
}