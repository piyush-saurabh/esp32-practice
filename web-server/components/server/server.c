#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <esp_http_server.h>

#define TAG "SERVER"

// Handler Function
static esp_err_t on_url_hit(httpd_req_t *req)
{
    ESP_LOGI(TAG, "url %s was hit", req->uri);
    char *message = "hello world!";
    httpd_resp_send(req, message, strlen(message));
    return ESP_OK;
}

// This function is called from main()
void RegisterEndPoints(void)
{
    // Create an endpoint for HTTP request
    // 1. Create a handler for the server
    // 2. Create the config
    
    // Handler for the server
    httpd_handle_t server = NULL;

    // Config for the server. The macro will start the server with default configuration
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the server
    ESP_LOGI(TAG, "starting server");
    if (httpd_start(&server, &config) != ESP_OK)
    {
        ESP_LOGE(TAG, "COULD NOT START SERVER");
    }

    // Register the endpoint
    httpd_uri_t first_end_point_config = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = on_url_hit // handler callback function once this URL is hit
    };

    // Register the handler with the server
    httpd_register_uri_handler(server, &first_end_point_config);








}