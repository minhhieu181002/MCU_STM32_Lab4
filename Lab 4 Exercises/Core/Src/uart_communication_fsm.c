/*
 * uart_communication_fsm.c
 *
 *  Created on: Nov 16, 2023
 *      Author: HP
 */
#include"uart_communication_fsm.h"
#include"software_timer.h"
#include"main.h"
#include"global.h"
#include <stdio.h>
#include <string.h>




uint8_t temp = 0;
uint8_t buffer [MAX_BUFFER_SIZE];
uint8_t buffer_flag = 0;

int command_state = 0;
const char userRequest[] = "!RST#";
const char userEnd[] = "!OK#";

char str[100];
int status_UART = 0;
int cnt_ADC_value = 0;

void normal_mode() {
	HAL_SuspendTick();
	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	HAL_ResumeTick();
}
void clearBuffer() {
	for (int i = 0; i < MAX_BUFFER_SIZE; i++) {
		buffer[i] = 0;
	}
	index_buffer = 0;

}

void command_parser_fsm () {
	switch(index_buffer){
	case 5:
		if(strcmp((const char*)buffer,userRequest) == 0) {
				ADC_value = HAL_ADC_GetValue(&hadc1);
				status_UART = SEND_ADC;
		}
		else{
			status_UART = ERROR_COMMAND;
		}
		break;
	case 4:
		if(strcmp((const char*)buffer,userEnd) == 0) {
			status_UART = END_COMMUNICATION;
		}
		else{
			status_UART = ERROR_COMMAND;
		}
		break;
	default:
		status_UART = ERROR_COMMAND;
		break;
	}

}
void uart_communication_fsm () {
	switch(status_UART) {
			case NORMAL:
				normal_mode();
				break;
			case SEND_ADC:
				HAL_UART_Transmit(&huart2, (uint8_t*)str, sprintf(str,"!ADC=%lu#\r\n", ADC_value), 1000);
				clearBuffer();
				status_UART = WAITING;  //time out for waiting
				setTimer4(300);
				break;
			case WAITING:
				if(timer4_flag == 1) {
					cnt_ADC_value++;
					HAL_UART_Transmit(&huart2, (uint8_t*)str,
							sprintf(str,"\r\n!ADC=%lu#\r\n", ADC_value), 1000);
					if(cnt_ADC_value >= 3){
						status_UART = END_COMMUNICATION;
						cnt_ADC_value = 0;
					}
					else{
						setTimer4(300);
					}

					clearBuffer();
				}
				break;
			case END_COMMUNICATION:
				HAL_UART_Transmit(&huart2, (uint8_t*)str,
						sprintf(str, "%s","\r\nCommunication is end\r\n"), 1000);
				clearBuffer();
				status_UART = NORMAL;
				break;
			case ERROR_COMMAND:
				HAL_UART_Transmit(&huart2, (uint8_t*)str,
						sprintf(str, "%s","Error Command\r\n"), 1000);
				clearBuffer();
				status_UART = NORMAL;
				break;
	}
}
