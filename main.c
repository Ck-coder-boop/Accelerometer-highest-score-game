#include "eeng1030_lib.h"
#include "display.h"
#include "i2c.h"
#include <stdint.h>
#include <stdio.h>
#include <stm32l432xx.h>

#include "sound_array.h"

#define ACCEL_ADDR 0x69

#define ACT_LEFT     0
#define ACT_RIGHT    1
#define ACT_FORWARD  2
#define ACT_BACK     3
#define ACT_NONE     255

#define X_THRESHOLD 900
#define Y_FORWARD_THRESHOLD 800
#define Y_BACK_THRESHOLD   -900

#define X_NEUTRAL_HIGH  300
#define X_NEUTRAL_LOW  -300
#define Y_NEUTRAL_HIGH  200
#define Y_NEUTRAL_LOW  -200

#define BUTTON_PORT GPIOB
#define BUTTON_PIN  4

#define RGB_R_PORT GPIOB
#define RGB_R_PIN  3
#define RGB_G_PORT GPIOA
#define RGB_G_PIN  12
#define RGB_B_PORT GPIOA
#define RGB_B_PIN  11

#define I2S_BCLK_PORT GPIOA
#define I2S_BCLK_PIN  8
#define I2S_LRC_PORT  GPIOA
#define I2S_LRC_PIN   9
#define I2S_DIN_PORT  GPIOA
#define I2S_DIN_PIN   10

#define ROUND_TIME_MS      4000
#define IDLE_SLEEP_MS      10000
#define DEBOUNCE_MS        250
#define RESULT_DISPLAY_MS  2000

extern volatile uint32_t milliseconds;

static void setup(void);
static void enter_idle_sleep(void);

static void accel_init(void);
static int16_t accel_read_axis(uint8_t reg);
static uint8_t detect_tilt(int32_t x, int32_t y);

static void rgb_off(void);
static void rgb_red(void);
static void rgb_green(void);
static void rgb_blue(void);
static void rgb_yellow(void);

static void draw_start(uint8_t high);
static void draw_action(uint8_t act, uint8_t score, uint8_t high);
static void draw_fail(uint8_t score, uint8_t high);
static void draw_high(uint8_t score);

static uint8_t next_action(void);

static void i2s_gpio_init(void);
static void i2s_send_bit(uint8_t bit);
static void i2s_send_word(int16_t sample);
static void i2s_send_stereo(int16_t left, int16_t right);
static void play_sound_file(void);

int main(void)
{
    uint8_t running = 0;
    uint8_t last_button = 1;
    uint8_t button;

    uint8_t action = ACT_NONE;
    uint8_t detected = ACT_NONE;

    uint8_t ready = 1;
    uint8_t score = 0;
    uint8_t high = 0;

    uint32_t round_start_time = 0;
    uint32_t last_activity_time = 0;

    int16_t x_raw, y_raw;
    int32_t x, y;

    setup();
    accel_init();

    rgb_blue();
    draw_start(high);

    last_activity_time = milliseconds;

    while (1)
    {
        button = (BUTTON_PORT->IDR & (1 << BUTTON_PIN)) ? 1 : 0;

        /*
         * Button is active-low because the internal pull-up is enabled.
         * A falling edge starts the game from the idle state.
         */
        if ((last_button == 1) && (button == 0) && !running)
        {
            running = 1;
            score = 0;
            ready = 1;

            action = next_action();
            round_start_time = milliseconds;
            last_activity_time = milliseconds;

            rgb_yellow();
            draw_action(action, score, high);

            delay_ms(DEBOUNCE_MS);
        }

        last_button = button;

        if (running)
        {
            /*
             * Read accelerometer X and Y axes.
             * Raw values are scaled to approximate acceleration values.
             */
            x_raw = accel_read_axis(0x12);
            y_raw = accel_read_axis(0x14);

            x = ((int32_t)x_raw * 981) / 16384;
            y = ((int32_t)y_raw * 981) / 16384;

            /*
             * The ready flag prevents one tilt from being counted repeatedly.
             * The player must return the device to a neutral position before
             * another movement is accepted.
             */
            if ((x < X_NEUTRAL_HIGH && x > X_NEUTRAL_LOW) &&
                (y < Y_NEUTRAL_HIGH && y > Y_NEUTRAL_LOW))
            {
                ready = 1;
            }

            detected = detect_tilt(x, y);

            if (ready && detected != ACT_NONE)
            {
                ready = 0;
                last_activity_time = milliseconds;

                if (detected == action)
                {
                    /*
                     * Correct input:
                     * score increases and a new random instruction is shown.
                     */
                    score++;

                    rgb_green();
                    delay_ms(200);

                    action = next_action();
                    round_start_time = milliseconds;

                    rgb_yellow();
                    draw_action(action, score, high);
                }
                else
                {
                    /*
                     * Incorrect input:
                     * game ends. If the score beats the high score,
                     * the stored audio array is played.
                     */
                    running = 0;

                    if (score > high)
                    {
                        high = score;
                        rgb_green();
                        draw_high(score);
                        play_sound_file();
                    }
                    else
                    {
                        rgb_red();
                        draw_fail(score, high);
                    }

                    delay_ms(RESULT_DISPLAY_MS);
                    rgb_blue();
                    draw_start(high);
                    last_activity_time = milliseconds;
                }
            }

            /*
             * Timeout:
             * if the player does not respond in time, the game ends.
             */
            if ((milliseconds - round_start_time) > ROUND_TIME_MS)
            {
                running = 0;

                if (score > high)
                {
                    high = score;
                    rgb_green();
                    draw_high(score);
                    play_sound_file();
                }
                else
                {
                    rgb_red();
                    draw_fail(score, high);
                }

                delay_ms(RESULT_DISPLAY_MS);
                rgb_blue();
                draw_start(high);
                last_activity_time = milliseconds;
            }
        }
        else
        {
            /*
             * Low-power idle behaviour:
             * after a period of inactivity, outputs are turned off and the CPU
             * waits for the next interrupt. A button press wakes the system.
             */
            if ((milliseconds - last_activity_time) > IDLE_SLEEP_MS)
            {
                enter_idle_sleep();
                last_activity_time = milliseconds;
                draw_start(high);
                rgb_blue();
            }
        }

        delay_ms(50);
    }
}

