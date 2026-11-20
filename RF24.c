/*
 * RF24.c
 *
 *  Created on: 22 Aug 2026
 *      Author: Ekemini Udofia
 */

#include "RF24.h"
#include "RF24_config.h"
#include "nRF24L01.h"

void csn(const RF24 *radio, mode pinMode) {
  HAL_GPIO_WritePin(radio->csn_port, radio->csn_pin, (GPIO_PinState)pinMode);
}

void ce(const RF24 *radio, mode pinMode) {
  HAL_GPIO_WritePin(radio->ce_port, radio->ce_pin, (GPIO_PinState)pinMode);
}

uint8_t read_register_bytes(const RF24 *radio, uint8_t reg, uint8_t* buf,
                      uint8_t len) {
  uint8_t cmd = R_REGISTER | (REGISTER_MASK & reg);
  uint8_t status = 0;

  csn(radio, LOW);

  // Send the command byte and receive the nRF status byte
  HAL_SPI_TransmitReceive(radio->hspi, &cmd, &status, 1, HAL_MAX_DELAY);

  uint8_t dummy[len];
  memset(dummy, 0xFF, len);

  // Get the actual data and put it in buf
  HAL_SPI_TransmitReceive(radio->hspi, dummy, buf, len, HAL_MAX_DELAY);

  csn(radio, HIGH);

  return status;
}

uint8_t read_register(const RF24 *radio, uint8_t reg) {
  uint8_t cmd[2] = {(R_REGISTER | (REGISTER_MASK & reg)), 0xFF};
  uint8_t result[2] = {0};  // To flush out the actual return data

  csn(radio, LOW);

  HAL_SPI_TransmitReceive(radio->hspi, cmd, result, 2, HAL_MAX_DELAY);

  csn(radio, HIGH);

  return result[1];
}

uint8_t write_register_bytes(const RF24 *radio, uint8_t reg, const uint8_t *buf,
                       uint8_t len) {
  uint8_t status = 0;
  uint8_t cmd = W_REGISTER | (REGISTER_MASK & reg);
  uint8_t dummy_rx[len];

  csn(radio, LOW);

  HAL_SPI_TransmitReceive(radio->hspi, &cmd, &status, 1, HAL_MAX_DELAY);

  HAL_SPI_TransmitReceive(radio->hspi, (uint8_t*)buf, dummy_rx, len, HAL_MAX_DELAY);

  csn(radio, HIGH);

  return status;
}

uint8_t write_register(const RF24* radio, uint8_t reg, uint8_t value) {
  uint8_t status = 0;
  uint8_t cmd = W_REGISTER | (REGISTER_MASK & reg);
  uint8_t dummy;

  csn(radio, LOW);

  HAL_SPI_TransmitReceive(radio->hspi, &cmd, &status, 1, HAL_MAX_DELAY);

  HAL_SPI_TransmitReceive(radio->hspi, &value, &dummy, 1, HAL_MAX_DELAY);

  csn(radio, HIGH);

  return status;
}

uint8_t write_payload(const RF24* radio, const void* buf, uint8_t len) {
  uint8_t status = 0;
  uint8_t cmd = W_TX_PAYLOAD;
  uint8_t blank = 0x00;

  const uint8_t* current = (const uint8_t*)buf;

  uint8_t data_len = min(len, radio->payload_size);
  uint8_t blank_len = radio->dynamic_payloads_enabled ? 0 : radio->payload_size - data_len;
  uint8_t dummy[data_len > blank_len ? data_len : blank_len];
  // printf("[Writing %u bytes %u blanks]",data_len,blank_len);

  csn(radio, LOW);

  HAL_SPI_TransmitReceive(radio->hspi, &cmd, &status, 1, HAL_MAX_DELAY);

  if (data_len > 0) {
    HAL_SPI_TransmitReceive(radio->hspi, (uint8_t*)current, dummy, data_len, HAL_MAX_DELAY);
  }

  if (blank_len > 0) {
    HAL_SPI_TransmitReceive(radio->hspi, &blank, dummy, blank_len, HAL_MAX_DELAY);
  }

  csn(radio, HIGH);

  return status;
}

