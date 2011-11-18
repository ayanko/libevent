#ifndef LIBEVENT_EXT_H
#define LIBEVENT_EXT_H

#include "ruby.h"
#include "ruby18_compat.h"

#include <event.h>
#include <evhttp.h>

VALUE mLibevent;
VALUE cLibevent_Base;
VALUE cLibevent_Signal;
VALUE cLibevent_Http;
VALUE cLibevent_HttpRequest;

typedef struct Libevent_Base {
  struct event_base *ev_base;
} Libevent_Base;

typedef struct Libevent_Signal {
  struct event *ev_event;
} Libevent_Signal;

typedef struct Libevent_Http {
  struct event_base *ev_base;
  struct evhttp *ev_http;
  struct evhttp *ev_http_parent;
} Libevent_Http;

typedef struct Libevent_HttpRequest {
  struct evhttp_request *ev_request;
  struct evbuffer *ev_buffer;
} Libevent_HttpRequest;

void Init_libevent_base();
void Init_libevent_signal();
void Init_libevent_http();
void Init_libevent_http_request();

#endif
