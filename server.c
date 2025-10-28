// server.c - Combined C backend + static frontend server
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601   // Windows 7 and above
#endif

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  typedef SOCKET socket_t;
  #define CLOSESOCKET closesocket
  #pragma comment(lib, "ws2_32")
#else
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  typedef int socket_t;
  #define INVALID_SOCKET (-1)
  #define SOCKET_ERROR   (-1)
  #define CLOSESOCKET close
#endif

#define PORT 8080
#define BUFFER_SIZE 16384

void send_all(socket_t s, const char *buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
#ifdef _WIN32
        int n = send(s, buf + sent, (int)(len - sent), 0);
#else
        ssize_t n = send(s, buf + sent, len - sent, 0);
#endif
        if (n <= 0) break;
        sent += n;
    }
}

void send_headers(socket_t client, const char *status, const char *ctype, size_t length) {
    char header[1024];
    snprintf(header, sizeof(header),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Content-Length: %zu\r\n"
        "\r\n",
        status, ctype, length
    );
    send_all(client, header, strlen(header));
}

void send_json(socket_t client, const char *status, const char *json) {
    send_headers(client, status, "application/json", strlen(json));
    send_all(client, json, strlen(json));
}

void send_file(socket_t client, const char *path, const char *ctype) {
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        send_json(client, "404 Not Found", "{\"error\":\"File not found\"}");
        return;
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    send_headers(client, "200 OK", ctype, (size_t)size);

    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
        send_all(client, buf, n);

    fclose(fp);
}

/* helper to extract small fields from JSON */
void extract_field(const char *src, const char *key, char *out, size_t outlen) {
    out[0] = '\0';
    const char *p = strstr(src, key);
    if (!p) return;
    p = strchr(p, ':');
    if (!p) return;
    p = strchr(p, '"');
    if (!p) return;
    p++;
    size_t i = 0;
    while (*p && *p != '"' && i + 1 < outlen)
        out[i++] = *p++;
    out[i] = '\0';
}

void handle_register(const char *req, socket_t client) {
    char name[100], email[100], pass[100];
    extract_field(req, "\"name\"", name, sizeof(name));
    extract_field(req, "\"email\"", email, sizeof(email));
    extract_field(req, "\"password\"", pass, sizeof(pass));

    if (strlen(email) == 0 || strlen(pass) < 4) {
        send_json(client, "400 Bad Request", "{\"success\":false,\"message\":\"Invalid registration data\"}");
        return;
    }

    FILE *fp = fopen("users.txt", "a");
    if (fp) { fprintf(fp, "%s,%s,%s\n", name, email, pass); fclose(fp); }

    send_json(client, "200 OK", "{\"success\":true,\"message\":\"Registered successfully\"}");
}

void handle_login(const char *req, socket_t client) {
    char email[100], pass[100];
    extract_field(req, "\"email\"", email, sizeof(email));
    extract_field(req, "\"password\"", pass, sizeof(pass));

    FILE *fp = fopen("users.txt", "r");
    char line[256];
    int ok = 0;
    if (fp) {
        while (fgets(line, sizeof(line), fp)) {
            char n[100], e[100], p[100];
            if (sscanf(line, "%99[^,],%99[^,],%99[^\n]", n, e, p) == 3) {
                if (strcmp(e, email) == 0 && strcmp(p, pass) == 0) {
                    ok = 1; break;
                }
            }
        }
        fclose(fp);
    }

    if (ok) send_json(client, "200 OK", "{\"success\":true,\"message\":\"Login successful!\"}");
    else send_json(client, "401 Unauthorized", "{\"success\":false,\"message\":\"Invalid email or password\"}");
}

void handle_client(socket_t client) {
    char buffer[BUFFER_SIZE+1] = {0};
#ifdef _WIN32
    int len = recv(client, buffer, BUFFER_SIZE, 0);
#else
    ssize_t len = recv(client, buffer, BUFFER_SIZE, 0);
#endif
    if (len <= 0) { CLOSESOCKET(client); return; }
    buffer[len] = '\0';

    if (strstr(buffer, "OPTIONS")) {
        send_json(client, "204 No Content", "{}");
    }
    else if (strstr(buffer, "POST /register")) {
        handle_register(buffer, client);
    }
    else if (strstr(buffer, "POST /login")) {
        handle_login(buffer, client);
    }
    else if (strstr(buffer, "GET /style.css")) {
        send_file(client, "style.css", "text/css");
    }
    else if (strstr(buffer, "GET /") && !strstr(buffer, "favicon")) {
        send_file(client, "index.html", "text/html; charset=utf-8");
    }
    else {
        send_json(client, "404 Not Found", "{\"success\":false,\"message\":\"Endpoint not found\"}");
    }

    CLOSESOCKET(client);
}

int main() {
#ifdef _WIN32
    WSADATA wsa; WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    socket_t server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) { perror("socket"); return 1; }

    int opt = 1;
#ifdef _WIN32
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); return 1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen"); return 1;
    }

    printf("✅ Server running at http://localhost:%d\n", PORT);

    while (1) {
        struct sockaddr_in caddr;
        socklen_t clen = sizeof(caddr);
        socket_t client = accept(server_fd, (struct sockaddr*)&caddr, &clen);
        if (client == INVALID_SOCKET) continue;
        handle_client(client);
    }

    CLOSESOCKET(server_fd);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
