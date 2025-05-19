#include "stm32f4xx_hal.h"
#include "wizchip_conf.h"
#include "stdio.h"

extern SPI_HandleTypeDef hspi2;

uint8_t SPIReadWrite(uint8_t data)
{
	//spi üzerinden bir byte veri gönderiliyor ve gelen cevap bekleniyor.
	//gönderdiğimiz data baytı spi sayesinde(full-dublex) aynı anda cevap döner
//	wait till FIFO has a free slot
	while ((hspi2.Instance ->SR & SPI_FLAG_TXE )!= SPI_FLAG_TXE); //txe: spi gönderme tamponu boşsa veri yazılır

	*(__IO uint8_t*)&hspi2.Instance->DR=data; //veriyi data register'a(dr) yazar,spi ile gönderir

	//now wait till data arrives
	while((hspi2.Instance->SR &SPI_FLAG_RXNE)!=SPI_FLAG_RXNE);//rxne: cevap geldi mi diye bekler

	return (*(__IO uint8_t*)&hspi2.Instance->DR);//gelen cevabı döner
}

void wizchip_select(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);//MAKES THE PIN LOW yapınca cihaz seçilir
}

void wizchip_deselect(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET); //MAKES THE PİN HIGH yapılınca iletişim kesilir
}//W5500 ile konuşmadan önce select(),iş bittiğinde deselect() yapılr

uint8_t wizchip_read()
{
	uint8_t rb;
	rb=SPIReadWrite(0x00);//dummy byte(spi'da Biz sadece veri almak isteriz,Ama SPI bir şey göndermeden alamaz.)
	return rb; //yani burda dummy veri gönder,cevap al
}

void wizchip_write(uint8_t wb)
{
	SPIReadWrite(wb);//1 bayt veri gönder
}

void wizchip_readburst(uint8_t* pBuf, uint16_t len)
{
	for(uint16_t i=0;i<len;i++)
	{
		*pBuf=SPIReadWrite(0x00);
		pBuf++;
	}
}

void wizchip_writeburst(uint8_t* pBuf,uint16_t len)
{
	for(uint16_t i=0;i<len;i++)
	{
		SPIReadWrite(*pBuf);
		pBuf++;
	}
}//w5500den çoklu veri okuma/yazma işlemlerinde kullanılır

void W5500IOInit()
{
	//initilize the two GPIO pins
	//reset -> pa
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_GPIOA_CLK_ENABLE(); //GPIOA portunun saatini aç

	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1; //PA0(reset),PA1(CS) RESET pini: W5500'e donanımsal reset atmak için.
	//CS pini: SPI haberleşmesinde aktif olan cihazı seçmek için.
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}


void w5500_enter_critical(void)//SPI işlemi sırasında başka bir kesme işleminin araya girip karışıklık yaratmasını engeller.
{
	__disable_irq();// Tüm kesmeleri geçici olarak kapat
}

void w5500_exit_critical(void)
{
	__enable_irq();// Kesmeleri tekrar aç
}

void W5500Init()
{
	uint8_t tmp;
	uint8_t memsize[2][8] = { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 2, 2, 2, 2, 2, 2, 2, 2 } };

	W5500IOInit();// GPIO'ları başlat

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);//CS high by default,başta pasif

	//Send a pulse on reset pin
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET); // Reset pini LOW
	tmp = 0xFF;
	while(tmp--);// Kısa bekleme
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET); // Reset pini HIGH

	//Callback fonksiyonları W5500 kütüphanesine tanıtılır:
	reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
	reg_wizchip_spi_cbfunc(wizchip_read, wizchip_write);
	reg_wizchip_spiburst_cbfunc(wizchip_readburst, wizchip_writeburst);
	reg_wizchip_cris_cbfunc(w5500_enter_critical, w5500_exit_critical);
	//W5500 kütüphanesi donanım bağımsızdır. Senin yazdığın SPI, GPIO, kritik bölge fonksiyonlarıyla çalışır.
	//Bu yüzden fonksiyonları bu şekilde "tanıtırsın".

	/* WIZChip Initialize*/
	if (ctlwizchip(CW_INIT_WIZCHIP, (void*) memsize) == -1) {
		printf("WIZCHIP Initialized Failed.\r\n");
		while (1);
	}
	//Her soket (0-7) için 2 KB alana sahip olacak şekilde memsize dizisi verilir.
	//Bu yapı, RX/TX buffer belleği için kullanılır.ctlwizchip(...): W5500'ün temel konfigürasyonunu yapar.
	printf("WIZCHIP Initialization Success.\r\n");

	//Check if SPI is working
	printf("Auto-detecting Ethernet Chip on SPI Port...");
	if(WIZCHIP_READ(VERSIONR)==0x04)
	{
		printf("W5500 Found on SPI\r\n");
	}
	else
	{
		printf("W5500 NOT Found on SPI\r\n");
		printf("Double Check SPI Cables\r\n");
		while(1);
	}//W5500 entegresinin VERSIONR (Version Register) değeri her zaman 0x04'tür.
	//Eğer bu değer dönmüyorsa, SPI bağlantısı veya donanımda hata var demektir.
}

