uint8_t read_payload(const RF24* radio, void* buf, uint8_t len) {
  uint8_t status = 0;
  uint8_t cmd = R_RX_PAYLOAD;
  uint8_t* current = (uint8_t*)buf;

  uint8_t data_len = min(len, radio->payload_size);
  uint8_t blank_len = radio->dynamic_payloads_enabled ? 0 : radio->payload_size - data_len;

  uint8_t dummy_rx_data[data_len];

  memset(dummy_rx_data, 0xFF, data_len);

  // printf("[Reading %u bytes %u blanks]",data_len,blank_len);

  csn(radio, LOW);

  HAL_SPI_TransmitReceive(radio->hspi, &cmd, &status, 1, HAL_MAX_DELAY);

  if (data_len > 0) {
    HAL_SPI_TransmitReceive(radio->hspi, dummy_rx_data, current, data_len, HAL_MAX_DELAY);
  }

  if (blank_len > 0) {
	uint8_t dummy_tx = 0xFF;
	uint8_t garbage_rx = 0;
	for (uint8_t i = 0; i < blank_len; i++) {
	  HAL_SPI_TransmitReceive(radio->hspi, &dummy_tx, &garbage_rx, 1, HAL_MAX_DELAY);
	}
  }

  csn(radio, HIGH);

  return status;
}

uint8_t flush_rx(const RF24* radio) {
  uint8_t status = 0;
  uint8_t cmd = FLUSH_RX;

  csn(radio, LOW);

  HAL_SPI_TransmitReceive(radio->hspi, &cmd, &status, 1, HAL_MAX_DELAY);

  csn(radio, HIGH);

  return status;
}

uint8_t flush_tx(const RF24* radio) {
  uint8_t status = 0;
  uint8_t cmd = FLUSH_TX;

  csn(radio, LOW);

  HAL_SPI_TransmitReceive(radio->hspi, &cmd, &status, 1, HAL_MAX_DELAY);

  csn(radio, HIGH);

  return status;
}

uint8_t get_status(const RF24* radio) {
  uint8_t status = 0;
  uint8_t cmd = NOP;
  csn(radio, LOW);

  HAL_SPI_TransmitReceive(radio->hspi, &cmd, &status, 1, HAL_MAX_DELAY);

  csn(radio, HIGH);

  return status;
}

//void print_status(uint8_t status) {
//  printf_P(PSTR("STATUS\t\t = 0x%02x RX_DR=%x TX_DS=%x MAX_RT=%x RX_P_NO=%x "
//                "TX_FULL=%x\r\n"),
//           status, (status & _BV(RX_DR)) ? 1 : 0, (status & _BV(TX_DS)) ? 1 : 0,
//           (status & _BV(MAX_RT)) ? 1 : 0, ((status >> RX_P_NO) & B111),
//           (status & _BV(TX_FULL)) ? 1 : 0);
//}
//
//void print_observe_tx(uint8_t value) {
//  printf_P(PSTR("OBSERVE_TX=%02x: POLS_CNT=%x ARC_CNT=%x\r\n"), value,
//           (value >> PLOS_CNT) & B1111, (value >> ARC_CNT) & B1111);
//}
//
//void print_byte_register(const RF24 *radio, const char* name, uint8_t reg, uint8_t qty) {
//  char extra_tab = strlen_P(name) < 8 ? '\t' : 0;
//  printf_P(PSTR(PRIPSTR "\t%c ="), name, extra_tab);
//  while (qty--) printf_P(PSTR(" 0x%02x"), read_register(radio, reg++));
//  printf_P(PSTR("\r\n"));
//}
//
//void print_address_register(const RF24 *radio, const char* name, uint8_t reg, uint8_t qty) {
//  char extra_tab = strlen_P(name) < 8 ? '\t' : 0;
//  printf_P(PSTR(PRIPSTR "\t%c ="), name, extra_tab);
//
//  while (qty--) {
//    uint8_t buffer[5];
//    read_register_bytes(radio, reg++, buffer, sizeof buffer);
//
//    printf_P(PSTR(" 0x"));
//    uint8_t* bufptr = buffer + sizeof buffer;
//    while (--bufptr >= buffer) printf_P(PSTR("%02x"), *bufptr);
//  }
//
//  printf_P(PSTR("\r\n"));
//}

