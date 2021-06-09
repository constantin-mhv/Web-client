#include <string>
#include <vector>
#include "single_include/nlohmann/json.hpp"

#define HOST "ec2-3-8-116-10.eu-west-2.compute.amazonaws.com"
#define PORT 8080
#define SERVER_IP "3.8.116.10"
#define LOGIN_URL "/api/v1/tema/auth/login"
#define TERMINATOR "\r\n"
#define CONTENT_TYPE_JSON "application/json"
#define REGISTER_URL "/api/v1/tema/auth/register"
#define ACCESS_URL "/api/v1/tema/library/access"
#define BOOKS_OPERATIONS_URL "/api/v1/tema/library/books"
#define LOGOUT_URL "/api/v1/tema/auth/logout"
#define NO_TOKEN_JWT "Authorization header is missing!"
#define STATUS_CREATED  201
#define STATUS_OK 200
#define STATUS_BAD_REQUEST 400
#define STATUS_UNAUTHORIZED 401
#define STATUS_FORBIDDEN 403
#define LINE_SEPARATOR "------------------------------------"

using namespace std;

extern int sockfd;
extern vector<string> saved_cookies;
extern bool logged_in;
extern nlohmann::json token_JWT;

struct response {
    int status;
    std::string status_message;
    std::vector<std::string> cookies;
    nlohmann::json json_field;
};

/* ==== requests.cpp ==== */
/** The function processes the response received from the server.
 * Returns the response structure with completed fields.
 * */
response handle_response(string data);
/** Creates a GET/DELETE request and returns it.
 * Action must be "GET" or "DELETE" */
string compute_get_or_delete_requestE(string action, string host,
    string url, string query_params, vector<string> *headers,
        vector<string> *cookies);
/* Creates a POST request */
string compute_post_requestE(string host, string url, vector<string> *headers,
    string content_type, string body_data, vector<string> *cookies);


/* ==== speak with server ==== */
int handle_command(string command);
int register_command();
int login();
int enter_library();
int get_books();
int add_book();
int get_book();
int logout();
int delete_book();
/* trimite mesaj la server, daca serverul nu
 raspunde, redeschide conexiunea. In caz de eroare
 la deschiderea conexiunii se va inchide programul,
 altefel se incearca din nou sa se trimite mesaj.
 In caz daca serverul iarasi nu raspunde, se va returna NULL.
 Altfel, returneaza raspunsul primit de la server. */

/** Send message to server. If server does not answer, reopen the connection.
 * In case of error during opening the connection, the program will close.
 * Otherwise, an attempt is made to send a message again.
 * If the server does not respond again, NULL will be returned.
 * Otherwise, it returns the response received from the server.
 * */
char *try_send_to_server(string message);
/* Displays error message using response fields */
void wrong_status(response r);

/* ==== other ==== */
/* Returns false if string was not int or if string was empty */
bool isInt(string s);
/* Delete token_JWT and saved_cookies */
void clear_inf();
void print_full_json(nlohmann::json j);
