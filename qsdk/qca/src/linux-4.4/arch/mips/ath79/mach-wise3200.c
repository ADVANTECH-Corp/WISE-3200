/*
 * Atheros AP143 & CUS531 reference board support for WISE-3200
 *
 * Copyright (c) 2015 The Linux Foundation. All rights reserved.
 * Copyright (c) 2012 Gabor Juhos <juhosg@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>
#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/ath79.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/delay.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "dev-i2c.h"
#include "machtypes.h"

#define WISE3200_GPIO_I2C_SDA		0
#define WISE3200_GPIO_PCIE_WAKE		1
#define WISE3200_GPIO_PCIE_DISABLE	2
#define WISE3200_GPIO_SPI_CS0		5
#define WISE3200_GPIO_SPI_CLK		6
#define WISE3200_GPIO_SPI_MOSI		7
#define WISE3200_GPIO_SPI_MISO		8
#define WISE3200_GPIO_UART_RX		9
#define WISE3200_GPIO_UART_TX		10
#define WISE3200_GPIO_SPI_CS1		11
#define WISE3200_GPIO_LED_WLAN		12
#define WISE3200_GPIO_LED_WAN		13
#define WISE3200_GPIO_SPI_CS2		14
#define WISE3200_GPIO_LED_LAN		15
#define WISE3200_GPIO_I2C_SCL		16
#define WISE3200_GPIO_RESET		17

#define WISE3200_MAC0_OFFSET		0
#define WISE3200_MAC1_OFFSET		6
#define WISE3200_WMAC_CALDATA_OFFSET	0x1000

#define AR934X_GPIO_IN_MUX_UART1_TX	22
#define AR934X_GPIO_OUT_SYS_RST_L	1
#define AR934X_GPIO_OUT_LED_LINK_0	41
#define AR934X_GPIO_OUT_LED_LINK_4	45

static void __init wise3200_gpio_setup(void)
{
	// Disable JTAG function
	ath79_gpio_function_enable(AR934X_GPIO_FUNC_JTAG_DISABLE);

	ath79_gpio_direction_select(WISE3200_GPIO_SPI_CS0, true);
	ath79_gpio_direction_select(WISE3200_GPIO_SPI_CLK, true);
	ath79_gpio_direction_select(WISE3200_GPIO_SPI_MOSI, true);
	ath79_gpio_direction_select(WISE3200_GPIO_SPI_MISO, false);
	ath79_gpio_direction_select(WISE3200_GPIO_UART_RX, false);
	ath79_gpio_direction_select(WISE3200_GPIO_UART_TX, true);
	ath79_gpio_direction_select(WISE3200_GPIO_SPI_CS1, true);
	ath79_gpio_direction_select(WISE3200_GPIO_LED_WLAN, true);
	ath79_gpio_direction_select(WISE3200_GPIO_LED_WAN, true);
	ath79_gpio_direction_select(WISE3200_GPIO_LED_LAN, true);
	ath79_gpio_direction_select(WISE3200_GPIO_PCIE_WAKE, false);
	ath79_gpio_direction_select(WISE3200_GPIO_SPI_CS2, true);
	ath79_gpio_direction_select(WISE3200_GPIO_PCIE_DISABLE, true);
	ath79_gpio_direction_select(WISE3200_GPIO_I2C_SCL, true);
	ath79_gpio_direction_select(WISE3200_GPIO_I2C_SDA, true);
	ath79_gpio_direction_select(WISE3200_GPIO_RESET, true);

	ath79_gpio_output_select(WISE3200_GPIO_SPI_CS0, QCA953X_GPIO_OUT_MUX_SPI_CS0);
	ath79_gpio_output_select(WISE3200_GPIO_SPI_CLK, QCA953X_GPIO_OUT_MUX_SPI_CLK);
	ath79_gpio_output_select(WISE3200_GPIO_SPI_MOSI, QCA953X_GPIO_OUT_MUX_SPI_MOSI);
	ath79_gpio_input_select(WISE3200_GPIO_SPI_MISO, AR934X_GPIO_IN_MUX_SPI_MISO);
	ath79_gpio_input_select(WISE3200_GPIO_UART_RX, AR934X_GPIO_IN_MUX_UART1_RD);
	ath79_gpio_output_select(WISE3200_GPIO_UART_TX, AR934X_GPIO_IN_MUX_UART1_TX);
	ath79_gpio_output_select(WISE3200_GPIO_SPI_CS1, QCA953X_GPIO_OUT_MUX_SPI_CS1);
	ath79_gpio_output_select(WISE3200_GPIO_LED_WLAN, AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(WISE3200_GPIO_LED_WAN, AR934X_GPIO_OUT_LED_LINK_4);
	ath79_gpio_output_select(WISE3200_GPIO_LED_LAN, AR934X_GPIO_OUT_LED_LINK_0);
	ath79_gpio_output_select(WISE3200_GPIO_SPI_CS2, QCA953X_GPIO_OUT_MUX_SPI_CS2);
	ath79_gpio_output_select(WISE3200_GPIO_PCIE_DISABLE, AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(WISE3200_GPIO_I2C_SCL, AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(WISE3200_GPIO_I2C_SDA, AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(WISE3200_GPIO_RESET, AR934X_GPIO_OUT_SYS_RST_L);

	__raw_writel(BIT(WISE3200_GPIO_SPI_CS0), ath79_gpio_base + AR71XX_GPIO_REG_SET);
	__raw_writel(BIT(WISE3200_GPIO_SPI_CS1), ath79_gpio_base + AR71XX_GPIO_REG_SET);
	__raw_writel(BIT(WISE3200_GPIO_SPI_CS2), ath79_gpio_base + AR71XX_GPIO_REG_SET);
	__raw_writel(BIT(WISE3200_GPIO_LED_WLAN), ath79_gpio_base + AR71XX_GPIO_REG_SET);
	__raw_writel(BIT(WISE3200_GPIO_LED_WAN), ath79_gpio_base + AR71XX_GPIO_REG_SET);
	__raw_writel(BIT(WISE3200_GPIO_LED_LAN), ath79_gpio_base + AR71XX_GPIO_REG_SET);
	__raw_writel(BIT(WISE3200_GPIO_PCIE_WAKE), ath79_gpio_base + AR71XX_GPIO_REG_SET);
	__raw_writel(BIT(WISE3200_GPIO_PCIE_DISABLE), ath79_gpio_base + AR71XX_GPIO_REG_SET);
}

static void __init wise3200_sys_reset(void)
{
	void __iomem *ath79_reset_base = ioremap(AR71XX_RESET_BASE, AR71XX_RESET_SIZE);
	void __iomem *reg = ath79_reset_base + AR71XX_RESET_REG_PCI_INT_ENABLE;

	/* Reset */
	mdelay(100);
	__raw_writel(AR934X_RESET_EXTERNAL, reg);
	mdelay(10);
	__raw_writel(AR934X_RESET_USBSUS_OVERRIDE, reg); //Set to default value
	mdelay(100);
}

