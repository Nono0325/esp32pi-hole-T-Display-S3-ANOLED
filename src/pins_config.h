#pragma once

/*********************** Display Config *************************/
#define LCD_USB_QSPI_DREVER   1

#define SPI_FREQUENCY         75000000
#define TFT_SPI_MODE          SPI_MODE0
#define TFT_SPI_HOST          SPI2_HOST

#define LCD_WIDTH             536
#define LCD_HEIGHT            240

#define TFT_WIDTH             240
#define TFT_HEIGHT            536
#define SEND_BUF_SIZE         (0x4000)

/*********************** QSPI Display Pins **********************/
#define TFT_QSPI_CS           6
#define TFT_QSPI_SCK          47
#define TFT_QSPI_D0           18
#define TFT_QSPI_D1           7
#define TFT_QSPI_D2           48
#define TFT_QSPI_D3           5
#define TFT_QSPI_RST          17

// Legacy SPI pin defines (for compatibility)
#define TFT_CS                TFT_QSPI_CS
#define TFT_RES               TFT_QSPI_RST
#define TFT_SCK               TFT_QSPI_SCK
#define TFT_MOSI              TFT_QSPI_D0
#define TFT_DC                7

/*********************** Board Peripherals **********************/
// GPIO 38: AMOLED Power enable on Touch edition / LED on Non-Touch edition
#define PIN_POWER_ON          38
#define PIN_LED               38

// Hardware Buttons
#define PIN_BUTTON_1          0    // BOOT Key (Short: Switch Page / Long: Update)
#define PIN_BUTTON_2          21   // User Key (Short: Toggle Pause / Long: Brightness)

// Touch Controller (CST816S/FT3168 - if equipped)
#define TOUCH_I2C_SDA         3
#define TOUCH_I2C_SCL         2
#define TOUCH_INT             21
#define TOUCH_RST             -1

// Battery Voltage ADC
#define PIN_BAT_VOLT          4
