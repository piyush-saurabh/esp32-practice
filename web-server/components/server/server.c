#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <esp_http_server.h>
#include <cJSON.h>
#include <driver/gpio.h>
#include <esp_spiffs.h> //for accessing spiffs (html pages)

#define TAG "SERVER"

// onboard LED PIN Number
#define LED 2

// Initialize the GPIO
void InitializeLed(){
    gpio_pad_select_gpio(LED);
    gpio_set_direction(LED,GPIO_MODE_OUTPUT);
}

// 1st Handler Function (GET handler)
static esp_err_t on_url_hit(httpd_req_t *req)
{
    ESP_LOGI(TAG, "url %s was hit", req->uri);

    // Access the spiffs to serve the page
    // Create the spiffs config (check the partition.csv to find the spiffs name)
    esp_vfs_spiffs_conf_t config = {
        .base_path = "/web",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false};

    // Initialize SPIFFS to use the configuration
    esp_vfs_spiffs_register(&config);


    char path[600];
    if (strcmp(req->uri, "/") == 0)
    {
        strcpy(path, "/web/index.html");
    }
    else
    {
        sprintf(path, "/web%s", req->uri);
    }

    // Open the file and read it
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        // If file doesn't exist, return 404 Not Found status
        httpd_resp_send_404(req);
    }
    else
    {
        char lineRead[256];
        while (fgets(lineRead, sizeof(lineRead), file))
        {
            httpd_resp_sendstr_chunk(req,lineRead);
        }

        // Terminate the connection
        httpd_resp_sendstr_chunk(req, NULL);
    }

    // Deinitialize spiffs
    esp_vfs_spiffs_unregister(NULL);

    // char *message = "hello world!";
    // httpd_resp_send(req, message, strlen(message));
    return ESP_OK;
}

// 2nd Handler Function (GET handler)
static esp_err_t on_get_temperature(httpd_req_t *req)
{
    ESP_LOGI(TAG, "url %s was hit", req->uri);
    char *message = "{\"temperature\": \"25\"}";

    // Setup the MIME type
    httpd_resp_set_type(req, "application/json");

    // Send the response
    httpd_resp_send(req, message, strlen(message));
    return ESP_OK;
}

// 3rd Handler Function (POST handler)
static esp_err_t on_led_set(httpd_req_t *req)
{
    ESP_LOGI(TAG, "url %s was hit", req->uri);
    
    // Create a buffer to hold the infomation posted in the request
    char buf[150];
    memset(buf, 0, sizeof(buf)); //initialize the buffer with zero

    // Get the information received in the request and store it in buffer
    // TODO validate content_len
    httpd_req_recv(req, buf, req->content_len);

    // Parse the request coming from the request
    /*
        Sample Request Body
        {"isLedOn": false}
    */
    cJSON *payload = cJSON_Parse(buf);
    cJSON *isLedOn = cJSON_GetObjectItem(payload, "isLedOn");

    // Toggle the GPIO Pin
    gpio_set_level(LED,cJSON_IsTrue(isLedOn));

    // Delete the JSON object to clean up the memory
    cJSON_Delete(payload);

    // Send back the response (acknowledgement)
    httpd_resp_set_status(req, "204 NO CONTENT");

    // Send the null response
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

// This function is called from main()
void RegisterEndPoints(void)
{
    // Initialize the GPIO (for controlling onboard LED)
    InitializeLed();

    // Create an endpoint for HTTP request
    // 1. Create a handler for the server
    // 2. Create the config
    
    // Handler for the server
    httpd_handle_t server = NULL;

    // Config for the server. The macro will start the server with default configuration
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Create wildcard route. This will point to a callback (already implemented by ESP IDF)
    config.uri_match_fn = httpd_uri_match_wildcard;

    // Start the server
    ESP_LOGI(TAG, "starting server");
    if (httpd_start(&server, &config) != ESP_OK)
    {
        ESP_LOGE(TAG, "COULD NOT START SERVER");
    }

    // Routing works in sequence

    // Register 2nd endpoint
    httpd_uri_t second_end_point_config = {
        .uri = "/api/temperature",
        .method = HTTP_GET,
        .handler = on_get_temperature // handler callback function once this URL is hit
    };
    // Register the handler with the server
    httpd_register_uri_handler(server, &second_end_point_config);

    // Register 3rd endpoint
    httpd_uri_t third_end_point_config = {
        .uri = "/api/led",
        .method = HTTP_POST,
        .handler = on_led_set // handler callback function once this URL is hit
    };
    // Register the handler with the server
    httpd_register_uri_handler(server, &third_end_point_config);

    // Register 1st endpoint
    // wild card route should be at the end
    httpd_uri_t first_end_point_config = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = on_url_hit // handler callback function once this URL is hit
    };
    // Register the handler with the server
    httpd_register_uri_handler(server, &first_end_point_config);

}