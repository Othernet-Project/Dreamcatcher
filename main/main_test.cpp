#include <driver/sdmmc_defs.h>
#include <driver/sdspi_host.h>
#include <driver/sdmmc_types.h>
#include <driver/sdspi_host.h>
#include <esp_spiffs.h>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "diskio.h"
#include "sd_diskio.h"
#include <fcntl.h>
#include <sys/unistd.h>
#include <dirent.h>
#include "SPIFFS.h"

#define SD_MISO  (gpio_num_t)10
#define SD_MOSI  (gpio_num_t)39
#define SD_SCK   (gpio_num_t)38
#define SD_CS    (gpio_num_t)40

static char VFS_MOUNT[] = "/files";
uint32_t _blocks;
uint8_t pdrv;
struct dirent *entry;
struct stat entry_stat;
static size_t spiffstotal = 0, spiffsused = 0;

static const char *TAG = "sd_card";

esp_err_t initSDcard()
{
    pinMode(SD_CS, OUTPUT); //VSPI SS
    gpio_set_pull_mode(SD_MISO, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
    gpio_set_pull_mode(SD_MOSI, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes
    gpio_set_pull_mode(SD_CS, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes

    sdmmc_card_t *sd_card;
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.flags = SDMMC_HOST_FLAG_SPI | SDMMC_HOST_FLAG_DEINIT_ARG | SDMMC_HOST_FLAG_1BIT;
    //host.max_freq_khz = 10000;
    sdspi_device_config_t sd_slot = SDSPI_DEVICE_CONFIG_DEFAULT(); // TODO
    sd_slot.gpio_cs = SD_CS;
    sd_slot.host_id = (spi_host_device_t)host.slot;

    esp_vfs_fat_sdmmc_mount_config_t sd_cfg = {
        .format_if_mount_failed = true,
        .max_files = 10,
        .allocation_unit_size = 4 * 1024};

    // on ESP32-S2, DMA channel must be the same as host id
    #define SPI_DMA_CHAN host.slot

    spi_bus_config_t bus_cfg = {};
    bus_cfg.mosi_io_num = SD_MOSI,
    bus_cfg.miso_io_num = SD_MISO,
    bus_cfg.sclk_io_num = SD_SCK,
    bus_cfg.quadwp_io_num = -1,
    bus_cfg.quadhd_io_num = -1,
    bus_cfg.max_transfer_sz = 4 * 1024;

    esp_err_t err = spi_bus_initialize((spi_host_device_t)host.slot, &bus_cfg, SPI_DMA_CH_AUTO);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return err;
    }

    err = esp_vfs_fat_sdspi_mount(VFS_MOUNT, &host, &sd_slot, &sd_cfg, &sd_card);

    if (err == ESP_OK)
    {
        uint32_t capacity = ((uint64_t)sd_card->csd.capacity) * sd_card->csd.sector_size;
        _blocks = (uint32_t)sd_card->csd.capacity;
        log_i("[FS] SD card '%s' mounted, max capacity %.2f MB\n", sd_card->cid.name, (float)(capacity / (1024 * 1024)));

        FATFS *fs;
        DWORD c;
        if (f_getfree("0:", &c, &fs) == FR_OK)
        {
            uint32_t used_space =
                ((uint64_t)fs->csize * (fs->n_fatent - 2 - fs->free_clst)) * fs->ssize;
            float max_space = ((uint64_t)fs->csize * (fs->n_fatent - 2)) * fs->ssize;
            Serial.printf("[FS] SD FAT usage: %.2f/%.2f MB\n",
                          (float)(used_space / (1024 * 1024)), (float)(max_space / (1024 * 1024)));

            log_d("SS => %d\n", fs->ssize);
            log_d("CS => %lld\n", ((uint64_t)fs->csize * (fs->n_fatent - 2)));
        }
        log_i("[FS] SD will be used for persistent storage.");
        mkdir("/files/tmp", 0755);
        pdrv = fs->pdrv;

        // testing open command
        int openRt = open( "/files/ilikeplanes", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR );
        write(openRt, "This is a test file", 19);
        close(openRt);
        //creat( "/files/ilikebuts", 0755);
        //fopen( "/files/vodka/ilikebigbuts", "w+" );
        
        Serial.print("Open cmd test: ");
        Serial.println(openRt);
    }

    return err;
}


void setup() {
  Serial.begin(115200);
  Serial.println("Testing ESP32 SD card with vfs and POSIX");

  Serial.print("ESP-IDF major");
  Serial.println(ESP_IDF_VERSION_MAJOR);
  Serial.print("ESP-IDF minor");
  Serial.println(ESP_IDF_VERSION_MINOR);
  Serial.print("ESP-IDF patch");
  Serial.println(ESP_IDF_VERSION_PATCH);

  Serial.println("Testing..");
  initSDcard();
  Serial.println("done");
}

void loop() {
  // put your main code here, to run repeatedly:
  
}