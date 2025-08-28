#include <pjmedia-audiodev/audiodev_imp.h>
#include <pj/assert.h>
#include <pj/log.h>
#include <pj/os.h>

#include "driver/i2s.h"

#define THIS_FILE "esp32_i2s_dev.c"

// Feather V2 I2S pin mapping
#define I2S_BCK   27  // BCLK -> MAX98357 BCLK & INMP441 BCLK
#define I2S_WS    25  // LRCLK -> MAX98357 LRC & INMP441 LRC
#define I2S_DOUT  22  // DIN -> MAX98357
#define I2S_DIN   32  // DOUT <- INMP441

#define ESP32_I2S_SAMPLE_RATE 16000
#define ESP32_I2S_CHANNELS    1
#define ESP32_I2S_BITS        16

/* Forward decls */
static pj_status_t factory_init(pjmedia_aud_dev_factory *f);
static pj_status_t factory_destroy(pjmedia_aud_dev_factory *f);
static unsigned    factory_get_dev_count(pjmedia_aud_dev_factory *f);
static pj_status_t factory_get_dev_info(pjmedia_aud_dev_factory *f, unsigned index,
                                        pjmedia_aud_dev_info *info);
static pj_status_t factory_default_param(pjmedia_aud_dev_factory *f, unsigned index,
                                         pjmedia_aud_param *param);
static pj_status_t factory_create_stream(pjmedia_aud_dev_factory *f,
                                         const pjmedia_aud_param *param,
                                         pjmedia_aud_rec_cb rec_cb,
                                         pjmedia_aud_play_cb play_cb,
                                         void *user_data,
                                         pjmedia_aud_stream **p_strm);

/* Factory ops */
static pjmedia_aud_dev_factory_op factory_ops =
{
    &factory_init,
    &factory_destroy,
    &factory_get_dev_count,
    &factory_get_dev_info,
    &factory_default_param,
    &factory_create_stream
};

/* Stream structure */
typedef struct esp32_i2s_strm
{
    pjmedia_aud_stream base;
    pj_pool_t         *pool;
    pjmedia_aud_param  param;

    pjmedia_aud_rec_cb rec_cb;
    pjmedia_aud_play_cb play_cb;
    void              *user_data;

    pj_bool_t          running;
    pj_thread_t       *thread;
} esp32_i2s_strm;

/* ===== Factory implementation ===== */
pjmedia_aud_dev_factory* pjmedia_esp32_i2s_factory(pj_pool_factory *pf)
{
    pjmedia_aud_dev_factory *f;
    f = PJ_POOL_ZALLOC_T(pf->pool, pjmedia_aud_dev_factory);
    f->op = &factory_ops;
    return f;
}

static pj_status_t factory_init(pjmedia_aud_dev_factory *f)
{
    PJ_LOG(4,(THIS_FILE, "ESP32 I2S audio factory initialized"));
    return PJ_SUCCESS;
}

static pj_status_t factory_destroy(pjmedia_aud_dev_factory *f)
{
    PJ_LOG(4,(THIS_FILE, "ESP32 I2S audio factory destroyed"));
    return PJ_SUCCESS;
}

static unsigned factory_get_dev_count(pjmedia_aud_dev_factory *f)
{
    return 1;
}

static pj_status_t factory_get_dev_info(pjmedia_aud_dev_factory *f, unsigned index,
                                        pjmedia_aud_dev_info *info)
{
    pj_bzero(info, sizeof(*info));
    pj_ansi_strcpy(info->name, "ESP32 I2S Feather");
    info->input_count = 1;
    info->output_count = 1;
    info->default_samples_per_sec = ESP32_I2S_SAMPLE_RATE;
    return PJ_SUCCESS;
}

static pj_status_t factory_default_param(pjmedia_aud_dev_factory *f, unsigned index,
                                         pjmedia_aud_param *param)
{
    pj_bzero(param, sizeof(*param));
    param->dir = PJMEDIA_DIR_CAPTURE_PLAYBACK;
    param->clock_rate = ESP32_I2S_SAMPLE_RATE;
    param->channel_count = ESP32_I2S_CHANNELS;
    param->samples_per_frame = ESP32_I2S_SAMPLE_RATE / 50; // 20ms frame
    param->bits_per_sample = ESP32_I2S_BITS;
    return PJ_SUCCESS;
}

/* Audio thread loop */
static int audio_thread(void *arg)
{
    esp32_i2s_strm *strm = (esp32_i2s_strm*)arg;
    int frame_size = strm->param.samples_per_frame * 2; // 16-bit mono

    void *buffer = pj_pool_alloc(strm->pool, frame_size);

    while (strm->running) {
        size_t bytes_read = 0, bytes_written = 0;

        /* Capture (from INMP441) */
        if (strm->rec_cb) {
            i2s_read(I2S_NUM_0, buffer, frame_size, &bytes_read, portMAX_DELAY);
            if (bytes_read > 0) {
                (*strm->rec_cb)(strm->user_data, buffer, (unsigned)(bytes_read/2));
            }
        }

        /* Playback (to MAX98357) */
        if (strm->play_cb) {
            (*strm->play_cb)(strm->user_data, buffer, strm->param.samples_per_frame);
            i2s_write(I2S_NUM_0, buffer, frame_size, &bytes_written, portMAX_DELAY);
        }
    }

    return 0;
}

static pj_status_t factory_create_stream(pjmedia_aud_dev_factory *f,
                                         const pjmedia_aud_param *param,
                                         pjmedia_aud_rec_cb rec_cb,
                                         pjmedia_aud_play_cb play_cb,
                                         void *user_data,
                                         pjmedia_aud_stream **p_strm)
{
    pj_pool_t *pool = pj_pool_create(pj_pool_factory_default_policy, "i2sstrm", 512, 512, NULL);
    esp32_i2s_strm *strm = PJ_POOL_ZALLOC_T(pool, esp32_i2s_strm);

    strm->pool = pool;
    strm->param = *param;
    strm->rec_cb = rec_cb;
    strm->play_cb = play_cb;
    strm->user_data = user_data;
    strm->running = PJ_TRUE;

    /* Configure I2S hardware */
    i2s_config_t cfg = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX,
        .sample_rate = ESP32_I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 6,
        .dma_buf_len = 256,
        .use_apll = false
    };
    i2s_driver_install(I2S_NUM_0, &cfg, 0, NULL);

    i2s_pin_config_t pins = {
        .bck_io_num = I2S_BCK,
        .ws_io_num  = I2S_WS,
        .data_out_num = I2S_DOUT,
        .data_in_num  = I2S_DIN,
        .mck_io_num = I2S_PIN_NO_CHANGE
    };
    i2s_set_pin(I2S_NUM_0, &pins);

    /* Start audio thread */
    pj_thread_create(pool, "i2s_audio", &audio_thread, strm, 0, 0, &strm->thread);

    *p_strm = &strm->base;
    return PJ_SUCCESS;
}
