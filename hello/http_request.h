#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "util.h"
#include "list.h"
#include "timer.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define TK_AGAIN EAGAIN

#define TK_HTTP_PARSE_INVALID_METHOD        10
#define TK_HTTP_PARSE_INVALID_REQUEST       11
#define TK_HTTP_PARSE_INVALID_HEADER        12

#define TK_HTTP_UNKNOWN                     0x0001
#define TK_HTTP_GET                         0x0002
#define TK_HTTP_HEAD                        0x0004
#define TK_HTTP_POST                        0x0008

#define TK_HTTP_OK                          200
#define TK_HTTP_NOT_MODIFIED                304
#define TK_HTTP_NOT_FOUND                   404
#define MAX_BUF 8124

typedef struct tk_http_request {
    char *root;
    int fd;
    int epoll_fd;
    char buff[MAX_BUF];
    size_t pos;
    size_t last;
    int state;

    void* request_start;
    void* method_end;
    int method;
    void* uri_start;
    void* uri_end;
    void* path_start;
    void* path_end;
    void* query_start;
    void* query_end;
    int http_major;
    int http_minor;
    void* request_end;

    struct list_head list;
    
    void* cur_header_key_start;
    void* cur_header_key_end;
    void* cur_header_value_start;
    void* cur_header_value_end;
    tk_timer_t* timer;
}tk_http_request_t;

typedef struct tk_http_out {
    int fd;
    int keep_alive;
    time_t mtime;
    int modified;
    int status;
}tk_http_out_t;

typedef struct tk_http_header {
    void* key_start;
    void* key_end;
    void* value_start;
    void* value_end;
    struct list_head list;
}tk_http_header_t;

typedef int (*tk_http_header_handler_pt)(tk_http_request_t* request,tk_http_out_t* out,char* data,int len)

typedef struct tk_http_header_handle {
    char* name;
    tk_http_header_handler_pt handler;
}tk_http_header_handle_t;

static int tk_http_process_ignore(tk_http_request_t* request,tk_http_out_t* out,char* data,int len) {
    (void) requet;
    (void) out;
    (void) data;
    (void) len;
    return 0;
};

static int tk_http_process_connection(tk_http_request_t* request,tk_http_out_t* out,char* data,int len) {
    (void) request;
    if(strncasecmp("keep-alive",data,len) == 0) out->keep_alive = 1;
    return 0;
};

static int tk_http_process_if_modified_since(tk_http_out_t* out,char* data,int len) {
    (void) request;
    (void) len;
    struct tm tm;
    if(strptime(data,"%a,%d %Y %H:%M:%S GMT",&tm) == (char*)NULL) {
        return 0;
    }
    time_t client_time = mktime(&tm);
    double time_diff = difftime(out->mtime,client_time);
    if(fabs(time_diff) < 1e-6) {
        out->modified = 0;
        out->status = TK_HTTP_NOT_MODIFIED;
    }
    return 0;
};

extern tk_http_header_handle_t tk_http_headers_in[] = {
    {"Host",tk_http_process_ignore},
    {"Connection",tk_http_process_connection},
    {"If-Modified-Since",tk_http_process_if_modified_since},
    {"",tk_http_process_ignore}
};

void tk_http_header_handle(tk_http_request_t *request,tk_http_out_t *out) {
    list_head* pos;
    tk_http_header_t* hd;
    tk_http_header_handle_t *header_in;
    int len;
    list_for_each(pos, &(request->list)){
        hd = list_entry(pos, tk_http_header_t, list);
        for(header_in = tk_http_headers_in; strlen(header_in->name) > 0; header_in++){
            if(strncmp(hd->key_start, header_in->name, hd->key_end - hd->key_start) == 0){
                len = hd->value_end - hd->value_start;
                (*(header_in->handler))(request, out, hd->value_start, len);
                break;
            }    
        }
        list_del(pos);
        free(hd);
    }
}

int tk_http_close_conn(tk_http_header_t *request) {
    close(request->fd);
    free(request);
    return 0;
}

int tk_init_request_t(tk_http_request_t *request,int fd,int epoll_fd,char* path) {
    request->fd =fd;
    request->epoll_fd = epoll_fd;
    request->pos = 0;
    request->last = 0;
    request->state = 0;
    request->root = path;
    INIT_LIST_HEAD(&(request->list));
    return 0;
}

int tk_init_out_t(tk_http_out_t* out,int fd) {
    out->fd = fd;
    out->keep_alive = 1;
    out->modified = 1;
    out->status = 200;
    return 0;
}

const char* get_shortmsg_from_status_code(int status_code) {
    if(status_code == TK_HTTP_OK) return "OK";
    if(status_code == TK_HTTP_NOT_MODIFIED) return "Not Modified";
    if(status_code == TK_HTTP_NOT_FOUND) return "Not Found";
    return "Unknown";
}

#endif