static struct i2c_gpio_platform_data ath79_i2c_gpio_data = {
	.sda_pin = WISE3200_GPIO_I2C_SDA,
	.scl_pin = WISE3200_GPIO_I2C_SCL,
};

static const struct i2c_board_info rtc_s35390a_info = {
	I2C_BOARD_INFO("s35390a", 0x30),
};

static struct ath79_spi_controller_data wise3200_spi0_cdata =
{
	.cs_type	= ATH79_SPI_CS_TYPE_INTERNAL,
	.is_flash	= true,
	.cs_line	= 0,
};

static struct ath79_spi_controller_data wise3200_spi1_cdata =
{
	.cs_type	= ATH79_SPI_CS_TYPE_INTERNAL,
	.is_flash	= true,
	.cs_line	= 1,
};

static struct ath79_spi_controller_data wise3200_spi2_cdata =
{
	.cs_type	= ATH79_SPI_CS_TYPE_INTERNAL,
	.cs_line	= 2,
};

static struct spi_board_info wise3200_spi_info[] = {
	{
		.bus_num	= 0,
		.chip_select	= 0,
		.max_speed_hz	= 25000000,
		.modalias	= "w25q128",
		.controller_data = &wise3200_spi0_cdata,
		.platform_data 	= NULL,
	},
	{
		.bus_num	= 0,
		.chip_select	= 1,
		.max_speed_hz   = 50000000,
		.modalias	= "ath79-spinand",
		.controller_data = &wise3200_spi1_cdata,
		.platform_data 	= NULL,
	},
	{
		.bus_num	= 0,
		.chip_select	= 2,
		.max_speed_hz	= 25000000,
		.modalias	= "tpm_spi_tis",
		.controller_data = &wise3200_spi2_cdata,
		.platform_data	= NULL,
	}
};

static struct ath79_spi_platform_data wise3200_spi_data = {
	.bus_num		= 0,
	.num_chipselect		= 3,
        .word_banger            = true,
};

static void __init wise3200_register_i2c_devices(
		struct i2c_board_info const *info)
{
	ath79_gpio_output_select(WISE3200_GPIO_I2C_SDA, AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(WISE3200_GPIO_I2C_SCL, AR934X_GPIO_OUT_GPIO);

	ath79_register_i2c(&ath79_i2c_gpio_data, info, info ? 1 : 0);
}

static void __init wise3200_setup(u8 *art)
{
	wise3200_gpio_setup();
	wise3200_sys_reset();

	ath79_register_spi(&wise3200_spi_data, wise3200_spi_info, ARRAY_SIZE(wise3200_spi_info));

	ath79_register_usb();

	ath79_wmac_set_led_pin(WISE3200_GPIO_LED_WLAN);
	ath79_register_wmac(art + WISE3200_WMAC_CALDATA_OFFSET, NULL);

	ath79_register_mdio(0, 0x0);
	ath79_register_mdio(1, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, art + WISE3200_MAC0_OFFSET, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, art + WISE3200_MAC1_OFFSET, 0);

	/* WAN port */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_register_eth(0);
	/* LAN ports */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_switch_data.phy_poll_mask |= BIT(4);
	ath79_switch_data.phy4_mii_en = 1;
	ath79_register_eth(1);

	/* GPIO based S/W I2C master device */
	wise3200_register_i2c_devices(&rtc_s35390a_info);
}

static void __init ap143_setup(void)
{
	u8 *art_ap143 = (u8 *) KSEG1ADDR(0x1fff0000);

	wise3200_setup(art_ap143);
}

static void __init cus531_nand_setup(void)
{
	u8 *art_cus531 = (u8 *) KSEG1ADDR(0x1f070000);

	wise3200_setup(art_cus531);
}

MIPS_MACHINE(ATH79_MACH_AP143, "AP143", "Qualcomm Atheros AP143 reference board",
	     ap143_setup);
MIPS_MACHINE(ATH79_MACH_CUS531_NAND, "CUS531-NAND", "Qualcomm Atheros CUS531 nand reference board",
	     cus531_nand_setup);