void RF24_Init(RF24 *radio, SPI_HandleTypeDef *_hspi, GPIO_TypeDef *_ceport, uint16_t _cepin, GPIO_TypeDef *_csport, uint16_t _cspin) {
  radio->hspi = _hspi;
  radio->ce_port = _ceport;
  radio->ce_pin = _cepin;
  radio->csn_port = _csport;
  radio->csn_pin = _cspin;

  // Settings
  radio->wide_band = true;
  radio->p_variant = false;
  radio->payload_size = 32;
  radio->ack_payload_available = false;
  radio->dynamic_payloads_enabled = false;
  radio->pipe0_reading_address = 0;

}

void setChannel(const RF24 *radio, uint8_t channel) {
  // TODO: This method could take advantage of the 'wide_band' calculation
  // done in setChannel() to require certain channel spacing.

  const uint8_t max_channel = 126;
  write_register(radio, RF_CH, min(channel, max_channel));
}

void setPayloadSize(RF24 *radio, uint8_t size) {
  const uint8_t max_payload_size = 32;
  radio->payload_size = min(size, max_payload_size);
}

uint8_t getPayloadSize(const RF24 *radio) { return radio->payload_size; }

//static const char* const rf24_datarate_e_str_P[] = {"1MBPS", "2MBPS",
//                                                    "250KBPS"};
//
//static const char* const rf24_model_e_str_P[] = {"nRF24L01", "nRF24L01+"};
//
//static const char* const rf24_crclength_e_str_P[] = {"Disabled", "8 bits",
//                                                     "16 bits"};
//
//static const char* const rf24_pa_dbm_e_str_P[] = {
//    "PA_MIN", "PA_LOW",
//    "PA_HIGH",  // (Note: Fixed the "LA_MED" typo from original library)
//    "PA_HIGH"};

//void printDetails(void) {
//  print_status(get_status());
//
//  print_address_register(PSTR("RX_ADDR_P0-1"), RX_ADDR_P0, 2);
//  print_byte_register(PSTR("RX_ADDR_P2-5"), RX_ADDR_P2, 4);
//  print_address_register(PSTR("TX_ADDR"), TX_ADDR);
//
//  print_byte_register(PSTR("RX_PW_P0-6"), RX_PW_P0, 6);
//  print_byte_register(PSTR("EN_AA"), EN_AA);
//  print_byte_register(PSTR("EN_RXADDR"), EN_RXADDR);
//  print_byte_register(PSTR("RF_CH"), RF_CH);
//  print_byte_register(PSTR("RF_SETUP"), RF_SETUP);
//  print_byte_register(PSTR("CONFIG"), CONFIG);
//  print_byte_register(PSTR("DYNPD/FEATURE"), DYNPD, 2);
//
//  printf_P(PSTR("Data Rate\t = %S\r\n"),
//           pgm_read_word(&rf24_datarate_e_str_P[getDataRate()]));
//  printf_P(PSTR("Model\t\t = %S\r\n"),
//           pgm_read_word(&rf24_model_e_str_P[isPVariant()]));
//  printf_P(PSTR("CRC Length\t = %S\r\n"),
//           pgm_read_word(&rf24_crclength_e_str_P[getCRCLength()]));
//  printf_P(PSTR("PA Power\t = %S\r\n"),
//           pgm_read_word(&rf24_pa_dbm_e_str_P[getPALevel()]));
//}

