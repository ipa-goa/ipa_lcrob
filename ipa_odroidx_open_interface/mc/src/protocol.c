
//----- Include Files ---------------------------------------------------------
#include <avr/io.h>		// include I/O definitions (port names, pin names, etc)
#include <avr/interrupt.h>	// include interrupt support
#define __DELAY_BACKWARD_COMPATIBLE__  // For delay 
#include <util/delay.h>
#include "protocol.h"
#include "uart.h"
#include "softuart.h"
#include "mcontroller.h"

#define 	DEFAULT_UART_BAUD_RATE 115200
#define 	MAX_STREAM_PACKETS 10 //Sets the maximum number of packet IDs to be expected with the command STREAM (Should be confirmed and set according to the capacity of the controller).

uint32_t			MASTER_UART_BAUD_RATE = DEFAULT_UART_BAUD_RATE;

uint8_t				STREAM_PACKET_ID[MAX_STREAM_PACKETS];
uint8_t				NUMBER_OF_PACKETS;	
volatile uint8_t	TIMER_OVERFLOW = 0;
/*
implement

(https://kforge.ros.org/turtlebot/turtlebot/file/d21c5431efda/turtlebot_driver/src/turtlebot_driver.py)
    57 ROOMBA_OPCODES = dict(
    58     start = 128,				impl
    59     baud = 129,				impl
    60     control = 130,			impl
    61     safe = 131,				impl
    62     full = 132,				impl
    63     power = 133,				
    64     spot = 134,				impl
    65     clean = 135,				impl
    66     max = 136,				?
    67     drive = 137,				TODO
    68     motors = 138,			impl
    69     leds = 139,				impl
    70     song = 140,				impl
    71     play = 141,				impl
    72     sensors = 142,			TODO
    73     force_seeking_dock = 143,		impl
    74     )
    75 
    76 CREATE_OPCODES = dict(
    77     soft_reset = 7,  # Where is this documented?
    78     low_side_drivers = 138,
    79     song = 140,
    80     play_song = 141,
    81     pwm_low_side_drivers = 144,
    82     direct_drive = 145,
    83     digital_outputs = 147,
    84     stream = 148,
    85     query_list = 149,
    86     pause_resume_stream = 150,
    87     send_ir = 151,
    88     script = 152,
    89     play_script = 153,
    90     show_script = 154,
    91     wait_time = 155,
    92     wait_distance = 156,
    93     wait_angle = 157,
    94     wait_event = 158,
    95     )
*/

#define OP_START	128
#define OP_BAUD		129
#define OP_CONTROL	130
#define OP_SAFE		131
#define OP_FULL		132
#define OP_SPOT		134
#define OP_COVER	135
#define OP_DEMO		136
#define OP_DRIVE	137
#define OP_LS_DRIVERS	138
#define OP_LEDS		139
#define OP_SONG		140
#define OP_PLAY_SONG	141
#define OP_SENSORS	142
#define OP_COVERDOCK	143
#define OP_DRIVE_DIRECT	145
#define OP_OUTPUT	147
#define OP_STREAM	148
#define OP_PAUSE_RESUME	150
#define OP_SEND_IR	151
#define OP_SCRIPT	152
#define OP_PLAY_SCRIPT	153
#define OP_SHOW_SCRIPT	154
#define OP_WAIT		155
#define OP_STREAM_RESPONSE 19

#define PID_BW_DROPS	7
#define PID_WALL	8
#define PID_CLIFF_L	9
#define PID_CLIFF_FL	10
#define PID_CLIFF_FR	11
#define PID_CLIFF_R	12
#define PID_VWALL	13
#define PID_LS_DRIVER	14
#define PID_DIRT_DETECT	15
#define PID_UNUSED1	16
#define PID_IR		17
#define PID_BUTTONS	18
#define PID_DISTANCE 19 
#define PID_ANGLE 20
#define PID_CHARGING_STATE 21
#define PID_VOLTAGE 22
#define PID_CURRENT 23
#define PID_TEMPERATURE 24
#define PID_BATTERY_CHARGE 25
#define PID_BATTERY_CAPACITY 26
#define PID_WALL_SIGNAL 27
#define PID_CLIFF_LEFT_SIGNAL 28
#define PID_CLIFF_FRONT_LEFT_SIGNAL 29
#define PID_CLIFF_FRONT_RIGHT_SIGNAL 30
#define PID_CLIFF_RIGHT_SIGNAL 31
#define PID_UNUSED2 32
#define PID_UNUSED3 33
#define PID_CHARGER_AVAILABLE 34
#define PID_OPEN_INTERFACE_MODE 35
#define PID_SONG_NUMBER 36
#define PID_SONG_PLAYING 37
#define PID_OI_STREAM_NUM_PACKETS 38
#define PID_VELOCITY 39
#define PID_RADIUS 40
#define PID_VELOCITY_RIGHT 41
#define PID_VELOCITY_LEFT 42


