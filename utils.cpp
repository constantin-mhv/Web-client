#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "helpers.h"

#include <string>
#include <iostream>
#include <sstream>
#include "headers.h"
#include "single_include/nlohmann/json.hpp"


using namespace std;
using json = nlohmann::json;

/** The function processes the response received from the server.
 * Returns the response structure with completed fields.
 * */
response handle_response(string data) {
    response result;

    /** on the first line is "HTTP/1.1 XXX status_message",
     * status_message will start from position 13 */
    result.status_message = data.substr(13, data.find("\r\n") - 13);
    /** in the first string at the second position
     * will bethe status of the answer */
    string temp;
    istringstream iss(data);
    iss >> temp >> temp;
    result.status = stoi(temp);
    iss.clear();

    /* find line with cookies */
    string cookies_found;
    size_t found_begin = data.find("Set-Cookie:");
    if (found_begin != string::npos) {
        cookies_found = data.substr(found_begin);
        size_t found_end = cookies_found.find("\r\n");
        cookies_found = cookies_found.substr(0, found_end);     

        /* parse cookies */
        iss.str(cookies_found);
        getline(iss, temp, ' ');
        while (getline (iss, temp, ' ')) {
            istringstream iss(temp);
            while (getline(iss, temp, ';'))
                result.cookies.push_back(temp);
        } 
    }

    /* find json */
    string terminators = "\r\n\r\n";
    found_begin = data.find(terminators);
    if (found_begin != string::npos) {
        if (data.length() > found_begin + terminators.length()) {
            if (data.at(found_begin + terminators.length()) == '['
                || data.at(found_begin + terminators.length()) == '{')
                result.json_field = 
                json::parse(data.substr(found_begin + terminators.length()));
            else
                /** If get the message "too many requests ...",
                 * put the received message in the "error" field at json */
                result.json_field =
                {{"error", data.substr(found_begin + terminators.length())}};
        }
    }

    return result;
}

int handle_command(string command) {
    if (command == "register") {
        return register_command();
    } else if (command == "login") {
        return login();
    } else if (command == "enter_library") {
        enter_library();
    } else if (command == "get_books") {
        get_books();
    } else if (command == "add_book") {
        add_book();
    } else if (command == "logout") {
        logout();
    } else if (command == "get_book") {
        get_book();
    } else if (command == "delete_book") {
        delete_book();
    }
    else {
        cout << "Command \"" + command + "\" not found." << endl;
    }
    return 1;
}

int register_command() {
    string username, passoword;
    json json_field;
    cout << "username=";
    getline(cin, username);
    cout << "password=";
    getline(cin, passoword);

    json_field =  {
        {"username", username},
        {"password", passoword}
    };
    string message =
    compute_post_requestE(HOST, REGISTER_URL,
        NULL, CONTENT_TYPE_JSON, json_field.dump(), NULL);
    
    char *data = try_send_to_server(message);
    /* if data == NULL, server is not responding */
    if (!data)
        return -1;
    response serv_resp = handle_response(data);
    /* If something is wrong, response ststus will be != STATUS_CREATED */
    if (serv_resp.status != STATUS_CREATED) {
        wrong_status(serv_resp);
        return -1;
    }
    cout << "You have successfully registered!" << endl;
    return 1;
}

int login() {
    if (logged_in) {
        cout << "You are already logged in!" << endl;
        return 1;
    }
    string username, passoword;
    json json_field;
    cout << "username=";
    getline(cin, username);
    cout << "password=";
    getline(cin, passoword);

    json_field =  {
        {"username", username},
        {"password", passoword}
    };

    string message = compute_post_requestE(HOST,
        LOGIN_URL, NULL, CONTENT_TYPE_JSON, json_field.dump(), NULL);
    
    char *data = try_send_to_server(message);
    /* if data == NULL, server is not responding */
    if (!data)
        return -1;
    
    response serv_resp = handle_response(data);
    /* if something is wrong, statusul response status will be != STATUS_OK */
    if (serv_resp.status != STATUS_OK) {
        wrong_status(serv_resp);
        return -1;
    }
    cout << "You have successfully logged in!" << endl;
    saved_cookies = serv_resp.cookies;
    logged_in = true;
    return 1;
}

