#ifndef _FETCH_H_
#define _FETCH_H_

struct FetchParams
{
    void ( *parseResponse)(char *incomingBuffer, char * output); // function pointer
    char message[300]; // data returned by the http response (after parsing)
};

void fetch(char *url, struct FetchParams *fetchParams);

#endif