static void enter_idle_sleep(void)
{
    rgb_off();

    /*
     * WFI places the CPU into sleep until an interrupt occurs.
     * SysTick or external interrupt activity can wake the device.
     * This is a light sleep mode and does not erase variables such as high score.
     */
    __WFI();
}

static uint8_t next_action(void)
{
    static uint8_t count = 0;
    count++;

    /*
     * Lightweight pseudo-random action generation using time and counter.
     * This avoids repeating a fixed sequence each time the game runs.
     */
    return (uint8_t)((milliseconds + count * 7) % 4);
}

static uint8_t detect_tilt(int32_t x, int32_t y)
{
    if (y < 200 && y > -200)
    {
        if (x > X_THRESHOLD) return ACT_RIGHT;
        if (x < -X_THRESHOLD) return ACT_LEFT;
    }

    if (x < 300 && x > -300)
    {
        if (y > Y_FORWARD_THRESHOLD) return ACT_FORWARD;
        if (y < Y_BACK_THRESHOLD) return ACT_BACK;
    }

    return ACT_NONE;
}

/* ===================== Display Functions ===================== */

static void draw_start(uint8_t high)
{
    uint16_t black = RGBToWord(0,0,0);
    uint16_t cyan = RGBToWord(0,255,255);
    uint16_t white = RGBToWord(255,255,255);

    char buf[16];

    fillRectangle(0,0,160,80,black);
    printTextX2("MOTION", 30, 10, cyan, black);
    printText("PRESS BUTTON", 25, 40, white, black);

    sprintf(buf, "HIGH:%d", high);
    printText(buf, 50, 62, white, black);
}

static void draw_action(uint8_t act, uint8_t score, uint8_t high)
{
    uint16_t black = RGBToWord(0,0,0);
    uint16_t yellow = RGBToWord(255,255,0);
    uint16_t white = RGBToWord(255,255,255);

    const char* txt[] = {"LEFT", "RIGHT", "FORWARD", "BACK"};
    char buf[20];

    fillRectangle(0,0,160,80,black);
    printTextX2((char*)txt[act], 18, 28, yellow, black);

    sprintf(buf, "S:%d H:%d", score, high);
    printText(buf, 40, 65, white, black);
}

static void draw_fail(uint8_t score, uint8_t high)
{
    uint16_t black = RGBToWord(0,0,0);
    uint16_t red = RGBToWord(255,0,0);
    uint16_t white = RGBToWord(255,255,255);

    char buf[20];

    fillRectangle(0,0,160,80,black);
    printTextX2("FAIL", 40, 10, red, black);

    sprintf(buf, "S:%d H:%d", score, high);
    printText(buf, 40, 60, white, black);
}

static void draw_high(uint8_t score)
{
    uint16_t black = RGBToWord(0,0,0);
    uint16_t green = RGBToWord(0,255,0);
    uint16_t white = RGBToWord(255,255,255);

    char buf[20];

    fillRectangle(0,0,160,80,black);
    printTextX2("NEW HIGH", 8, 20, green, black);

    sprintf(buf, "SCORE:%d", score);
    printText(buf, 45, 60, white, black);
}

