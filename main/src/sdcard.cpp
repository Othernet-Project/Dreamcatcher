
#include <driver/sdmmc_defs.h>
#include <driver/sdspi_host.h>
#include <driver/sdmmc_types.h>
#include <driver/sdspi_host.h>
#include "driver/sdmmc_host.h"
#include <esp_spiffs.h>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "diskio.h"
#include "sd_diskio.h"
#include <fcntl.h>
#include "settings.h"
#include <sys/unistd.h>
#include <dirent.h>
#include "SPIFFS.h"

static char VFS_MOUNT[] = "/files";
uint8_t pdrv;
uint32_t _blocks;
struct dirent *entry;
struct stat entry_stat;
static size_t spiffstotal = 0, spiffsused = 0;

static const char *TAG = "sd_card";

// saves a String to a Log File
bool logToFile(char *logText){
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    int openRt = open( "/files/log/log.txt", O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR );

    char *newLogEntry = (char*) heap_caps_malloc(512, MALLOC_CAP_SPIRAM);
    sprintf(newLogEntry,"[%02d.%02d.%d - %02d:%02d:%02d] %s", timeinfo.tm_mday, timeinfo.tm_mon+1, timeinfo.tm_year + 1900, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, logText);

    write(openRt, newLogEntry, strlen(newLogEntry));
    write(openRt, "\r\n", strlen("\r\n"));
    close(openRt);
    return true;
}

esp_err_t initSDcard()
{
    pinMode(SD_CS, OUTPUT); //VSPI SS
    gpio_set_pull_mode(SD_MISO, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
    gpio_set_pull_mode(SD_MOSI, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes
    gpio_set_pull_mode(SD_CS, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes

    sdmmc_card_t *sd_card;
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.flags = SDMMC_HOST_FLAG_SPI | SDMMC_HOST_FLAG_DEINIT_ARG | SDMMC_HOST_FLAG_1BIT;
    host.max_freq_khz = SDMMC_FREQ_DEFAULT;
    sdspi_device_config_t sd_slot = SDSPI_DEVICE_CONFIG_DEFAULT(); // TODO
    sd_slot.gpio_cs = SD_CS;
    sd_slot.host_id = (spi_host_device_t)host.slot;

    esp_vfs_fat_sdmmc_mount_config_t sd_cfg = {
        .format_if_mount_failed = true,
        .max_files = 16,
        .allocation_unit_size = 16 * 1024};

    // on ESP32-S2, DMA channel must be the same as host id
    #define SPI_DMA_CHAN host.slot

    spi_bus_config_t bus_cfg = {};
    bus_cfg.mosi_io_num = SD_MOSI,
    bus_cfg.miso_io_num = SD_MISO,
    bus_cfg.sclk_io_num = SD_SCK,
    bus_cfg.quadwp_io_num = -1,
    bus_cfg.quadhd_io_num = -1;
    //bus_cfg.max_transfer_sz = 4 * 1024;

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
        //add log folder to SD card if it doesen't exist already
        mkdir("/files/log", 0755);
        pdrv = fs->pdrv;
    }

    //sdmmc_card_print_info(stdout, sd_card);
    Serial.print("SD card err var: ");
    Serial.println(err);

    return err;
}

void cleanup()
{
    char entrypath[265] = "/files/";
    DIR *dir = opendir(VFS_MOUNT);

    if (!dir) {
        return;
    }
    entry = readdir(dir);
    
    while (entry != NULL) {

        if(strncmp("tmp/", entry->d_name, 4) == 0)
        {
            strcpy(entrypath + 7, entry->d_name);
            unlink(entrypath);
        }

        entry = readdir(dir);
    }
    closedir(dir);
}

esp_err_t initSPIFFS()
{
    // if(!SPIFFS.begin(true, VFS_MOUNT)){
    //     Serial.println("SPIFFS Mount Failed");
    //     return ESP_FAIL;
    // }
    // return ESP_OK;


    Serial.println("Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = VFS_MOUNT,
      .partition_label = "spiffs",
      .max_files = 5,
      .format_if_mount_failed = true
    };

    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            Serial.println("Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            Serial.println("Failed to find SPIFFS partition");
        } else {
            Serial.printf("Failed to initialize SPIFFS (%s) \n", esp_err_to_name(ret));
        }
        return ret;
    }

    ret = esp_spiffs_info(conf.partition_label, &spiffstotal, &spiffsused);
    if (ret != ESP_OK) {
        Serial.printf("Failed to get SPIFFS partition information (%s) \n", esp_err_to_name(ret));
    } else {
        Serial.printf("Partition size: total: %d, used: %d \n", spiffstotal, spiffsused);
    }

    cleanup();

    return ret;
}

extern "C" esp_err_t formatSD()
{
    FRESULT res = FR_OK;
    esp_err_t err = ESP_OK;
    const size_t workbuf_size = 4096;
    void *workbuf = NULL;
    Serial.println("partitioning card");

    workbuf = ff_memalloc(workbuf_size);
    if (workbuf == NULL)
    {
        return ESP_ERR_NO_MEM;
    }

    DWORD plist[] = {100, 0, 0, 0};
    res = f_fdisk(pdrv, plist, workbuf);
    if (res != FR_OK)
    {
        err = ESP_FAIL;
        ESP_LOGE(TAG, "f_fdisk failed (%d)", res);
    }
    else
    {
        size_t alloc_unit_size = 512;
        char drv[3] = {'0', ':', 0};
        ESP_LOGW(TAG, "formatting card, allocation unit size=%d", alloc_unit_size);
        res = f_mkfs(drv, FM_ANY, alloc_unit_size, workbuf, workbuf_size);
        if (res != FR_OK)
        {
            err = ESP_FAIL;
            ESP_LOGE(TAG, "f_mkfs failed (%d)", res);
        }
    }
    mkdir("/files/tmp", 0755);  // add tmp folder
    mkdir("/files/log", 0755);  // add log folder

    Serial.println("partitioning card finished");
    free(workbuf);
    return err;
}

extern "C" void getFreeStorageSPIFFS(uint64_t* total, uint64_t* used)
{
    *total = spiffstotal;
    *used = spiffsused;
}
