// Author: Edward Eldridge
// Program: SHA-256 Algorithm implentation in C
// Resources: https://github.com/EddieEldridge/SHA256-in-C/blob/master/README.md
// Section Reference: https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf

// SHA-256 Algorithm implementation for STM32
// Based on NIST FIPS 180-4 standard

#include <stdint.h>

// Helper macros for bit operations
#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define SHR(x, n) ((x) >> (n))

// SHA-256 functions
static uint32_t sig0(uint32_t x) {
  return (ROTR(x, 7)) ^ (ROTR(x, 18)) ^ (SHR(x, 3));
}

static uint32_t sig1(uint32_t x) {
  return (ROTR(x, 17)) ^ (ROTR(x, 19)) ^ (SHR(x, 10));
}

static uint32_t SIG0(uint32_t x) {
  return (ROTR(x, 2)) ^ (ROTR(x, 13)) ^ (ROTR(x, 22));
}

static uint32_t SIG1(uint32_t x) {
  return (ROTR(x, 6)) ^ (ROTR(x, 11)) ^ (ROTR(x, 25));
}

static uint32_t Ch(uint32_t x, uint32_t y, uint32_t z) {
  return ((x & y) ^ (~x & z));
}

static uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) {
  return ((x & y) ^ (x & z) ^ (y & z));
}

// K constants (Section 4.2.2)
static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
    0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
    0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
    0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

// Initial hash values (Section 5.3.3)
static const uint32_t H0[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                               0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

// Byte swap for little-endian systems
static uint32_t byteSwap32(uint32_t x) {
  return ((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) |
         ((x) << 24);
}

// Main SHA-256 hashing function
void sha256_hash(const uint8_t *data, uint32_t len, uint32_t hash[8]) {
  uint32_t W[64];
  uint32_t a, b, c, d, e, f, g, h;
  uint32_t T1, T2;

  // Initialize hash values
  for (int i = 0; i < 8; i++) {
    hash[i] = H0[i];
  }

  // Process each 512-bit block
  uint32_t blocks = len / 64;
  for (uint32_t i = 0; i <= blocks; i++) {
    uint32_t remaining = len - i * 64;
    uint32_t block_size = (remaining >= 64) ? 64 : remaining;

    // Prepare message schedule
    for (uint32_t j = 0; j < 16; j++) {
      if (j * 4 + 4 <= block_size) {
        W[j] = (data[i * 64 + j * 4] << 24) | (data[i * 64 + j * 4 + 1] << 16) |
               (data[i * 64 + j * 4 + 2] << 8) | (data[i * 64 + j * 4 + 3]);
        W[j] = byteSwap32(W[j]); // Convert to big-endian
      } else if (j * 4 < block_size) {
        // Handle partial word at end of message
        uint32_t word = 0;
        for (uint32_t k = 0; k < block_size - j * 4; k++) {
          word |= data[i * 64 + j * 4 + k] << (24 - 8 * k);
        }
        W[j] = byteSwap32(word);
      } else if (i == blocks) {
        // Padding
        if (block_size < 56) {
          // Single padding block
          if (j == block_size / 4) {
            // Add 1 followed by zeros
            uint32_t padding = 0x80 << (24 - 8 * (block_size % 4));
            W[j] = byteSwap32(padding);
          } else if (j == 14) {
            // Add length in bits (big-endian)
            W[j] = byteSwap32((uint32_t)(len * 8 >> 32));
          } else if (j == 15) {
            W[j] = byteSwap32((uint32_t)(len * 8));
          } else {
            W[j] = 0;
          }
        } else {
          // Need two padding blocks
          if (j < 14) {
            W[j] = 0;
          } else if (j == 14) {
            W[j] = byteSwap32((uint32_t)(len * 8 >> 32));
          } else if (j == 15) {
            W[j] = byteSwap32((uint32_t)(len * 8));
          }
        }
      } else {
        W[j] = 0;
      }
    }

    // Extend the first 16 words into the remaining 48 words
    for (uint32_t j = 16; j < 64; j++) {
      W[j] = sig1(W[j - 2]) + W[j - 7] + sig0(W[j - 15]) + W[j - 16];
    }

    // Initialize working variables
    a = hash[0];
    b = hash[1];
    c = hash[2];
    d = hash[3];
    e = hash[4];
    f = hash[5];
    g = hash[6];
    h = hash[7];

    // Compression function main loop
    for (uint32_t j = 0; j < 64; j++) {
      T1 = h + SIG1(e) + Ch(e, f, g) + K[j] + W[j];
      T2 = SIG0(a) + Maj(a, b, c);
      h = g;
      g = f;
      f = e;
      e = d + T1;
      d = c;
      c = b;
      b = a;
      a = T1 + T2;
    }

    // Update hash values
    hash[0] += a;
    hash[1] += b;
    hash[2] += c;
    hash[3] += d;
    hash[4] += e;
    hash[5] += f;
    hash[6] += g;
    hash[7] += h;
  }
}