bool begin(RF24* radio) {
  ce(radio, LOW);
  csn(radio, HIGH);

  // Must allow the radio time to settle else configuration bits will not
  // necessarily stick. This is actually only required following power up but
  // some settling time also appears to be required after resets too. For full
  // coverage, we'll always assume the worst. Enabling 16b CRC is by far the
  // most obvious case if the wrong timing is used - or skipped. Technically we
  // require 4.5ms + 14us as a worst case. We'll just call it 5ms for good
  // measure. WARNING: Delay is based on P-variant whereby non-P *may* require
  // different timing.
  HAL_Delay(5);

  // Set 1500uS (minimum for 32B payload in ESB@250KBPS) timeouts, to make
  // testing a little easier WARNING: If this is ever lowered, either 250KBS
  // mode with AA is broken or maximum packet sizes must never be used. See
  // documentation for a more complete explanation.
  write_register(radio, SETUP_RETR, (0x04 << ARD) | (0x0F << ARC));

  // Restore our default PA level
  setPALevel(radio, RF24_PA_MAX);

  // Determine if this is a p or non-p RF24 module and then
  // reset our data rate back to default value. This works
  // because a non-P variant won't allow the data rate to
  // be set to 250Kbps.
  if (setDataRate(radio, RF24_250KBPS)) {
    radio->p_variant = true;
  }

  // Then set the data rate to the slowest (and most reliable) speed supported
  // by all hardware.
  setDataRate(radio, RF24_1MBPS);

  // Initialize CRC and request 2-byte (16bit) CRC
  setCRCLength(radio, RF24_CRC_16);

  // Disable dynamic payloads, to match dynamic_payloads_enabled setting
  write_register(radio, DYNPD, 0);

  // Reset current status
  // Notice reset and flush is the last thing we do
  write_register(radio, STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));

  // Set up default configuration.  Callers can always change it later.
  // This channel should be universally safe and not bleed over into adjacent
  // spectrum.
  setChannel(radio, 76);

  // Flush buffers
  flush_rx(radio);
  flush_tx(radio);

  write_register(radio, CONFIG, (_BV(EN_CRC) | _BV(CRCO) | _BV(MASK_RX_DR) | _BV(MASK_TX_DS) | _BV(MASK_MAX_RT)));

  powerUp(radio);

  uint8_t config_register = read_register(radio, CONFIG);
//  printf("[REG] CONFIG %d\n", config_register);
  return config_register == (_BV(EN_CRC) | _BV(CRCO) | _BV(PWR_UP) | _BV(MASK_RX_DR) | _BV(MASK_TX_DS) | _BV(MASK_MAX_RT)) ? true : false;
}

void startListening(const RF24* radio) {
  write_register(radio, CONFIG, read_register(radio, CONFIG) | _BV(PWR_UP) | _BV(PRIM_RX));
  write_register(radio, STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));

  // Restore the pipe0 adddress, if exists
  if (radio->pipe0_reading_address) {
    write_register_bytes(radio, RX_ADDR_P0, (const uint8_t*)&radio->pipe0_reading_address, 5);
  }

  // Flush buffers
  flush_rx(radio);
  flush_tx(radio);

  // Go!
  ce(radio, HIGH);

  // wait for the radio to come up (130us actually only needed)
  // Edit: No Microsecond HAL delay function, so just wait for 5ms
  HAL_Delay(5);
}

void stopListening(const RF24* radio) {
  ce(radio, LOW);
  flush_tx(radio);
  flush_rx(radio);
}

void powerDown(const RF24 *radio) {
  write_register(radio, CONFIG, read_register(radio, CONFIG) & ~_BV(PWR_UP));
}

void powerUp(const RF24 *radio) {
  write_register(radio, CONFIG, read_register(radio, CONFIG) | _BV(PWR_UP));
}

