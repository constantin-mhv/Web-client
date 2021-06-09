#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "helpers.h"

#include <string>
#include <iostream>
#include <cstring>
#include <sstream>
#include "headers.h"
#include "single_include/nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

vector<string> saved_cookies;
int sockfd;
bool logged_in = false;
json token_JWT;

string valid_input(string command);

int main(int argc, char *argv[]) {
    string message;
    string serv_resp;
    string input, to_skip;
    sockfd = open_connection(SERVER_IP, PORT, PF_INET, SOCK_STREAM, 0);
    if (sockfd == 0)
        error("open_connection error");

    while (getline(cin, input)) {
        input = valid_input(input);
        if (input == "exit")
            break;
        /* valid_input returns " " if in string input were only whitespaces */
        else if (input == " ")
            continue;
        else
            handle_command(input);
    }
    
    close(sockfd);
    return 1;
}

/** Validate input.
 * If the input has more than one word, it returns an empty string.
 * If string contains only whitespaces, returns "".
 * Otherwise, it returns the word without whitespaces. 
 * */
string valid_input(string command) {
    istringstream iss(command);
    string result, temp;
    if (!(iss >> result))
        return " ";
    if (iss >> temp) {
        result.clear();
    }
    iss.clear();
    return result; 
}
