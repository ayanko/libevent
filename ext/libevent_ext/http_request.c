#include "ext.h"

static VALUE t_allocate(VALUE klass);

static void t_free(Libevent_HttpRequest *http_request);

static VALUE t_initialize(VALUE self);

static VALUE t_get_remote_host(VALUE self);

static VALUE t_get_remote_port(VALUE self);

static VALUE t_get_http_version(VALUE self);

static VALUE t_get_command(VALUE self);

static VALUE t_get_uri(VALUE self);

static VALUE t_get_uri_scheme(VALUE self);

static VALUE t_get_uri_path(VALUE self);

static VALUE t_get_uri_query(VALUE self);

static VALUE t_get_host(VALUE self);

static VALUE t_get_input_headers(VALUE self);

static VALUE t_get_body(VALUE self);

static VALUE t_send_reply(VALUE self, VALUE code, VALUE headers, VALUE body);

static VALUE t_send_error(VALUE self, VALUE code, VALUE reason);

static VALUE t_add_output_header(VALUE self, VALUE key, VALUE value);

static VALUE t_set_output_headers(VALUE self, VALUE headers);

static VALUE t_clear_output_headers(VALUE self);

static VALUE t_send_chunk(VALUE chunk, VALUE self);

static VALUE t_send_reply_start(VALUE self, VALUE code, VALUE reason);

static VALUE t_send_reply_chunk(VALUE self, VALUE chunk);

static VALUE t_send_reply_end(VALUE self);

void Init_libevent_http_request() {
  cLibevent_HttpRequest = rb_define_class_under(mLibevent, "HttpRequest", rb_cObject);

  rb_define_alloc_func(cLibevent_HttpRequest, t_allocate);
  
  rb_define_method(cLibevent_HttpRequest, "initialize", t_initialize, 0);
  rb_define_method(cLibevent_HttpRequest, "get_remote_host", t_get_remote_host, 0);
  rb_define_method(cLibevent_HttpRequest, "get_remote_port", t_get_remote_port, 0);
  rb_define_method(cLibevent_HttpRequest, "get_http_version", t_get_http_version, 0);
  rb_define_method(cLibevent_HttpRequest, "get_command", t_get_command, 0);
  rb_define_method(cLibevent_HttpRequest, "get_uri", t_get_uri, 0);
  rb_define_method(cLibevent_HttpRequest, "get_uri_scheme", t_get_uri_scheme, 0);
  rb_define_method(cLibevent_HttpRequest, "get_uri_path", t_get_uri_path, 0);
  rb_define_method(cLibevent_HttpRequest, "get_uri_query", t_get_uri_query, 0);
  rb_define_method(cLibevent_HttpRequest, "get_host", t_get_host, 0);
  rb_define_method(cLibevent_HttpRequest, "get_input_headers", t_get_input_headers, 0);
  rb_define_method(cLibevent_HttpRequest, "get_body", t_get_body, 0);
  rb_define_method(cLibevent_HttpRequest, "add_output_header", t_add_output_header, 2);
  rb_define_method(cLibevent_HttpRequest, "set_output_headers", t_set_output_headers, 1);
  rb_define_method(cLibevent_HttpRequest, "clear_output_headers", t_clear_output_headers, 0);
  rb_define_method(cLibevent_HttpRequest, "send_reply", t_send_reply, 3);
  rb_define_method(cLibevent_HttpRequest, "send_error", t_send_error, 2);
  rb_define_method(cLibevent_HttpRequest, "send_reply_start", t_send_reply_start, 2);
  rb_define_method(cLibevent_HttpRequest, "send_reply_chunk", t_send_reply_chunk, 1);
  rb_define_method(cLibevent_HttpRequest, "send_reply_end", t_send_reply_end, 0);
}

/*
 * Allocate memory
 */
static VALUE t_allocate(VALUE klass) {
  Libevent_HttpRequest *http_request = ALLOC(Libevent_HttpRequest);

  http_request->ev_request = NULL;
  http_request->ev_buffer = evbuffer_new();

  return Data_Wrap_Struct(klass, 0, t_free, http_request); 
}