int enter_library() {
    string message = compute_get_or_delete_requestE("GET",
    HOST, ACCESS_URL, "", NULL, &saved_cookies);
    char *data = try_send_to_server(message);
    if (!data)
        return -1;
    response serv_resp = handle_response(data);
    if (serv_resp.status != STATUS_OK) {
        wrong_status(serv_resp);
        return -1;
    }
    cout << "You have successfully entered the library!" << endl;
    token_JWT = serv_resp.json_field;
    return 1;
}

int get_books() {
    vector<string>headers;
    if (!token_JWT.empty()) {
        headers.push_back("Authorization: Bearer " 
            + token_JWT["token"].get<string>());
    }
    string message = compute_get_or_delete_requestE("GET",
        SERVER_IP, BOOKS_OPERATIONS_URL, "", &headers, NULL);
    
    char *data = try_send_to_server(message);
    /* if data == NULL, server is not responding */
    if (!data)
        return -1;
    
    response serv_resp = handle_response(data);
    /* if something is wrong, statusul response status will be != STATUS_OK */
    if (serv_resp.status != STATUS_OK) {
        wrong_status(serv_resp);
        return -1;
    }
    cout << "Books:" << endl;
    if (serv_resp.json_field.empty())
        cout << "No books." << endl;
    else 
        print_full_json(serv_resp.json_field);
        
    return 1;
}

int add_book() {
    string title, author, genre, publisher, temp_page_count;
    int page_count;
    json json_field;
    cout << "title=";
    getline(cin, title);
    cout << "author=";
    getline(cin, author);
    cout << "genre=";
    getline(cin, genre);
    cout << "publisher=";
    getline(cin, publisher);
    cout << "page_count=";
    getline(cin, temp_page_count);
    /* if page_count was entered in the wrong format */
    if (!isInt(temp_page_count) ||
        (page_count = atoi(temp_page_count.c_str()) <= 0)) {
        cout << "Invalid format for field \"page_count\".";
        cout << " To repeat operation type \"add_book\" again." << endl;
        return -1;
    }
    page_count = atoi(temp_page_count.c_str());

    vector<string>headers;
    if (!token_JWT.empty()) {
        headers.push_back("Authorization: Bearer " +
            token_JWT["token"].get<string>());
    }
    json_field =  {
         {"title", title},
         {"author", author},
         {"genre", genre},
         {"page_count", page_count},
         {"publisher", publisher}
    };

    string message = compute_post_requestE(HOST, BOOKS_OPERATIONS_URL,
        &headers, CONTENT_TYPE_JSON, json_field.dump(), NULL);

    char *data = try_send_to_server(message);
    /* if data == NULL, server is not responding */
    if (!data)
        return -1;
    
    response serv_resp = handle_response(data);
    /* if something is wrong, statusul response status will be != STATUS_OK */
    if (serv_resp.status != STATUS_OK) {
        wrong_status(serv_resp);
        return -1;
    }
    cout << "Book \"" << title << "\" successfully added." <<endl;
    return 1;
}

int logout() {
    string message = compute_get_or_delete_requestE("GET",
    SERVER_IP, LOGOUT_URL, "", NULL, &saved_cookies);
    
    char *data = try_send_to_server(message);
    /* if data == NULL, server is not responding */
    if (!data)
        return -1;
    
    response serv_resp = handle_response(data);
    /* if something is wrong, statusul response status will be != STATUS_OK */
    if (serv_resp.status != STATUS_OK ) {
        wrong_status(serv_resp);
        clear_inf();
        logged_in = false;
        return -1;
    }
    cout << "You have successfully logged out." << endl;
    clear_inf();
    logged_in = false;
    return 1;
}

