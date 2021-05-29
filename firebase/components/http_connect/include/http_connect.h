#ifndef _HTTP_CONNECT_H_
#define _HTTP_CONNECT_H_

// HTTP Headers
typedef struct
{
    char *key;
    char *val;
} Header;

// HTTP Request type
typedef enum
{
    GET,
    POST
} HttpMethod;


struct HttpConnectParams
{
    void ( *http_response_parser)(char *incomingBuffer, char * output); // function pointer
    char parsedResponse[2048]; // data returned by the http response after parsing TODO: make this buffer dynamic
    Header header[3]; // Array size will be the number of headers required in the request
    HttpMethod method; // HTTP Request Method
    char *body; // pointer to the request body (POST)
    int status; // HTTP response status
    int headerCount; // count of headers sent in the request
    const uint8_t *serverCert; // name of the root server cert which is required for verification
};

void send_http_request(char *url, struct HttpConnectParams *httpConnectParams);

#endif