/*
 * Free memory
 */
static void t_free(Libevent_HttpRequest *http_request) {
  if ( http_request->ev_buffer != NULL ) {
    evbuffer_free(http_request->ev_buffer);
  }

  xfree(http_request);
}

/*
 * Initialize HttpRequest object
 * @raise [ArgumentError] if object created withot evhttp_request c data
 */
static VALUE t_initialize(VALUE self) {
  Libevent_HttpRequest *http_request;

  Data_Get_Struct(self, Libevent_HttpRequest, http_request);
  if ( !http_request->ev_request )
    rb_raise(rb_eArgError, "http_request C data is not given");

  return self;
}

/*
 * Add output header
 * @param [String] key a header key
 * @param [String] value a header value
 * @return [true false]
 */
static VALUE t_add_output_header(VALUE self, VALUE key, VALUE value) {
  Libevent_HttpRequest *http_request;
  struct evkeyvalq *ev_headers;
  int status;

  Data_Get_Struct(self, Libevent_HttpRequest, http_request);
  ev_headers = evhttp_request_get_output_headers(http_request->ev_request);
  status = evhttp_add_header(ev_headers, RSTRING_PTR(key), RSTRING_PTR(value));

  return ( status == -1 ? Qfalse : Qtrue);
}

/*
 * Set request output headers
 * @param [Hash Array] headers 
 * @return [nil]
 */
static VALUE t_set_output_headers(VALUE self, VALUE headers) { 
  Libevent_HttpRequest *http_request;
  VALUE pairs;
  VALUE pair;
  VALUE key;
  VALUE val;
  struct evkeyvalq *ev_headers;
  int i;

  Data_Get_Struct(self, Libevent_HttpRequest, http_request);

  ev_headers = evhttp_request_get_output_headers(http_request->ev_request);

  pairs = rb_funcall(headers, rb_intern("to_a"), 0);

  for ( i=0 ; i < RARRAY_LEN(pairs); i++ ) {
    pair = rb_ary_entry(pairs, i);
    key = rb_ary_entry(pair, 0);
    val = rb_ary_entry(pair, 1);
    evhttp_add_header(ev_headers, RSTRING_PTR(key), RSTRING_PTR(val));
  }

  return Qnil;
}

/*
 * Removes all output headers.
 * @return [nil]
 */
static VALUE t_clear_output_headers(VALUE self) { 
  Libevent_HttpRequest *http_request;
  struct evkeyvalq *ev_headers;

  Data_Get_Struct(self, Libevent_HttpRequest, http_request);
  ev_headers = evhttp_request_get_output_headers(http_request->ev_request);
  evhttp_clear_headers(ev_headers);

  return Qnil;
}

/*
 * Get request input headers
 * @return [Hash] 
 */
static VALUE t_get_input_headers(VALUE self) {
  Libevent_HttpRequest *http_request;
  struct evkeyvalq *ev_headers;
  struct evkeyval *ev_header;
  VALUE headers;

  Data_Get_Struct(self, Libevent_HttpRequest, http_request);

  headers = rb_hash_new();

  ev_headers = evhttp_request_get_input_headers(http_request->ev_request);

  for ( ev_header = ev_headers->tqh_first; ev_header; ev_header = ev_header->next.tqe_next ) {
    rb_hash_aset(headers, rb_str_new2(ev_header->key), rb_str_new2(ev_header->value));
  }

  return headers;
}

/*
 * Get the remote address of associated connection
 * @return [String] IP address
 */
static VALUE t_get_remote_host(VALUE self) {
  Libevent_HttpRequest *http_request;

  Data_Get_Struct(self, Libevent_HttpRequest, http_request);

  return rb_str_new2(http_request->ev_request->remote_host);
}

/*
 * Get the remote port of associated connection
 * @return [Fixnum] port
 */
static VALUE t_get_remote_port(VALUE self) {
  Libevent_HttpRequest *http_request;

  Data_Get_Struct(self, Libevent_HttpRequest, http_request);

  return INT2FIX(http_request->ev_request->remote_port);
}

