#include "htpa32_eeprom.h"
#include <string.h>

static void write_u16(uint8_t *buf, uint16_t v)
{
  buf[0] = (uint8_t) (v);
  buf[1] = (uint8_t) (v >> 8);
}

static void write_u32(uint8_t *buf, uint32_t v)
{
  buf[0] = (uint8_t) (v);
  buf[1] = (uint8_t) (v >> 8);
  buf[2] = (uint8_t) (v >> 16);
  buf[3] = (uint8_t) (v >> 24);
}

static uint16_t read_u16(const uint8_t *buf)
{
  return (uint16_t) (((uint16_t) buf[0]) | ((uint16_t) buf[1] << 8));
}

static uint32_t read_u32(const uint8_t *buf)
{
  return ((uint32_t) buf[0]) | (((uint32_t) buf[1]) << 8) | (((uint32_t) buf[2]) << 16) | (((uint32_t) buf[3]) << 24);
}

static void write_float(uint8_t *buf, float f)
{
  uint32_t tmp;
  memcpy (&tmp, &f, sizeof(float));
  write_u32 (buf, tmp);
}

static float read_float(const uint8_t *buf)
{
  uint32_t tmp = read_u32 (buf);
  float f;
  memcpy (&f, &tmp, sizeof(float));
  return f;
}

size_t htpa32_serialize(const htpa32_eeprom_t *src, uint8_t *buffer, size_t buffer_size)
{
  if (buffer_size < HTPA32_EEPROM_SERIALIZED_SIZE)
    return 0;

  uint8_t *p = buffer;

#define W_U8(v)      (*p++ = (uint8_t)(v))
#define W_U16(v)     do { write_u16(p, (uint16_t)(v)); p += 2; } while(0)
#define W_U32(v)     do { write_u32(p, (uint32_t)(v)); p += 4; } while(0)
#define W_F32(v)     do { write_float(p, (float)(v)); p += 4; } while(0)

  W_F32(src->pixc_min);
  W_F32(src->pixc_max);

  W_U8(src->grad_scale);
  W_U16(src->tablenumber);

  W_U8(src->epsilon);
  W_U8(src->mbit_calib);
  W_U8(src->bias_calib);
  W_U8(src->clk_calib);
  W_U8(src->bpa_calib);
  W_U8(src->pu_calib);
  W_U8(src->arraytype);

  W_U16(src->vddth1);
  W_U16(src->vddth2);

  W_F32(src->ptat_gradient);
  W_F32(src->ptat_offset);

  W_U16(src->ptat_th1);
  W_U16(src->ptat_th2);

  W_U8(src->vddsc_gradient);
  W_U8(src->vddsc_offset);
  W_U8(src->global_offset);

  W_U16(src->global_gain);

  W_U8(src->mbit_user);
  W_U8(src->bias_user);
  W_U8(src->clk_user);
  W_U8(src->bpa_user);
  W_U8(src->pu_user);

  W_U32(src->device_id);
  W_U8(src->norof_deadpix);

  for (int i = 0; i < 24; i++)
    W_U16(src->deadpix_addr[i]);
  for (int i = 0; i < 12; i++)
    W_U16(src->deadpix_mask[i]);

  for (int i = 0; i < 256; i++)
    W_U16((uint16_t )src->vddcomp_gradient[i]);
  for (int i = 0; i < 256; i++)
    W_U16((uint16_t )src->vddcomp_offset[i]);

  for (int i = 0; i < 1024; i++)
    W_U16((uint16_t )src->th_gradient[i]);
  for (int i = 0; i < 1024; i++)
    W_U16((uint16_t )src->th_offset[i]);

  for (int i = 0; i < 1024; i++)
    W_U16(src->p[i]);

#undef W_U8
#undef W_U16
#undef W_U32
#undef W_F32

  return HTPA32_EEPROM_SERIALIZED_SIZE;
}

int htpa32_deserialize(htpa32_eeprom_t *dst, const uint8_t *buffer, size_t buffer_size)
{
  if (buffer_size < HTPA32_EEPROM_SERIALIZED_SIZE)
    return -1;

  const uint8_t *p = buffer;

#define R_U8(v)      ((v) = *p++)
#define R_U16(v)     do { (v) = read_u16(p); p += 2; } while(0)
#define R_U32(v)     do { (v) = read_u32(p); p += 4; } while(0)
#define R_F32(v)     do { (v) = read_float(p); p += 4; } while(0)

  R_F32(dst->pixc_min);
  R_F32(dst->pixc_max);

  R_U8(dst->grad_scale);
  R_U16(dst->tablenumber);

  R_U8(dst->epsilon);
  R_U8(dst->mbit_calib);
  R_U8(dst->bias_calib);
  R_U8(dst->clk_calib);
  R_U8(dst->bpa_calib);
  R_U8(dst->pu_calib);
  R_U8(dst->arraytype);

  R_U16(dst->vddth1);
  R_U16(dst->vddth2);

  R_F32(dst->ptat_gradient);
  R_F32(dst->ptat_offset);

  R_U16(dst->ptat_th1);
  R_U16(dst->ptat_th2);

  R_U8(dst->vddsc_gradient);
  R_U8(dst->vddsc_offset);
  R_U8(dst->global_offset);

  R_U16(dst->global_gain);

  R_U8(dst->mbit_user);
  R_U8(dst->bias_user);
  R_U8(dst->clk_user);
  R_U8(dst->bpa_user);
  R_U8(dst->pu_user);

  R_U32(dst->device_id);
  R_U8(dst->norof_deadpix);

  for (int i = 0; i < 24; i++)
    R_U16(dst->deadpix_addr[i]);
  for (int i = 0; i < 12; i++)
    R_U16(dst->deadpix_mask[i]);

  for (int i = 0; i < 256; i++)
    R_U16(dst->vddcomp_gradient[i]);
  for (int i = 0; i < 256; i++)
    R_U16(dst->vddcomp_offset[i]);

  for (int i = 0; i < 1024; i++)
    R_U16(dst->th_gradient[i]);
  for (int i = 0; i < 1024; i++)
    R_U16(dst->th_offset[i]);

  for (int i = 0; i < 1024; i++)
    R_U16(dst->p[i]);

#undef R_U8
#undef R_U16
#undef R_U32
#undef R_F32

  return 0;
}