/* ===================== RGB LED Functions ===================== */

static void rgb_off(void)
{
    RGB_R_PORT->ODR &= ~(1 << RGB_R_PIN);
    RGB_G_PORT->ODR &= ~(1 << RGB_G_PIN);
    RGB_B_PORT->ODR &= ~(1 << RGB_B_PIN);
}

static void rgb_red(void)
{
    rgb_off();
    RGB_R_PORT->ODR |= (1 << RGB_R_PIN);
}

static void rgb_green(void)
{
    rgb_off();
    RGB_G_PORT->ODR |= (1 << RGB_G_PIN);
}

static void rgb_blue(void)
{
    rgb_off();
    RGB_B_PORT->ODR |= (1 << RGB_B_PIN);
}

static void rgb_yellow(void)
{
    rgb_off();
    RGB_R_PORT->ODR |= (1 << RGB_R_PIN);
    RGB_G_PORT->ODR |= (1 << RGB_G_PIN);
}

/* ===================== Audio Functions ===================== */

static void i2s_gpio_init(void)
{
    pinMode(I2S_BCLK_PORT, I2S_BCLK_PIN, 1);
    pinMode(I2S_LRC_PORT, I2S_LRC_PIN, 1);
    pinMode(I2S_DIN_PORT, I2S_DIN_PIN, 1);

    I2S_BCLK_PORT->ODR &= ~(1 << I2S_BCLK_PIN);
    I2S_LRC_PORT->ODR &= ~(1 << I2S_LRC_PIN);
    I2S_DIN_PORT->ODR &= ~(1 << I2S_DIN_PIN);
}

static void i2s_send_bit(uint8_t bit)
{
    if (bit)
        I2S_DIN_PORT->ODR |= (1 << I2S_DIN_PIN);
    else
        I2S_DIN_PORT->ODR &= ~(1 << I2S_DIN_PIN);

    I2S_BCLK_PORT->ODR |= (1 << I2S_BCLK_PIN);
    for (volatile int d = 0; d < 3; d++);

    I2S_BCLK_PORT->ODR &= ~(1 << I2S_BCLK_PIN);
    for (volatile int d = 0; d < 3; d++);
}

static void i2s_send_word(int16_t sample)
{
    uint16_t value = (uint16_t)sample;

    for (int bit = 15; bit >= 0; bit--)
    {
        i2s_send_bit((value >> bit) & 1);
    }
}

static void i2s_send_stereo(int16_t left, int16_t right)
{
    I2S_LRC_PORT->ODR &= ~(1 << I2S_LRC_PIN);
    i2s_send_word(left);

    I2S_LRC_PORT->ODR |= (1 << I2S_LRC_PIN);
    i2s_send_word(right);
}

static void play_sound_file(void)
{
    /*
     * The sound data is stored as signed 16-bit PCM values in sound_array.h.
     * The sample is sent to both left and right channels.
     */
    for (uint32_t i = 0; i < sound_length; i++)
    {
        int16_t s = sound_data[i] * 3;
        i2s_send_stereo(s, s);
    }

    I2S_DIN_PORT->ODR &= ~(1 << I2S_DIN_PIN);
}

/* ===================== Setup and Accelerometer ===================== */

static void setup(void)
{
    RCC->AHB2ENR |= (1 << 0) | (1 << 1);

    initClocks();
    initSysTick();
    init_display();
    initI2C();

    pinMode(BUTTON_PORT, BUTTON_PIN, 0);
    enablePullUp(BUTTON_PORT, BUTTON_PIN);

    pinMode(RGB_R_PORT, RGB_R_PIN, 1);
    pinMode(RGB_G_PORT, RGB_G_PIN, 1);
    pinMode(RGB_B_PORT, RGB_B_PIN, 1);

    i2s_gpio_init();

    rgb_off();
}

static void accel_init(void)
{
    /*
     * Write to accelerometer power/control register.
     * This places the device into the active measurement mode.
     */
    I2CStart(ACCEL_ADDR, WRITE, 2);
    I2CWrite(0x7E);
    I2CWrite(0x11);
    I2CStop();

    delay_ms(100);
}

static int16_t accel_read_axis(uint8_t reg)
{
    uint16_t low, high;

    I2CStart(ACCEL_ADDR, WRITE, 1);
    I2CWrite(reg);
    I2CReStart(ACCEL_ADDR, READ, 2);

    low = I2CRead();
    high = I2CRead();

    I2CStop();

    return (int16_t)(low | (high << 8));
}