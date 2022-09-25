#define LOFARO_DEFINE_ACH 1

typedef enum {
        HZ_NULL,
	HZ_STATE_50_IMU_MOTOR_FT,
	HZ_STATE_50_IMU,
	HZ_STATE_125_IMU,
	HZ_STATE_100_IMU_FT_SLOW,
	HZ_STATE_100_IMU_MOTORS_SLOW,
	HZ_REF_SLOW_TOP,
        DARWIN_HZ_MODE_COUNT
}__attribute__((packed)) darwin_hz_mode_index_t;
