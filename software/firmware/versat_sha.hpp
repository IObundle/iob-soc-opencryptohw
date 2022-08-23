#ifndef INCLUDED_VERSAT_SHA_HPP
#define INCLUDED_VERSAT_SHA_HPP

#include "versat.hpp"

#include "unitWrapper.hpp"
#include "unitVerilogWrappers.hpp"

Accelerator* InstantiateSHA(Versat* versat);

static uint load_bigendian_32(const uint8_t *x);

static void store_bigendian_32(uint8_t *x, uint64_t u);

static size_t versat_crypto_hashblocks_sha256(const uint8_t *in, size_t inlen);

void versat_sha256(uint8_t *out, const uint8_t *in, size_t inlen);
#endif // INCLUDED_VERSAT_SHA_HPP
