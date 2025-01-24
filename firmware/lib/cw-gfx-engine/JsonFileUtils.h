// #pragma once

#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "nvs_flash.h"

struct JsonFileUtils
{

  void initNVS()
  {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
      ESP_ERROR_CHECK(nvs_flash_erase());
      err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
  }

  static void openNVS()
  {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
    {
      Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
      return;
    }
  }

  // nvs_handle_t getHandle()
  // {
  //   nvs_handle_t nvs_handle;
  //   esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
  //   if (err != ESP_OK)
  //   {
  //     Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  //     return NULL;
  //   }
  //   return nvs_handle;
  // }

  static void saveJson(const char *filename, const char *jsonData)
  {
    openNVS();
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
    {
      Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
      return;
    }

    size_t json_size = strlen(jsonData) + 1; // 包含字符串终止符

    err = nvs_set_blob(nvs_handle, filename, jsonData, json_size);
    if (err != ESP_OK)
    {
      Serial.printf("Error (%s) writing JSON data to NVS!\n", esp_err_to_name(err));
    }
    else
    {
      Serial.println("JSON data saved successfully!");
    }
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK)
    {
      Serial.printf("Error (%s) committing NVS changes!\n", esp_err_to_name(err));
    }
    nvs_close(nvs_handle);
  }

  static String readJson(const char *filename)
  {
    openNVS();
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
    {
      Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
      return "";
    }

    char *read_json_data = nullptr;
    size_t read_json_size = 0;

    err = nvs_get_blob(nvs_handle, filename, nullptr, &read_json_size);
    if (err != ESP_OK)
    {
      Serial.printf("Error (%s) getting JSON data size!\n", esp_err_to_name(err));
      return "";
    }

    if (read_json_size > 0)
    {
      read_json_data = new char[read_json_size];
      err = nvs_get_blob(nvs_handle, filename, read_json_data, &read_json_size);
      if (err != ESP_OK)
      {
        Serial.printf("Error (%s) reading JSON data!\n", esp_err_to_name(err));
      }
      else
      {
        Serial.println("JSON data retrieved successfully!");
        Serial.println(read_json_data);
      }
      String json_data = read_json_data;
      // 关闭NVS句柄
      //nvs_close(my_handle);
      return json_data;
      // delete[] read_json_data.;
    }
    return "";
  }

  static void erase_blob_from_nvs(const char *filename)
  {
    nvs_handle_t my_handle;
    esp_err_t err;

    // 打开NVS命名空间
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
      Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
      return;
    }

    // 删除指定键值（Blob数据）
    err = nvs_erase_key(my_handle, filename); // 假设键名为"json_data"
    if (err == ESP_OK)
    {
      Serial.println("Blob data erased successfully!");
    }
    else
    {
      Serial.printf("Error (%s) erasing key!\n", esp_err_to_name(err));
    }

    // 提交删除操作
    err = nvs_commit(my_handle);
    if (err != ESP_OK)
    {
      Serial.printf("Error (%s) committing NVS changes!\n", esp_err_to_name(err));
    }

    // 关闭NVS句柄
    nvs_close(my_handle);
  }

  static void saveJsonToFile(const char *fileName, const char *json)
  {
    File file = SPIFFS.open(fileName, "w");
    if (!file)
    {
      Serial.println("文件写入失败");
      return;
    }
    file.print(json);
    file.close();
    Serial.println("JSON数据已保存到文件");
  }

  static String readJsonFromFile(const char *filename)
  {
    File file = SPIFFS.open(filename, "r");
    if (!file)
    {
      Serial.println("文件读取失败");
      return "";
    }

    String json = file.readString();
    file.close();
    return json;
  }
};
