#include "decoder.h"

#define MODEL_NAME "FlowService FT-01 water meter"

static int
ft01_callback(r_device *decoder, bitbuffer_t *bitbuffer)
{

    if (bitbuffer->bits_per_row[0] != 105) {
        return DECODE_ABORT_LENGTH;
    }

    uint8_t* msg = bitbuffer->bb[0];

    uint8_t d[14];

    for (size_t i = 0; i < sizeof(d); i++) {
        d[i] = bitrow_get_byte(msg,  i * 8);
        d[i] = reverse8(d[i]);
    }

    if (d[0] != 0x11 || d[1] != 0x20 || d[13] != 0x01) {
        printf("decode failed! %x %x %x\r\n", d[0], d[1], d[13]);
        return DECODE_FAIL_SANITY;
    }

    uint16_t address = (d[2] << 8) + d[3];

    uint32_t cold_raw = (d[4] << 0) | (d[5] << 8) | (d[6] << 16);
    uint32_t warm_raw = (d[7] << 0) | (d[8] << 8) | (d[9] << 16);

    float warm = warm_raw / 100.0;
    float cold = cold_raw / 100.0;

    data_t *data = data_make(
            "model", "", DATA_STRING, MODEL_NAME,
            "address", "Address", DATA_FORMAT, "%05d", DATA_INT, address,
            "warm_m3", "Warm water m3", DATA_FORMAT, "%.3f", DATA_DOUBLE, warm,
            "cold_m3", "Cold water m3", DATA_FORMAT, "%.3f", DATA_DOUBLE, cold,
            NULL);

    decoder_output_data(decoder, data);

    return 1;
}

static char *output_fields[] = {
        "model",
        "address",
        "warm_m3",
        "cold_m3",
        NULL,
};

r_device ft01 = {
        .name        = MODEL_NAME,
        .modulation  = OOK_PULSE_PWM,
        .short_width = 400,
        .long_width  = 800,
        .gap_limit   = 1000,
        .reset_limit = 4000,
        .decode_fn   = &ft01_callback,
        .disabled    = 0,
        .fields      = output_fields,
};
