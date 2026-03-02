#ifndef HTPA32_EEPROM_H
#define HTPA32_EEPROM_H

#include <stdint.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HTPA32_EEPROM_VERSION   1
#define HTPA32_EEPROM_SERIALIZED_SIZE  7289u

typedef struct
{
  float pixc_min;
  float pixc_max;

  uint8_t grad_scale;
  uint16_t tablenumber;

  uint8_t epsilon;
  uint8_t mbit_calib;
  uint8_t bias_calib;
  uint8_t clk_calib;
  uint8_t bpa_calib;
  uint8_t pu_calib;
  uint8_t arraytype;

  uint16_t vddth1;
  uint16_t vddth2;

  float ptat_gradient;
  float ptat_offset;

  uint16_t ptat_th1;
  uint16_t ptat_th2;

  uint8_t vddsc_gradient;
  uint8_t vddsc_offset;
  uint8_t global_offset;

  uint16_t global_gain;

  uint8_t mbit_user;
  uint8_t bias_user;
  uint8_t clk_user;
  uint8_t bpa_user;
  uint8_t pu_user;

  uint32_t device_id;
  uint8_t norof_deadpix;

  uint16_t deadpix_addr[24];
  uint16_t deadpix_mask[12];

  int16_t vddcomp_gradient[256];
  int16_t vddcomp_offset[256];

  int16_t th_gradient[1024];
  int16_t th_offset[1024];

  uint16_t p[1024];

} htpa32_eeprom_t;

/**
 *  Serialize the eeprom struct into buffer.
 *  @param[in]  htpa32 array to serialize
 *  @param[out] Buffer to write the serialized data to
 *  @param[in]  Buffer size for safety
 *  @return     Number of bytes written. Returns 0 if the buffer is too small.
 **/
size_t htpa32_serialize(const htpa32_eeprom_t *src, uint8_t *buffer, size_t buffer_size);

/**
 *  Deserialize the eeprom struct from buffer.
 *  @param[out] htpa32 array to deserialize the buffer into
 *  @param[out] Buffer to deserialize the data from
 *  @param[in]  Buffer size for safety
 *  @return     Returns 0 on success. Returns -1 if the buffer is too small.
 **/
int htpa32_deserialize(htpa32_eeprom_t *dst, const uint8_t *buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif
