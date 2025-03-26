// ISP settings
#define AE_MARGIN 0.1 // default marging for the auto exposure error
#define AE_INITIAL_EXPOSURE 35 // initial exposure value

static inline
int8_t csign(float x)
{
    return (x > 0) - (x < 0);
}

static inline
uint8_t AE_is_adjusted(float sk)
{
    return (sk < AE_MARGIN && sk > -AE_MARGIN) ? 1 : 0;
}

static
uint8_t AE_control_exposure(
    statistics_t* global_stats,
    chanend_t c_control)
{
    // Initial exposure
    static uint8_t new_exp = AE_INITIAL_EXPOSURE;
    static uint8_t skip_ae_control = 0; // if too dark for a certain frames, skip AE control

    // Compute skewness and adjust exposure if needed
    float sk = stats_compute_mean_skewness(global_stats);
    if (AE_is_adjusted(sk)) {
        return 1;
    }
    else {
        // Adjust exposure
        new_exp = AE_compute_new_exposure((float)new_exp, sk);
        // Send new exposure
        uint32_t encoded_cmd = ENCODE(SENSOR_SET_EXPOSURE, new_exp);
        chan_out_word(c_control, encoded_cmd);
        chan_in_word(c_control);
        // Skip AE control if too dark
        if (new_exp > 70) {
            skip_ae_control++;
            if (skip_ae_control > 5) {
                skip_ae_control = 0;
                return 1;
            }
        }
    }
    return 0;
}

void camera_isp_ae(image_cfg_t* image)
{
    const size_t img_size = W * H;
    const float inv_img_size = 1.0f / img_size;
    static histograms_t histograms;
    static statistics_t statistics;

    // Compute stats
    stats_compute_stats(&statistics, &histograms, inv_img_size);

    // AE control exposure
    uint8_t ae_done = AE_control_exposure(&statistics, c_control);
}