void init(void){
	NUMBER_OF_PACKETS = 0;
	TIMER_OVERFLOW = 0;
	uart_init(UART_BAUD_SELECT(MASTER_UART_BAUD_RATE, F_CPU ));
	//All initiialization is to be done here.
	//ADC
	softuart_init(PORT_1);
	softuart_init(PORT_2);
	sei();
	motor_init(); // enable motors after both soft uart ports are enabled.
}

uint8_t uart_get_valid_char(){
	uint16_t buffer = 0;
	do{
		buffer = uart_getc();
	}while((buffer & UART_NO_DATA) || (buffer & UART_OVERRUN_ERROR) || (buffer & UART_BUFFER_OVERFLOW));
	return (uint8_t)(buffer & 0xff);
}
void sendSensorPacket(uint8_t packet_id) {
	switch(packet_id) {
		case PID_LS_DRIVER:

		case PID_CHARGING_STATE:
		case PID_CLIFF_L:
		case PID_CLIFF_R:
		case PID_CLIFF_FR:
		case PID_CLIFF_FL:
		case PID_WALL:
		case PID_VWALL:
		case PID_BUTTONS:
		case PID_UNUSED1:
		case PID_UNUSED2:
		case PID_BW_DROPS:
		case PID_TEMPERATURE:
		case PID_CHARGER_AVAILABLE:
		case PID_OPEN_INTERFACE_MODE:
		case PID_SONG_NUMBER:
		case PID_SONG_PLAYING:
		case PID_OI_STREAM_NUM_PACKETS:
			uart_putc(0);
			break;

		case PID_IR:
			uart_putc(0xff);
			break;
		case PID_DISTANCE:
		case PID_ANGLE:
		case PID_VOLTAGE: //Will have to implement 
		case PID_CURRENT:
		case PID_BATTERY_CHARGE:
		case PID_BATTERY_CAPACITY:
		case PID_WALL_SIGNAL:
		case PID_CLIFF_LEFT_SIGNAL:
		case PID_CLIFF_FRONT_LEFT_SIGNAL:
		case PID_CLIFF_FRONT_RIGHT_SIGNAL:
		case PID_CLIFF_RIGHT_SIGNAL:
		case PID_UNUSED3:
		case PID_VELOCITY:
		case PID_RADIUS:
		case PID_VELOCITY_RIGHT:
		case PID_VELOCITY_LEFT:
			uart_putc(0x00);
			uart_putc(0x00);
			break;

	}
}

void parseSendSensorPacket(uint8_t packet_id){
	switch(packet_id) {
		case 0:
			for(packet_id=7; packet_id<=26; packet_id++) sendSensorPacket(packet_id);
			break;
		case 1:
			for(packet_id=7; packet_id<=16; packet_id++) sendSensorPacket(packet_id);
			break;
		case 2:
			for(packet_id=17; packet_id<=20; packet_id++) sendSensorPacket(packet_id);
			break;
		case 3:
			for(packet_id=21; packet_id<=26; packet_id++) sendSensorPacket(packet_id);
			break;
		case 4:
			for(packet_id=27; packet_id<=34; packet_id++) sendSensorPacket(packet_id);
			break;
		case 5:
			for(packet_id=35; packet_id<=42; packet_id++) sendSensorPacket(packet_id);
			break;
		case 6:
			for(packet_id=7; packet_id<=42; packet_id++) sendSensorPacket(packet_id);
			break;
		//Implement all packet types here
		default: sendSensorPacket(packet_id); 
			break;
	}
}

