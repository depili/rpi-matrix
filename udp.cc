/* -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
Compile in rpi-rgb-led-matrix directory.
make  # compile library
g++ -Wall -O3 -g -Iinclude simple-udp.cc -o simple-udp -Llib -lrgbmatrix -lrt -lm -lpthread
*/

#include "led-matrix.h"
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

const int kListenPort = 9999;

using rgb_matrix::RGBMatrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

static rgb_matrix::FrameCanvas *FillFramebuffer(rgb_matrix::RGBMatrix *matrix, rgb_matrix::FrameCanvas *canvas,
                                    const char *buffer) {
  const int width = matrix->width();
  const int height = matrix->height();

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int r = *buffer++;
      int g = *buffer++;
      int b = *buffer++;
      canvas->SetPixel(x, y, r, g, b);
    }
  }
  return matrix->SwapOnVSync(canvas);
}


int main(int argc, char **argv) {
  int port = kListenPort;
  
  // Set some defaults
  RGBMatrix::Options my_defaults;
  my_defaults.chain_length = 4;
  my_defaults.rows = 32;
  my_defaults.show_refresh_rate = true;
  rgb_matrix::RuntimeOptions runtime_defaults;
  runtime_defaults.drop_privileges = 1;
  RGBMatrix *matrix = rgb_matrix::CreateMatrixFromFlags(&argc, &argv,
                                                        &my_defaults,
                                                        &runtime_defaults);
  if (matrix == NULL) {
    PrintMatrixFlags(stderr, my_defaults, runtime_defaults);
    return 1;
  }

  rgb_matrix::FrameCanvas *swap_buffer = matrix->CreateFrameCanvas();
  const int framebuffer_size = matrix->width() * matrix->height() * 3;
  char *const packet_buffer = new char[framebuffer_size];

  struct sigaction sa;
  sa.sa_handler = InterruptHandler;
  sa.sa_flags = SA_RESETHAND | SA_NODEFER;
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGINT,  &sa, NULL);

  int s;
  if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
      perror("creating UDP socket");
      exit(1);
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);
  if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
      perror("bind");
      exit(1);
  }

  matrix->Clear();

  while (!interrupt_received) {
    const ssize_t buffer_bytes = recvfrom(s, packet_buffer, framebuffer_size,
                                          0, NULL, 0);
    if (interrupt_received)
      break;

    if (buffer_bytes < 1)
      continue;
    
    if (buffer_bytes == framebuffer_size) {
      swap_buffer = FillFramebuffer(matrix, swap_buffer, packet_buffer);
    } else {
      continue;
    }
  }

  delete matrix;   // Make sure to delete it in the end.
}