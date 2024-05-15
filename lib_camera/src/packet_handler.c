// Copyright 2023-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdio.h>


#include <xcore/assert.h>
#include <xcore/select.h>
#include <print.h>

#include "packet_handler.h"
#include "sensor_control.h"

#define FRAMES_TO_STOP 30

unsigned only_once = 1;

// Contains the local state info for the packet handler thread.
static frame_state_t ph_state = {
    1,  // wait_for_frame_start
    0,  // frame_number
    0,  // in_line_number
    0   // out_line_number
};

// -------- Error handling --------
static
void handle_unknown_packet(
  mipi_data_type_t data_type)
{
  xassert(data_type < 0x3F && "Packet non valid");
}

static
void handle_no_expected_lines()
{
  if (ph_state.in_line_number >= SENSOR_RAW_IMAGE_HEIGHT_PIXELS) {
    // We've received more lines of image data than we expected.
#ifdef ASSERT_ON_TOO_MANY_LINES
    xassert(0 && "Recieved too many lines");
#endif
  }
}

// -------- Frame handling --------
static
void handle_packet(
  const mipi_packet_t* pkt,
  chanend_t c_control)
{
  // Definitions
  const mipi_header_t header = pkt->header;
  const mipi_data_type_t data_type = MIPI_GET_DATA_TYPE(header);

  // Wait for a clean frame
  if (ph_state.wait_for_frame_start
    && data_type != MIPI_DT_FRAME_START) return;

  // Handle packets depending on their type
  switch (data_type)
  {
  case MIPI_DT_FRAME_START:
    printstrln("SOF");
    ph_state.wait_for_frame_start = 0;
    ph_state.in_line_number = 0;
    ph_state.out_line_number = 0;
    ph_state.frame_number++;
    break;

  case MIPI_EXPECTED_FORMAT:
    handle_no_expected_lines();
    ph_state.in_line_number++;
    break;

  case MIPI_DT_FRAME_END:
    printstrln("EOF");
    if (ph_state.frame_number >= FRAMES_TO_STOP && only_once) {
      printstrln("-----> Stopping sensor stream, 5 seconds");
      uint32_t encoded_cmd = ENCODE(SENSOR_STREAM_STOP, 0);
      chan_out_word(c_control, encoded_cmd);
      chan_in_word(c_control);
      //only_once = 0;
    }
    break;

  default:
    handle_unknown_packet(data_type);
    break;
  }
}


// -------- Main packet handler thread --------
void mipi_packet_handler(
  streaming_chanend_t c_pkt,
  streaming_chanend_t c_ctrl,
  chanend_t c_control)
{

  __attribute__((aligned(8)))
    mipi_packet_t packet_buffer[MIPI_PKT_BUFFER_COUNT];
  mipi_packet_t* pkt;
  unsigned pkt_idx = 0;

  // Give the MIPI packet receiver a first buffer
  s_chan_out_word(c_pkt, (unsigned)&packet_buffer[pkt_idx]);

  while (1) {
    pkt_idx = (pkt_idx + 1) & (MIPI_PKT_BUFFER_COUNT - 1);

    // Swap buffers with the receiver thread. Give it the next buffer
    // to fill and take the last filled buffer from it. 
    pkt = (mipi_packet_t*)s_chan_in_word(c_pkt);
    
    // send info to MipiReciever
    s_chan_out_word(c_pkt, (unsigned)&packet_buffer[pkt_idx]);
    // Get information regarding the packet
    // const mipi_header_t header = pkt->header;
    // const mipi_data_type_t data_type = MIPI_GET_DATA_TYPE(header);

    // Process the packet 
    // unsigned time_start = measure_time();
    handle_packet(pkt, c_control);
    // unsigned time_proc = measure_time() - time_start;
  }
}