void parse(void)
{
	int16_t vel, radius;
	int16_t vel1, vel2;
	uint8_t command = 0;
	uint8_t packet_id = 0;
	uint16_t uart_read = 0;

	uart_read = uart_getc();
	if ((uart_read & 0xff00)!= 0)
		return; //Have to test
	command = uart_read;
	switch(command ) {	//opcode
		case OP_START:
			break;
		case OP_BAUD:
			switch(uart_get_valid_char()) {//baud
				case 0: MASTER_UART_BAUD_RATE=300;break;
				case 1: MASTER_UART_BAUD_RATE=600;break;
				case 2: MASTER_UART_BAUD_RATE=1200;break;
				case 3: MASTER_UART_BAUD_RATE=2400;break;
				case 4: MASTER_UART_BAUD_RATE=4800;break;
				case 5: MASTER_UART_BAUD_RATE=9600;break;
				case 6: MASTER_UART_BAUD_RATE=14400;break;
				case 7: MASTER_UART_BAUD_RATE=19200;break;
				case 8: MASTER_UART_BAUD_RATE=28800;break;
				case 9: MASTER_UART_BAUD_RATE=38400;break;
				case 10: MASTER_UART_BAUD_RATE=57600;break;
				default: MASTER_UART_BAUD_RATE=115200;break;
			}
			cli();// Should be tested
			uart_init(UART_BAUD_SELECT(MASTER_UART_BAUD_RATE, F_CPU ));
			sei();
			break;
		case OP_CONTROL: 
		case OP_SAFE:
			break;
		case OP_PLAY_SCRIPT:
			break;
		case OP_SHOW_SCRIPT:
//			uartPutByte(0); //no script
			uart_get_valid_char();
			break;
		case OP_FULL:
			break;
		case OP_DEMO:	
			uart_get_valid_char();
			break;
		case OP_SEND_IR:	
			uart_get_valid_char();
			break;
		case OP_PLAY_SONG:	
			uart_get_valid_char();			
			break;
		case OP_LEDS:	
			uart_get_valid_char();uart_get_valid_char();uart_get_valid_char();
			break;
		case OP_OUTPUT:	
			uart_get_valid_char();			
			break;
		case OP_COVER:
			break;
		case OP_COVERDOCK:
			break;
		case OP_SPOT:
			break;
		case OP_LS_DRIVERS:	
			uart_get_valid_char();			
			break;
		case OP_DRIVE:
			//TODO: Maybe? since all calculations are to be done on the master. 
			vel    = (((uint16_t)uart_get_valid_char())<<8) | uart_get_valid_char();
			radius = (((uint16_t)uart_get_valid_char())<<8) | uart_get_valid_char();
			break;
		case OP_DRIVE_DIRECT:
			//TODO:
			vel1 = (((uint16_t)uart_get_valid_char())<<8) | uart_get_valid_char();
			vel2 = (((uint16_t)uart_get_valid_char())<<8) | uart_get_valid_char();
			motor_setVel(vel1, vel2);
			break;
		case OP_SONG:
			uart_get_valid_char();			
			vel = uart_get_valid_char();			
			for(radius=0; radius!=vel; radius++) {
				uart_get_valid_char();uart_get_valid_char();
			}
			break;
		case OP_SENSORS:
			packet_id = uart_get_valid_char();			
			parseSendSensorPacket(packet_id);
			break;
		case OP_STREAM:
			NUMBER_OF_PACKETS = uart_get_valid_char();
			for (int i=0; i<NUMBER_OF_PACKETS; i++){ // Make sure the array STREAM_PACKET_ID is big enough
				STREAM_PACKET_ID[i] = uart_get_valid_char();
			}
			//A timer is set for 15 ms, if the stream is enabled a STREAM_RESPONSE packet is generated.
			TCNT1 = 0x78FF; //15ms
			TCCR1B |= (1 << CS11);
	 		TIMSK1 |= (1 << TOIE1);
			break;
		case OP_PAUSE_RESUME:
			if (uart_get_valid_char()){
				if (NUMBER_OF_PACKETS>0){
					TCNT1 = 0x78FF; //15ms
					TCCR1B |= (1 << CS11);
	 				TIMSK1 |= (1 << TOIE1);
				}
			}
			else{
				TCNT1 = 0x0; //Disabling timer
				TCCR1B &= 0xF8;
	 			TIMSK1 &= ~(1 << TOIE1);
			}
			//TODO:
			break;
		case OP_SCRIPT:
			vel = uart_get_valid_char();			
			for(radius=0; radius!=vel; radius++) {
				uart_get_valid_char();
			}
			break;
		case OP_WAIT:
			_delay_ms( uart_get_valid_char()*15 );
			break;
	}

/*	if(recveivedM1) {
	}

	if(recveivedM2) {
	}
	*/
}

void generateStreamResponse(){
	uint8_t count=0;
	uart_putc(OP_STREAM_RESPONSE);
	for (int i=0; i<NUMBER_OF_PACKETS; i++){
		switch(STREAM_PACKET_ID[i]){
			case PID_VOLTAGE:
			case PID_VELOCITY:
			case PID_VELOCITY_LEFT:
			case PID_VELOCITY_RIGHT:
				count+=3;
		}
	}
	uart_putc(count);
	uart_enable_checksum();
	for(int i = 0; i<NUMBER_OF_PACKETS; i++){
		uart_putc(STREAM_PACKET_ID[i]);
		parseSendSensorPacket(STREAM_PACKET_ID[i]);
	}
	uart_put_checksum();
}


ISR(TIMER1_OVF_vect){
	TIMER_OVERFLOW = 1;
	TCNT1 = 0x78FF; //15ms
	TCCR1B |= (1 << CS11);
	TIMSK1 |= (1 << TOIE1);
}
