// PMSIS (Pulp Microcontroller Software Interface Standard)
#include "pmsis.h"

// BSP (Board Support Package) For Bitcraze AI-Deck
#include "bsp/bsp.h"
#include "bsp/buffer.h"
#include "bsp/camera/himax.h"

// STD Include
#include "stdio.h"
#include "stdint.h"

// Local Include
#include "cpx.h"

// Macros
#define GAP8_FREQUENCY 250 *1000 *1000
#define GAP8_VOLTAGE 1200  // 1200
#define GAP8_UART_BAUDRATE 115200
#define LED_PIN 2
#define BUFFER_SIZE 6

// Local prototypes
static void task_main(void);
void task_rw_uart();

struct pi_cluster_conf cluster_conf;

// Bytes we want to send
static char *value;

static CPXPacket_t txp;

/*
 * send_buffer fills the cpx packet with the real data and sends the packet
 * pkt: the cpx packet wrapping the real data
 * buf: the real data we want to send
 * buf_size: data size
 */
static void send_buffer(CPXPacket_t *pkt, uint8_t *buf, uint32_t buf_size) {
    uint32_t pkt_size = 0;
    uint32_t buf_offset = 0;

    do {
        pkt_size = sizeof(pkt->data);
        if (buf_offset + pkt_size > buf_size) {
            pkt_size = buf_size - buf_offset;
        }
        memcpy(pkt->data, &buf[buf_offset], sizeof(pkt->data));
        pkt->dataLength = pkt_size;
        cpxSendPacketBlocking(pkt);
        buf_offset += pkt_size;
    } while (buf_offset < buf_size);

    return;
}

int main(void) {
    pi_bsp_init();
    pi_freq_set(PI_FREQ_DOMAIN_FC, GAP8_FREQUENCY);
    pi_pmu_voltage_set(PI_PMU_DOMAIN_FC, GAP8_VOLTAGE);
    pmsis_kickoff((void *)task_main);

    return 0;
}

struct pi_device uart_device;
struct pi_uart_conf uart_conf;

void task_main() {
    // Config UART
    pi_uart_conf_init(&uart_conf);
    uart_conf.baudrate_bps = GAP8_UART_BAUDRATE;
    pi_open_from_conf(&uart_device, &uart_conf);

    // Open UART
    if (pi_uart_open(&uart_device)) {
        printf("[UART] open failed !\n");
        pmsis_exit(-1);
    }

    // Open Led
    pi_device_t led_gpio_dev;

    // Initialize the LED pin
    pi_gpio_pin_configure(&led_gpio_dev, LED_PIN, PI_GPIO_OUTPUT);
    pi_gpio_pin_write(&led_gpio_dev, LED_PIN, 1);

    // CPX
    cpxInit();
    cpxPrintToConsole(LOG_TO_CRTP, "[UART_rw][task_main] Welcome!\n");
    cpxPrintToConsole(LOG_TO_CRTP, "[UART_rw][task_main] configMINIMAL_STACK_SIZE = %d\n", configMINIMAL_STACK_SIZE);
    cpxPrintToConsole(LOG_TO_CRTP, "[UART_rw][task_main] tskIDLE_PRIORITY = %d\n", tskIDLE_PRIORITY);

    BaseType_t xTask;

    xTask = xTaskCreate(
            task_rw_uart,
            "task_rw_uart",
            configMINIMAL_STACK_SIZE*4,
            NULL,
            tskIDLE_PRIORITY + 1,
            NULL);

    if (xTask != pdPASS) {
        cpxPrintToConsole(LOG_TO_CRTP, "[UART_rw][task_main] Error: task_rw_uart did not start!\n");
        pmsis_exit(-1);
    }

    while (1) {
        pi_yield();
    }
}

void task_rw_uart(void *param) {

    cpxPrintToConsole(LOG_TO_CRTP, "[UART_rw][task_main] task_rw_uart is running...\n");
    // Free the memory for the bytes we want to send and initialize them
    value = (char *)pmsis_l2_malloc(BUFFER_SIZE);
    value[0] = 'E'; value[1] = 'C'; value[2] = 'S'; value[3] = 'L'; value[4] = 'a'; value[5] = 'b';

    // Configure the cpx route and data length
    cpxInitRoute(CPX_T_GAP8, CPX_T_STM32, CPX_F_CRTP, &txp.route);
    txp.dataLength = sizeof(value);

    // UART Communication
    while (1) {
        vTaskDelay(1000);
        // Write chars on UART (cpx)
        send_buffer(&txp, (uint8_t*)value, BUFFER_SIZE);
    }
}
