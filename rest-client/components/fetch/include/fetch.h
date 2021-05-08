#ifndef _FETCH_H_
#define _FETCH_H_

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

/*
struct FetchParms
{
    void (*OnGotData)(char *incomingBuffer, char *output);
    char message[300];
    Header header[3];
    int headerCount;
    HttpMethod method;
    char *body;
    int status;
};
*/

struct FetchParams
{
    void ( *parseResponse)(char *incomingBuffer, char * output); // function pointer
    char message[300]; // data returned by the http response (after parsing) TODO: make this buffer dynamic
    Header header[3]; // Array size will be the number of headers required in the request
    HttpMethod method; // HTTP Request Method
    char *body; // pointer to the request body (POST)
    int status; // HTTP response status
    int headerCount; // count of headers sent in the request
};

void fetch(char *url, struct FetchParams *fetchParams);

#endif