bool write(RF24 *radio, const void* buf, uint8_t len) {
  bool result = false;

  // Begin the write
  startWrite(radio, buf, len);

  // ------------
  // At this point we could return from a non-blocking write, and then call
  // the rest after an interrupt

  // Instead, we are going to block here until we get TX_DS (transmission
  // completed and ack'd) or MAX_RT (maximum retries, transmission failed).
  // Also, we'll timeout in case the radio is flaky and we get neither.

  // IN the end, the send should be blocking.  It comes back in 60ms worst case,
  // or much faster if I tighted up the retry logic.  (Default settings will be
  // 1500us. Monitor the send
  uint8_t observe_tx;
  uint8_t status;
  uint32_t sent_at = HAL_GetTick();
  const uint32_t timeout = 500;  // ms to wait for timeout
  do {
    status = read_register_bytes(radio, OBSERVE_TX, &observe_tx, 1);
  } while (!(status & (_BV(TX_DS) | _BV(MAX_RT))) &&
           (HAL_GetTick() - sent_at < timeout));

  // The part above is what you could recreate with your own interrupt handler,
  // and then call this when you got an interrupt
  // ------------

  // Call this when you get an interrupt
  // The status tells us three things
  // * The send was successful (TX_DS)
  // * The send failed, too many retries (MAX_RT)
  // * There is an ack packet waiting (RX_DR)
  bool tx_ok, tx_fail;
  whatHappened(radio, &tx_ok, &tx_fail, &(radio->ack_payload_available));

  // printf("%u%u%u\r\n",tx_ok,tx_fail,ack_payload_available);

  result = tx_ok;
//  IF_SERIAL_DEBUG(Serial.print(result ? "...OK." : "...Failed"));

  // Handle the ack packet
  if (radio->ack_payload_available) {
    radio->ack_payload_length = getDynamicPayloadSize(radio);
  }

  // Yay, we are done.

  // Power down
  powerDown(radio);

  // Flush buffers (Is this a relic of past experimentation, and not needed
  // anymore??)
  flush_tx(radio);

  return result;
}

void startWrite(const RF24 *radio, const void* buf, uint8_t len) {
  // Transmitter power-up
  write_register(radio, CONFIG, (read_register(radio, CONFIG) | _BV(PWR_UP)) & ~_BV(PRIM_RX));
  HAL_Delay(5); // No microsecond delay in hal so let's just use 5ms

  // Send the payload
  write_payload(radio, buf, len);

  // Allons!
  ce(radio, HIGH);
  HAL_Delay(5); // No microsecond delay in hal so let's just use 5ms
  ce(radio, LOW);
}

uint8_t getDynamicPayloadSize(const RF24 *radio) {
  uint8_t result = 0;
  uint8_t status = 0;
  uint8_t cmd = R_RX_PL_WID;
  uint8_t dummy = 0xFF;

  csn(radio, LOW);

  HAL_SPI_TransmitReceive(radio->hspi, &cmd, &status, 1, HAL_MAX_DELAY);

  HAL_SPI_TransmitReceive(radio->hspi, &dummy, &result, 1, HAL_MAX_DELAY);

  csn(radio, HIGH);

  return result;
}

bool available_null(const RF24 *radio) { return available(radio, NULL); }

bool available(const RF24 *radio, uint8_t* pipe_num) {
  uint8_t status = get_status(radio);

  // Too noisy, enable if you really want lots o data!!
  // IF_SERIAL_DEBUG(print_status(status));

  bool result = (status & _BV(RX_DR));

  if (result) {
    // If the caller wants the pipe number, include that
    if (pipe_num) *pipe_num = (status >> RX_P_NO) & 0x07;

    // Clear the status bit

    // ??? Should this REALLY be cleared now?  Or wait until we
    // actually READ the payload?

    write_register(radio, STATUS, _BV(RX_DR));

    // Handle ack payload receipt
    if (status & _BV(TX_DS)) {
      write_register(radio, STATUS, _BV(TX_DS));
    }
  }

  return result;
}

bool read(const RF24 *radio, void* buf, uint8_t len) {
  // Fetch the payload
  read_payload(radio, buf, len);

  // was this the last of the data available?
  return read_register(radio, FIFO_STATUS) & _BV(RX_EMPTY);
}

void whatHappened(const RF24 *radio, bool* tx_ok, bool* tx_fail, bool* rx_ready) {
  // Read the status & reset the status in one easy call
  // Or is that such a good idea?
  uint8_t status =
      write_register(radio, STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));

  // Report to the user what happened
  *tx_ok = status & _BV(TX_DS);
  *tx_fail = status & _BV(MAX_RT);
  *rx_ready = status & _BV(RX_DR);
}

