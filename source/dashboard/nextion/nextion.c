// original author: Matthew Flanagan
// matthewdavidflanagan@outlook.com

// converted by: Luke Oxley 
// lcoxley@purdue.edu

#include "nextion.h"

extern q_handle_t q_tx_usart;

void set_flag(char* obj_name, uint8_t enable)
{
  if (enable) set_value(obj_name, NXT_VALUE, FLAG_ENABLED_PIC);
  else set_value(obj_name, NXT_VALUE, FLAG_DISABLED_PIC);
}

void set_float(char* obj_name, char* param, float num, uint8_t precision)
{
  char result[NXT_STR_SIZE];
  char *ptr = &result[0];
  result[0] = '\0';
  strcat(ptr, obj_name);
  strcat(ptr, param);
  sprintf(&result[strlen(result)], "\"%.*f\"", precision, num);
  strcat(ptr, NXT_CMD_TERM);
  qSendToBack(&q_tx_usart, (uint16_t *) ptr);
}

void set_value(char* obj_name, char* param, uint16_t val)
{
  char result[NXT_STR_SIZE];
	char* ptr = &result[0];
	result[0] = '\0';
  char  str_buff[6] = {0,0,0,0,0, '\0'};
  uint16_t rem = 0;
  for (int count = 4; count >= 0; count--)
  {
    rem = val % 10;
    val = val / 10;
    str_buff[count] = rem + ASCII_OFFSET;
  }
  strcat(ptr, obj_name);
  strcat(ptr, param);
  strcat(ptr, str_buff);
  strcat(ptr, NXT_CMD_TERM);
  qSendToBack(&q_tx_usart, (uint16_t *) ptr);
}

void set_text(char* obj_name, char* param, char* text)
{
  char result[NXT_STR_SIZE];
	char* ptr = &result[0];
	result[0] = '\0';
  strcat(ptr, obj_name);
  strcat(ptr, param);
  strcat(ptr, "\"");
  strcat(ptr, text);
  strcat(ptr, "\"");
  strcat(ptr, NXT_CMD_TERM);
  qSendToBack(&q_tx_usart, (uint16_t *) ptr);
}

void set_page(char* page_name) {
  char result[NXT_STR_SIZE]; //.val=XXXXXFFF
	char* ptr = &result[0];
	result[0] = '\0';
  strcat(ptr, "page ");
  strcat(ptr, page_name);
  strcat(ptr, NXT_CMD_TERM);
  qSendToBack(&q_tx_usart, (uint16_t *) ptr);
}

/**
 * @brief Sets the brightness of the Nextion display.
 * @param percentage The desired brightness level as a percentage (0-100).
 */
void set_brightness(uint8_t percentage) {
  if (percentage > 100) percentage = 100;
  char result[NXT_STR_SIZE];
  snprintf(result, sizeof(result), "dims=%d%s", percentage, NXT_CMD_TERM);
  qSendToBack(&q_tx_usart, (uint16_t *) result);
}