/*
 * Get request HTTP version
 * @return [String] http version
 */
static VALUE t_get_http_version(VALUE self) {
  Libevent_HttpRequest *http_request;
  char http_version[3];

  Data_Get_Struct(self, Libevent_HttpRequest, http_request);
  sprintf(http_version, "HTTP/%d.%d", http_request->ev_request->major, http_request->ev_request->minor);

  return rb_str_new2(http_version);
}

/*
 * Get request command (i.e method)
 *
 * @return [String] http method for known command
 * @return [nil] if method is not supported
 * @example
 *   GET
 */
static VALUE t_get_command(VALUE self) {
  Libevent_HttpRequest *http_request;
  VALUE command;

  Data_Get_Struct(self, Libevent_HttpRequest, http_request);

  switch ( evhttp_request_get_command(http_request->ev_request) ) {
    case EVHTTP_REQ_GET     : command = rb_str_new2("GET");     break;
    case EVHTTP_REQ_POST    : command = rb_str_new2("POST");    break;
    case EVHTTP_REQ_HEAD    : command = rb_str_new2("HEAD");    break;
    case EVHTTP_REQ_PUT     : command = rb_str_new2("PUT");     break;
    case EVHTTP_REQ_DELETE  : command = rb_str_new2("DELETE");  break;
    case EVHTTP_REQ_OPTIONS : command = rb_str_new2("OPTIONS"); break;
    case EVHTTP_REQ_TRACE   : command = rb_str_new2("TRACE");   break;
    case EVHTTP_REQ_CONNECT : command = rb_str_new2("CONNECT"); break;
    case EVHTTP_REQ_PATCH   : command = rb_str_new2("PATCH");   break;
    default: command = Qnil; break;
  }

  return command;
}

/*
 * Returns the host associated with the request. If a client sends an absolute
 * URI, the host part of that is preferred. Otherwise, the input headers are
 * searched for a Host: header. NULL is returned if no absolute URI or Host:
 * header is provided.
 * @return [String] host
*/
static VALUE t_get_host(VALUE self) {
  Libevent_HttpRequest *http_request;

  Data_Get_Struct(self, Libevent_HttpRequest, http_request);

  return rb_str_new2(evhttp_request_get_host(http_request->ev_request));
}

/*
 * Get request URI
 * @return [String] uri string
 */
static VALUE t_get_uri(VALUE self) {
  Libevent_HttpRequest *http_request;

  Data_Get_Struct(self, Libevent_HttpRequest, http_request);

  return rb_str_new2(evhttp_request_get_uri(http_request->ev_request));
}

/*
 * Read body from request input buffer.
 * @return [String] body
 */
static VALUE t_get_body(VALUE self) {
  Libevent_HttpRequest *http_request;
  struct evbuffer *ev_buffer;
  int length;
  VALUE body;

  Data_Get_Struct(self, Libevent_HttpRequest, http_request);

  ev_buffer = evhttp_request_get_input_buffer(http_request->ev_request);

  length = evbuffer_get_length(ev_buffer);
  body = rb_str_new(0, length);
  evbuffer_copyout(ev_buffer, RSTRING_PTR(body), length);

  return body;
}

/*
 * Send error to client
 *
 * @param [Fixnum] code HTTP code
 * @param [String] reason (optional) short error descriptor
 * @return [nil]
 */
static VALUE t_send_error(VALUE self, VALUE code, VALUE reason) {
  Libevent_HttpRequest *http_request;

  Data_Get_Struct(self, Libevent_HttpRequest, http_request);

  evhttp_send_error(http_request->ev_request, FIX2INT(code), reason == Qnil ? NULL : RSTRING_PTR(reason));

  return Qnil;
}

/*
 * Send reply to client
 * @param [Fixnum] code HTTP code
 * @param [Hash] headers hash of http output headers
 * @param [Object] body object that response to each method that returns strings
 * @return [nil]
 */