void openWritingPipe(RF24 *radio, uint64_t value) {
  // Note that AVR 8-bit uC's store this LSB first, and the NRF24L01(+)
  // expects it LSB first too, so we're good.

  write_register_bytes(radio, RX_ADDR_P0, (uint8_t*)&value, 5);
  write_register_bytes(radio, TX_ADDR, (uint8_t*)&value, 5);

  const uint8_t max_payload_size = 32;
  write_register(radio, RX_PW_P0, min(radio->payload_size, max_payload_size));
}

static const uint8_t child_pipe[] = {
    RX_ADDR_P0, RX_ADDR_P1, RX_ADDR_P2, RX_ADDR_P3, RX_ADDR_P4, RX_ADDR_P5};
static const uint8_t child_payload_size[] = {
    RX_PW_P0, RX_PW_P1, RX_PW_P2, RX_PW_P3, RX_PW_P4, RX_PW_P5};
static const uint8_t child_pipe_enable[] = {ERX_P0, ERX_P1, ERX_P2,
                                                    ERX_P3, ERX_P4, ERX_P5};

void openReadingPipe(RF24 *radio, uint8_t child, uint64_t address) {
  // If this is pipe 0, cache the address.  This is needed because
  // openWritingPipe() will overwrite the pipe 0 address, so
  // startListening() will have to restore it.
  if (child == 0) radio->pipe0_reading_address = address;

  if (child <= 6) {
    // For pipes 2-5, only write the LSB
    if (child < 2) {
      write_register_bytes(radio, child_pipe[child], (const uint8_t*)&address, 5);
    } else {
      write_register_bytes(radio, child_pipe[child], (const uint8_t*)&address, 1);
    }
    write_register(radio, child_payload_size[child], radio->payload_size);

    // Note it would be more efficient to set all of the bits for all open
    // pipes at once.  However, I thought it would make the calling code
    // more simple to do it this way.
    write_register(radio, EN_RXADDR,
                   read_register(radio, EN_RXADDR) |
                       _BV(child_pipe_enable[child]));
  }
}


// AI did this for testing, come back and review later
void openReadingPipe_u8(RF24 *radio, uint8_t child, const uint8_t* address) {
  // 1. If this is pipe 0, cache the address.
  if (child == 0) {
    // Unpack the byte array safely into your uint64_t struct member
    radio->pipe0_reading_address = 0;
    memcpy(&(radio->pipe0_reading_address), address, 5);
  }

  // 2. nRF24L01 has pipes 0 to 5 (child <= 5)
  if (child <= 5) {
    // For pipes 0 and 1, write the full 5 bytes.
    // For pipes 2-5, only write the LSB (1 byte).
    if (child < 2) {
      write_register_bytes(radio, child_pipe[child], address, 5);
    } else {
      write_register_bytes(radio, child_pipe[child], address, 1);
    }

    // Set payload size register
    write_register(radio, child_payload_size[child], radio->payload_size);

    // Enable this specific reading pipe
    write_register(radio, EN_RXADDR,
                   read_register(radio, EN_RXADDR) |
                       _BV(child_pipe_enable[child]));
  }
}


void toggle_features(const RF24 *radio) {
  uint8_t status = 0;
  uint8_t cmd1 = ACTIVATE;
  uint8_t cmd2 = 0x73;
  uint8_t dummy_rx = 0;

  csn(radio, LOW);

  HAL_SPI_TransmitReceive(radio->hspi, &cmd1, &status, 1, HAL_MAX_DELAY);

  HAL_SPI_TransmitReceive(radio->hspi, &cmd2, &dummy_rx, 1, HAL_MAX_DELAY);

  csn(radio, HIGH);

  HAL_Delay(1);	// FIX: Give the nRF24L01 core a brief moment to process the state change
}

