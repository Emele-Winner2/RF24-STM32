/*
 * RF24.h
 *
 *  Created on: 22 Aug 2026
 *      Author: Ekemini Udofia
 */

#ifndef INC_RF24_H_
#define INC_RF24_H_

#include <RF24_config.h>

typedef enum { LOW = 0, HIGH } mode;

typedef enum {
  RF24_PA_MIN = 0,
  RF24_PA_LOW,
  RF24_PA_HIGH,
  RF24_PA_MAX,
  RF24_PA_ERROR
} rf24_pa_dbm_e;

typedef enum { RF24_1MBPS = 0, RF24_2MBPS, RF24_250KBPS } rf24_datarate_e;

typedef enum {
  RF24_CRC_DISABLED = 0,
  RF24_CRC_8,
  RF24_CRC_16
} rf24_crclength_e;

typedef struct {
  SPI_HandleTypeDef* hspi;       // Pointer to SPI instance (e.g., &hspi1)
  bool wide_band;                /* 2Mbs data rate in use? */
  bool p_variant;                /* False for RF24L01 and true for RF24L01P */
  uint8_t payload_size;          /**< Fixed size of payloads */
  bool ack_payload_available;    /**< Whether there is an ack payload waiting */
  bool dynamic_payloads_enabled; /**< Whether dynamic payloads are enabled. */
  uint8_t ack_payload_length;    /**< Dynamic size of pending ack payload. */
  uint64_t
      pipe0_reading_address; /**< Last address set on pipe 0 for reading. */

  GPIO_TypeDef* ce_port;   // GPIO Port for CE (e.g., GPIOB)
  uint16_t ce_pin;         // GPIO Pin for CE (e.g., GPIO_PIN_0)
  GPIO_TypeDef* csn_port;  // GPIO Port for CSN (e.g., GPIOA)
  uint16_t csn_pin;        // GPIO Pin for CSN (e.g., GPIO_PIN_4)
} RF24;

void csn(const RF24 *radio, mode pinMode);
void ce(const RF24 *radio, mode pinMode);
uint8_t read_register_bytes(const RF24 *radio, uint8_t reg, uint8_t* buf, uint8_t len);
uint8_t read_register(const RF24 *radio, uint8_t reg);
uint8_t write_register_bytes(const RF24 *radio, uint8_t reg, const uint8_t* buf, uint8_t len);
uint8_t write_register(const RF24 *radio, uint8_t reg, uint8_t value);
uint8_t write_payload(const RF24 *radio, const void* buf, uint8_t len);
uint8_t read_payload(const RF24 *radio, void* buf, uint8_t len);
uint8_t flush_rx(const RF24 *radio);
uint8_t flush_tx(const RF24 *radio);
uint8_t get_status(const RF24 *radio);
//void print_status(uint8_t status);
//void print_observe_tx(uint8_t value);
//void print_byte_register(const char* name, uint8_t reg, uint8_t qty = 1);
//void print_address_register(const char* name, uint8_t reg, uint8_t qty = 1);
void toggle_features(const RF24 *radio);

void RF24_Init(RF24 *radio, SPI_HandleTypeDef *_hspi,
		GPIO_TypeDef *_ceport, uint16_t _cepin, GPIO_TypeDef *_csport, uint16_t _cspin);
//#ifdef ARDUINO  // I'm yet to change this
//SPI.setBitOrder(MSBFIRST);
//SPI.setDataMode(SPI_MODE0);
//SPI.setClockDivider(SPI_CLOCK_DIV4);
//#endif
//
bool begin(RF24 *radio);
void startListening(const RF24 *radio);
void stopListening(const RF24 *radio);
bool write(RF24 *radio, const void* buf, uint8_t len);
bool available_null(const RF24 *radio);
bool read(const RF24 *radio, void* buf, uint8_t len);
void openWritingPipe(RF24 *radio, uint64_t address);
void openReadingPipe(RF24 *radio, uint8_t number, uint64_t address);
void openReadingPipe_u8(RF24 *radio, uint8_t child, const uint8_t* address);
void setRetries(const RF24 *radio, uint8_t delay, uint8_t count);
void setChannel(const RF24 *radio, uint8_t channel);
void setPayloadSize(RF24 *radio, uint8_t size);
uint8_t getPayloadSize(const RF24 *radio);
uint8_t getDynamicPayloadSize(const RF24 *radio);
void enableAckPayload(const RF24 *radio);
void enableDynamicPayloads(RF24 *radio);
bool isPVariant(const RF24 *radio);
void setAutoAck(const RF24 *radio, bool enable);
void setAutoAck_pipe(const RF24 *radio, uint8_t pipe, bool enable);
void setPALevel(const RF24 *radio, rf24_pa_dbm_e level);
rf24_pa_dbm_e getPALevel(const RF24 *radio);
bool setDataRate(RF24 *radio, rf24_datarate_e speed);
rf24_datarate_e getDataRate(const RF24 *radio);
void setCRCLength(const RF24 *radio, rf24_crclength_e length);
rf24_crclength_e getCRCLength(const RF24 *radio);
void disableCRC(const RF24 *radio);
void printDetails(const RF24 *radio);
void powerDown(const RF24 *radio);
void powerUp(const RF24 *radio);
bool available(const RF24 *radio, uint8_t* pipe_num);
void startWrite(const RF24 *radio, const void* buf, uint8_t len);
void writeAckPayload(const RF24 *radio, uint8_t pipe, const void* buf, uint8_t len);
bool isAckPayloadAvailable(RF24 *radio);
void whatHappened(const RF24 *radio, bool *tx_ok, bool *tx_fail, bool *rx_ready);
bool testCarrier(const RF24 *radio);
bool testRPD(const RF24 *radio);
//bool isValid(const RF24 *radio) { return radio->ce_pin != 0xff && radio->csn_pin != 0xff; }

#endif /* INC_RF24_H_ */
