#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int boolSetup = 0;

// Change GPS to URL format
int gpsToBleUrl(char *lat, char *lon, int lat_sign, int lon_sign, int lat_dec, int lon_dec, char *result) {
	int i, count;
	char buff[100];
	int lat_len, lon_len, boolLat;

	lat_len = strlen(lat);
	lon_len = strlen(lon);

	i = 0;
	count = 0;
	boolLat = 0;
	// boolLat == 0 --> lat, boolLat == 1 --> lon
	while(1) {
		if((count % 3) == 0) {
			if(boolLat == 0)
				buff[count] = ' ';
			else
				buff[count + 27] = ' ';
		}
		else {
			switch (count) {
				case 1 : case 4 :
					if(boolLat == 0)
						buff[count] = '0';
					else
						buff[count + 27] = '0';
					break;
				case 2 :
					if(boolLat == 0)
						buff[count] = lat_sign + '0';
					else
						buff[count + 27] = lon_sign + '0';
					break;
				case 5 :
					if(boolLat == 0)
						buff[count] = lat_dec + '0';
					else
						buff[count + 27] = lon_dec + '0';
					break;
				default :
					if(boolLat == 0) {
						if(i >= lat_len) {
							buff[count] = '0';
							i++;
						} else {
							buff[count] = lat[i];
							i++;
						}
					} else {
						if(i >= lon_len) {
							buff[count + 27] = '0';
							i++;
						} else {
							buff[count + 27] = lon[i];
							i++;
						}
					}
					break;
			}
		}
		count++;

		if(boolLat == 0 && i > 13) {
			boolLat = 1;
			count = 0;
			i = 0;
		} else if(boolLat == 1 && i > 11) {
			break;
		}
	}

	buff[count + 27] = '\0';
	for(i = 0; i < sizeof(buff); i++)
		result[i] = buff[i];

	return 0;
}

// Bluetooth beacon signal transmission
void sendBeacon() {
	// file open variable
	FILE *fp;

	int count ;
	double real;
	// GPS variable
	double lat;
	double lon;
	char *lat_buff;
	char *lon_buff;
	char lat_char[10];
	char lon_char[10];
	int dec_lat, sign_lat;
	int dec_lon, sign_lon;
	// beacon signal variable
	char send_beacon[256];
	char *beacon = "sudo hcitool -i hci0 cmd 0x08 0x0008 1F 02 01 06 03 03 AA FE 17 16 AA FE 10 00 02";
	// Start Receiving GPS signal
	system("./gps/gps.sh");

	sleep(5);
	// Read GPS signal
	if ( fp = fopen("./gps/gps.data", "r")) {
		count =0;

	        while(0 < fscanf(fp, "%lf", &real)){
			if(count == 0)
				lat=real;
			else
				lon=real;
			count++;
		}
	}
	fclose(fp);

	char buffer[100];
	char buffer2[100];
	// Convert double to char
	lat_buff = ecvt(lat, 8, &dec_lat, &sign_lat);
	strncpy(lat_char, lat_buff, sizeof(lat_char));

	lon_buff = ecvt(lon, 9, &dec_lon, &sign_lon);
	strncpy(lon_char, lon_buff, sizeof(lon_char));
	// make URL
	gpsToBleUrl(lat_char, lon_char, sign_lat, sign_lon, dec_lat, dec_lon, buffer);
	// Completes the beacon signal
	sprintf(send_beacon, "%s%s", beacon, buffer);
	// Executes basic commands for beacon signal transmission
	if(boolSetup == 0) {
		system("sudo hciconfig hci0 up");
		system("sudo hciconfig hci0 leadv");
		system("sudo hciconfig hci0 noscan");
		boolSetup = 1;
	}
	// Show information
	system("clear");
	printf("\n\n\n\t\tSend Beacon......\n");
	printf("\t\t=================================\n");
	printf("\t\tLAT : %lf || LON : %lf\n", lat, lon);
	printf("%s\n", send_beacon);
	system(send_beacon);
	printf("\t\t=================================\n");
	printf("\t\tQuit key is 'q'\n");
}