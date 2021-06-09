#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include "headers.h"

using namespace std;


/** Creates a GET / DELETE request and returns it.
 * Action must be "GET" or "DELETE" */
string compute_get_or_delete_requestE(string action, string host, string url,
        string query_params, vector<string> *headers,
            vector<string> *cookies) {
    string message;
    string line;

    line = action + " " + url;
    if (query_params.length() != 0) {
         line += '?' + query_params + " HTTP/1.1"; 
    } else {
        line += " HTTP/1.1";
    }
    message = line + TERMINATOR;

    message += "Host: " + host + TERMINATOR;

    /* add additional headers */
    if(headers != NULL) {
        for(size_t i = 0; i < (*headers).size(); i++) {
            message += (*headers)[i] + TERMINATOR;
        }
    }
    if (cookies != NULL && (*cookies).size() > 0) {
        message += "Cookie: ";
        for(size_t i = 0; i < (*cookies).size(); ++i) {
            message += (*cookies)[i];
            if(i != (*cookies).size() - 1)
                message += "; ";
        }
        message += TERMINATOR;
    }
    message += TERMINATOR;
        return message;
}



string compute_post_requestE(string host, string url,
                    vector<string> *headers, string content_type,
    string body_data, vector<string> *cookies) {
    string message;
    string line;

    message = "POST " + url + " HTTP/1.1" + TERMINATOR;
    message += "Host: " + host + TERMINATOR;

    /* add additional headers */
    if(headers != NULL) {
        for(size_t i = 0; i < (*headers).size(); i++) {
            message += (*headers)[i] + TERMINATOR;
        }
    }

    if (cookies != NULL && (*cookies).size() > 0) {
        message += "Cookie: ";
        for(size_t i = 0; i < (*cookies).size(); ++i) {
            message += (*cookies)[i];
            if(i != (*cookies).size() - 1)
                message += "; ";
        }
        message += TERMINATOR;  
    }

    message += "Content-Type: " + content_type + TERMINATOR;
    message += "Content-Length: " + to_string(body_data.length()) + TERMINATOR;

    message += TERMINATOR;
    message += body_data;
    message += TERMINATOR;

    return message;
}