#include "ext.h"

static VALUE t_allocate(VALUE klass);

static void t_free(Libevent_Http *http);

static VALUE t_initialize(VALUE self, VALUE object);

static VALUE t_bind_socket(VALUE self, VALUE address, VALUE port);

static VALUE t_set_request_handler(VALUE self, VALUE handler);

static VALUE t_set_timeout(VALUE self, VALUE timeout);

static void t_request_handler(struct evhttp_request *ev_request, void *context);

static VALUE t_add_virtual_host(VALUE self, VALUE domain, VALUE vhttp);

void Init_libevent_http() {
  cLibevent_Http = rb_define_class_under(mLibevent, "Http", rb_cObject);
  
  rb_define_alloc_func(cLibevent_Http, t_allocate);

  rb_define_method(cLibevent_Http, "initialize", t_initialize, 1);
  rb_define_method(cLibevent_Http, "bind_socket", t_bind_socket, 2);
  rb_define_method(cLibevent_Http, "set_request_handler", t_set_request_handler, 1);
  rb_define_method(cLibevent_Http, "set_timeout", t_set_timeout, 1);
  rb_define_method(cLibevent_Http, "add_virtual_host", t_add_virtual_host, 2);
}

/*
 * Allocate memory
 */
static VALUE t_allocate(VALUE klass) {
  Libevent_Http *http = ALLOC(Libevent_Http);

  http->ev_base = NULL;
  http->ev_http = NULL;
  http->ev_http_parent = NULL;

  return Data_Wrap_Struct(klass, 0, t_free, http); 
}

/*
 * Free memory
 */
static void t_free(Libevent_Http *http) {
  if ( http->ev_http ) {
    // main http frees all associated vhosts
    if ( http->ev_http_parent == NULL )
      evhttp_free(http->ev_http);
  }

  xfree(http);
}

/*
 * Initialize http instance and allocate evhttp structure
 *
 * @param [Base] object
 * @yield [self]
 */
static VALUE t_initialize(VALUE self, VALUE object) {
  Libevent_Http *http;
  Libevent_Base *base;

  Data_Get_Struct(self, Libevent_Http, http);
  Data_Get_Struct(object, Libevent_Base, base);

  http->ev_base = base->ev_base;
  http->ev_http = evhttp_new(http->ev_base);

  if (!http->ev_http) {
    rb_fatal("Couldn't create evhttp");
  }

  rb_iv_set(self, "@base", object);

  if (rb_block_given_p())
    rb_yield(self);

  return self;
}

/*
 * Binds an HTTP instance on the specified address and port.
 * Can be called multiple times to bind the same http server to multiple different ports.
 * @param [String] address IP address
 * @param [Fixnum] port port to bind
 * @return [true] on success
 * @return [false] on failure
 */
static VALUE t_bind_socket(VALUE self, VALUE address, VALUE port) {
  Libevent_Http *http;
  int status;

  Data_Get_Struct(self, Libevent_Http, http);
  Check_Type(address, T_STRING);
  Check_Type(port, T_FIXNUM);
  status = evhttp_bind_socket(http->ev_http, RSTRING_PTR(address), FIX2INT(port));

  return ( status == -1 ? Qfalse : Qtrue );
}

/*
 * Set a callback for all requests that are not caught by specific callbacks.
 * @note
 *   handler should response to :call method.
 *
 *   Libevent::HttpRequest instance will be passed to handler as first argument
 *   
 * @param [Object] handler object that response to :call
 * @return [nil]
 */
static VALUE t_set_request_handler(VALUE self, VALUE handler) {
  Libevent_Http *http;

  Data_Get_Struct(self, Libevent_Http, http);

  if ( !rb_respond_to(handler, rb_intern("call")))
    rb_raise(rb_eArgError, "handler does not response to call method");

  rb_iv_set(self, "@request_handler", handler);
  evhttp_set_gencb(http->ev_http, t_request_handler, (void *)handler);

  return Qnil;
}

/*
 * C callback function that create HttpRequest instance and call Ruby handler object with it.
 */
static void t_request_handler(struct evhttp_request *ev_request, void* context) {
  Libevent_HttpRequest *le_http_request;
  VALUE http_request;
  VALUE handler = (VALUE)context;

  http_request = rb_obj_alloc(cLibevent_HttpRequest);
  Data_Get_Struct(http_request, Libevent_HttpRequest, le_http_request);
  le_http_request->ev_request = ev_request;
  rb_obj_call_init(http_request, 0, 0);

  rb_funcall(handler, rb_intern("call"), 1, http_request);
}

/*
 * Set the timeout for an HTTP request.
 * @param [Fixnum] timeout he timeout, in seconds
 * return nil
 */
static VALUE t_set_timeout(VALUE self, VALUE timeout) {
  Libevent_Http *http;

  Data_Get_Struct(self, Libevent_Http, http);
  evhttp_set_timeout(http->ev_http, NUM2INT(timeout));

  return Qnil;
}

/*
 * Adds a virtual host to the http server.
 * It is possible to have hierarchical vhosts.
 * @param [String] domain the glob pattern against which the hostname is matched. The match is case insensitive and follows otherwise regular shell matching.
 * @param [Http] vhttp the virtual host to add the regular http server
 * @return [true false]
 * @example 
 *   A vhost with the pattern *.example.com may have other vhosts with patterns foo.example.com and bar.example.com associated with it.
 * @note
 *   A virtual host is a newly initialized evhttp object that has request handler
 *   It most not have any listing sockets associated with it.
 */
static VALUE t_add_virtual_host(VALUE self, VALUE domain, VALUE vhttp) {
  Libevent_Http *le_http;
  Libevent_Http *le_vhttp;
  int status;

  Data_Get_Struct(self, Libevent_Http, le_http);
  Data_Get_Struct(vhttp, Libevent_Http, le_vhttp);
  Check_Type(domain, T_STRING);
  le_vhttp->ev_http_parent = le_http->ev_http;
  status = evhttp_add_virtual_host(le_http->ev_http, RSTRING_PTR(domain), le_vhttp->ev_http);

  return ( status == -1 ? Qfalse : Qtrue );
}