int get_book() {
    string id_str;
    int id;
    cout << "id=";
    getline(cin, id_str);
    /* if id was entered in the wrong format */
    if (!isInt(id_str) || (id = atoi(id_str.c_str()) <= 0)) {
        cout << "Invalid format for field \"id\".";
        cout << " To repeat operation type \"get_book\" again." << endl;
        return -1;
    }
    id = atoi(id_str.c_str());
    vector<string>headers;
    if (!token_JWT.empty()) {
        headers.push_back("Authorization: Bearer " +
            token_JWT["token"].get<string>());
    }
    string url = BOOKS_OPERATIONS_URL;
    url += "/" + to_string(id);;

    string message = compute_get_or_delete_requestE("GET",
        SERVER_IP, url, "", &headers, NULL);

    char *data = try_send_to_server(message);
    /* if data == NULL, server is not responding */
    if (!data)
        return -1;
    
    response serv_resp = handle_response(data);

    /* if something is wrong, statusul response status will be != STATUS_OK */
    if (serv_resp.status != STATUS_OK) {
        wrong_status(serv_resp);
        return -1;
    }

    cout << "Your book:" << endl;
    print_full_json(serv_resp.json_field);
    return 1;
}

int delete_book() {
    string id_str;
    int id;
    cout << "id=";
    getline(cin, id_str);
    /* if id was entered in the wrong format */
    if (!isInt(id_str) || (id = atoi(id_str.c_str()) <= 0)) {
        cout << "Invalid format for field \"id\". To repeat operation type \"delete_book\" again." << endl;
        return -1;
    }
    id = atoi(id_str.c_str());
    vector<string>headers;
    if (!token_JWT.empty()) {
        headers.push_back("Authorization: Bearer " + token_JWT["token"].get<string>());
    }
    string url = BOOKS_OPERATIONS_URL;
    url += "/" + to_string(id);;

    string message = compute_get_or_delete_requestE("DELETE", SERVER_IP, url, "", &headers, NULL);

    char *data = try_send_to_server(message);
    /* if data == NULL, server is not responding */
    if (!data)
        return -1;
    
    response serv_resp = handle_response(data);
    /* if something is wrong, statusul response status will be != STATUS_OK */
    if (serv_resp.status != STATUS_OK) {
        wrong_status(serv_resp);
        return -1;
    }
    cout << "You have successfully deleted the book." << endl;
    return 1;
}

/** Send message to server. If server does not answer, reopen the connection.
 * In case of error during opening the connection, the program will close.
 * Otherwise, an attempt is made to send a message again.
 * If the server does not respond again, NULL will be returned.
 * Otherwise, it returns the response received from the server.
 * */
char *try_send_to_server(string message) {
    send_to_server(sockfd, &message[0]);
    char *data = receive_from_server(sockfd);

    if (data == NULL || strlen(data) == 0) {
        sockfd = open_connection(SERVER_IP, PORT, PF_INET, SOCK_STREAM, 0);
        if (sockfd == 0)
            error("open_connection error");
    }

    send_to_server(sockfd, &message[0]);
    data = receive_from_server(sockfd);

    if (data == NULL || strlen(data) == 0) {
        cout << "Server is not responding!" << endl;
        return NULL;
    }
    json j;
    j.empty();
    return data;


}

/* Displays error message using response fields */
void wrong_status(response r) {
    cout << "ERROR: " << r.status_message << endl;
    if (!r.json_field.empty())
        cout << r.json_field["error"].get<string>() << endl;
}

void print_full_json(json j) {
    if (!j.empty()) {
        for (auto iter = j.begin(); iter != j.end(); ++iter) {
            for (auto i = (*iter).items().begin();
                    i != (*iter).items().end(); ++i) {
                cout << i.key() << " : " << i.value() <<  endl;
            }
            cout << LINE_SEPARATOR << endl;
        }
    }
}

void clear_inf() {
    saved_cookies.clear();
    token_JWT.clear();
}

bool isInt(string s) {
    if (s.length() == 0)
        return false;
    for (auto i : s) 
        if (isdigit(i) == false) 
            return false; 
    return true; 
}