void enableDynamicPayloads(RF24 *radio) {
  // Enable dynamic payload throughout the system
  write_register(radio, FEATURE, read_register(radio, FEATURE) | _BV(EN_DPL));

  // If it didn't work, the features are not enabled
  if (!read_register(radio, FEATURE)) {
    // So enable them and try again
    toggle_features(radio);
    write_register(radio, FEATURE, read_register(radio, FEATURE) | _BV(EN_DPL));
  }


  // Enable dynamic payload on all pipes
  //
  // Not sure the use case of only having dynamic payload on certain
  // pipes, so the library does not support it.
  write_register(radio, DYNPD, read_register(radio, DYNPD) | _BV(DPL_P5) | _BV(DPL_P4) |
                            _BV(DPL_P3) | _BV(DPL_P2) | _BV(DPL_P1) |
                            _BV(DPL_P0));

  radio->dynamic_payloads_enabled = true;
}

void enableAckPayload(const RF24 *radio) {
  //
  // enable ack payload and dynamic payload features
  //

  write_register(radio, FEATURE,
                 read_register(radio, FEATURE) | _BV(EN_ACK_PAY) | _BV(EN_DPL));

  // If it didn't work, the features are not enabled
  if (!read_register(radio, FEATURE)) {
    // So enable them and try again
    toggle_features(radio);
    write_register(radio, FEATURE,
                   read_register(radio, FEATURE) | _BV(EN_ACK_PAY) | _BV(EN_DPL));
  }

  //
  // Enable dynamic payload on pipes 0 & 1
  //

  write_register(radio, DYNPD, read_register(radio, DYNPD) | _BV(DPL_P1) | _BV(DPL_P0));
}

void writeAckPayload(const RF24 *radio, uint8_t pipe, const void* buf, uint8_t len) {
  const uint8_t* current = (const uint8_t*)buf;
  uint8_t cmd = W_ACK_PAYLOAD | (pipe & 0x07);
  uint8_t status = 0;

  csn(radio, LOW);

  HAL_SPI_TransmitReceive(radio->hspi, &cmd, &status, 1, HAL_MAX_DELAY);

  const uint8_t max_payload_size = 32;
  uint8_t data_len = min(len, max_payload_size);
  uint8_t dummy_rx[data_len];

  if (data_len > 0) {
    HAL_SPI_TransmitReceive(radio->hspi, (uint8_t*)current, dummy_rx, data_len, HAL_MAX_DELAY);
  }

  csn(radio, HIGH);
}

bool isAckPayloadAvailable(RF24 *radio) {
  bool result = radio->ack_payload_available;
  radio->ack_payload_available = false;
  return result;
}

bool isPVariant(const RF24 *radio) { return radio->p_variant; }

void setAutoAck(const RF24 *radio, bool enable) {
  if (enable)
    write_register(radio, EN_AA, 0x3F);
  else
    write_register(radio, EN_AA, 0);
}

void setAutoAck_pipe(const RF24 *radio, uint8_t pipe, bool enable) {
  if (pipe <= 6) {
    uint8_t en_aa = read_register(radio, EN_AA);
    if (enable) {
      en_aa |= _BV(pipe);
    } else {
      en_aa &= ~_BV(pipe);
    }
    write_register(radio, EN_AA, en_aa);
  }
}

bool testCarrier(const RF24 *radio) { return (read_register(radio, CD) & 1); }

bool testRPD(const RF24 *radio) { return (read_register(radio, RPD) & 1); }

void setPALevel(const RF24 *radio, rf24_pa_dbm_e level) {
  uint8_t setup = read_register(radio, RF_SETUP);
  setup &= ~(_BV(RF_PWR_LOW) | _BV(RF_PWR_HIGH));

  // switch uses RAM (evil!)
  if (level == RF24_PA_MAX) {
    setup |= (_BV(RF_PWR_LOW) | _BV(RF_PWR_HIGH));
  } else if (level == RF24_PA_HIGH) {
    setup |= _BV(RF_PWR_HIGH);
  } else if (level == RF24_PA_LOW) {
    setup |= _BV(RF_PWR_LOW);
  } else if (level == RF24_PA_MIN) {
    // nothing
  } else if (level == RF24_PA_ERROR) {
    // On error, go to maximum PA
    setup |= (_BV(RF_PWR_LOW) | _BV(RF_PWR_HIGH));
  }

  write_register(radio, RF_SETUP, setup);
}