static VALUE t_send_reply(VALUE self, VALUE code, VALUE headers, VALUE body) {
  Libevent_HttpRequest *http_request;

  Data_Get_Struct(self, Libevent_HttpRequest, http_request);
  Check_Type(code, T_FIXNUM);
  Check_Type(headers, T_HASH);

  t_set_output_headers(self, headers);

  evhttp_send_reply_start(http_request->ev_request, FIX2INT(code), NULL);
  rb_iterate(rb_each, body, t_send_chunk, self);
  evhttp_send_reply_end(http_request->ev_request);

  return Qnil;
}

/*
 * #send_reply iteration method to send chunk of data to client
 * @param [String] chunk 
 * @param [Object] self HttpRequest instance
 * @return [nil]
 */
static VALUE t_send_chunk(VALUE chunk, VALUE self) {
  Libevent_HttpRequest *http_request;

  Data_Get_Struct(self, Libevent_HttpRequest, http_request);

  evbuffer_add(http_request->ev_buffer, RSTRING_PTR(chunk), RSTRING_LEN(chunk));
  evhttp_send_reply_chunk(http_request->ev_request, http_request->ev_buffer);

  return Qnil;
}

/*
 * Start reply to client
 * @param [Fixnum] code HTTP code
 * @param [String] reason (optional) response descriptor
 * @return [nil]
 */
static VALUE t_send_reply_start(VALUE self, VALUE code, VALUE reason) {
  Libevent_HttpRequest *http_request;

  Data_Get_Struct(self, Libevent_HttpRequest, http_request);
  Check_Type(code, T_FIXNUM);

  evhttp_send_reply_start(http_request->ev_request, FIX2INT(code), reason == Qnil ? NULL : RSTRING_PTR(reason));

  return Qnil;
}

/*
 * Send chunk of data to client
 * @param [String] chunk string
 * @return [nil]
 */
static VALUE t_send_reply_chunk(VALUE self, VALUE chunk) {
  Libevent_HttpRequest *http_request;

  Data_Get_Struct(self, Libevent_HttpRequest, http_request);

  evbuffer_add(http_request->ev_buffer, RSTRING_PTR(chunk), RSTRING_LEN(chunk));
  evhttp_send_reply_chunk(http_request->ev_request, http_request->ev_buffer);

  return Qnil;
}

/*
 * Stop reply to client
 * @return [nil]
 */
static VALUE t_send_reply_end(VALUE self) {
  Libevent_HttpRequest *http_request;

  Data_Get_Struct(self, Libevent_HttpRequest, http_request);
  evhttp_send_reply_end(http_request->ev_request);

  return Qnil;
}

/*
 * Get request URI scheme
 * @return [String] http or https
 * @return [nil]
 */
static VALUE t_get_uri_scheme(VALUE self) {
  Libevent_HttpRequest *http_request;
  const struct evhttp_uri *ev_uri;
  const char* scheme;

  Data_Get_Struct(self, Libevent_HttpRequest, http_request);

  ev_uri = evhttp_request_get_evhttp_uri(http_request->ev_request);
  scheme = evhttp_uri_get_scheme(ev_uri);

  return(scheme ? rb_str_new2(scheme) : Qnil);
}

/*
 * Get request URI path
 * @return [String]
 * @return [nil]
 */
static VALUE t_get_uri_path(VALUE self) {
  Libevent_HttpRequest *http_request;
  const char *path;

  Data_Get_Struct(self, Libevent_HttpRequest, http_request);

  path = evhttp_uri_get_path(http_request->ev_request->uri_elems);

  return(path ? rb_str_new2(path) : Qnil);
}

/*
 * Get request URI query
 * @return [String]
 * @return [nil]
 */
static VALUE t_get_uri_query(VALUE self) {
  Libevent_HttpRequest *http_request;
  const char *query;

  Data_Get_Struct(self, Libevent_HttpRequest, http_request);

  query = evhttp_uri_get_query(http_request->ev_request->uri_elems);

  return(query ? rb_str_new2(query) : Qnil);
}