rf24_pa_dbm_e getPALevel(const RF24 *radio) {
  rf24_pa_dbm_e result = RF24_PA_ERROR;
  uint8_t power =
      read_register(radio, RF_SETUP) & (_BV(RF_PWR_LOW) | _BV(RF_PWR_HIGH));

  // switch uses RAM (evil!)
  if (power == (_BV(RF_PWR_LOW) | _BV(RF_PWR_HIGH))) {
    result = RF24_PA_MAX;
  } else if (power == _BV(RF_PWR_HIGH)) {
    result = RF24_PA_HIGH;
  } else if (power == _BV(RF_PWR_LOW)) {
    result = RF24_PA_LOW;
  } else {
    result = RF24_PA_MIN;
  }

  return result;
}

bool setDataRate(RF24 *radio, rf24_datarate_e speed) {
  bool result = false;
  uint8_t setup = read_register(radio, RF_SETUP);

  // HIGH and LOW '00' is 1Mbs - our default
  radio->wide_band = false;
  setup &= ~(_BV(RF_DR_LOW) | _BV(RF_DR_HIGH));
  if (speed == RF24_250KBPS) {
    // Must set the RF_DR_LOW to 1; RF_DR_HIGH (used to be RF_DR) is already 0
    // Making it '10'.
    radio->wide_band = false;
    setup |= _BV(RF_DR_LOW);
  } else {
    // Set 2Mbs, RF_DR (RF_DR_HIGH) is set 1
    // Making it '01'
    if (speed == RF24_2MBPS) {
      radio->wide_band = true;
      setup |= _BV(RF_DR_HIGH);
    } else {
      // 1Mbs
      radio->wide_band = false;
    }
  }
  write_register(radio, RF_SETUP, setup);

  // Verify our result
  if (read_register(radio, RF_SETUP) == setup) {
    result = true;
  } else {
    radio->wide_band = false;
  }

  return result;
}

rf24_datarate_e getDataRate(const RF24 *radio) {
  rf24_datarate_e result;
  uint8_t dr = read_register(radio, RF_SETUP) & (_BV(RF_DR_LOW) | _BV(RF_DR_HIGH));

  // switch uses RAM (evil!)
  // Order matters in our case below
  if (dr == _BV(RF_DR_LOW)) {
    // '10' = 250KBPS
    result = RF24_250KBPS;
  } else if (dr == _BV(RF_DR_HIGH)) {
    // '01' = 2MBPS
    result = RF24_2MBPS;
  } else {
    // '00' = 1MBPS
    result = RF24_1MBPS;
  }
  return result;
}

void setCRCLength(const RF24 *radio, rf24_crclength_e length) {
  uint8_t config = read_register(radio, CONFIG) & ~(_BV(CRCO) | _BV(EN_CRC));

  // switch uses RAM (evil!)
  if (length == RF24_CRC_DISABLED) {
    // Do nothing, we turned it off above.
  } else if (length == RF24_CRC_8) {
    config |= _BV(EN_CRC);
  } else {
    config |= _BV(EN_CRC);
    config |= _BV(CRCO);
  }
  write_register(radio, CONFIG, config);
}

rf24_crclength_e getCRCLength(const RF24 *radio) {
  rf24_crclength_e result = RF24_CRC_DISABLED;
  uint8_t config = read_register(radio, CONFIG) & (_BV(CRCO) | _BV(EN_CRC));

  if (config & _BV(EN_CRC)) {
    if (config & _BV(CRCO))
      result = RF24_CRC_16;
    else
      result = RF24_CRC_8;
  }

  return result;
}

void disableCRC(const RF24 *radio) {
  uint8_t disable = read_register(radio, CONFIG) & ~_BV(EN_CRC);
  write_register(radio, CONFIG, disable);
}

void setRetries(const RF24 *radio, uint8_t delay, uint8_t count) {
  write_register(radio, SETUP_RETR, (delay & 0xf) << ARD | (count & 0xf) << ARC);
}
