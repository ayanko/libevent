#include "ext.h"

static VALUE t_allocate(VALUE klass);

static void t_free(Libevent_Signal *signal);

static VALUE t_initialize(VALUE self, VALUE base, VALUE name, VALUE handler);

static VALUE t_destroy(VALUE self);

static void t_handler(evutil_socket_t signal_number, short events, void *context);

void Init_libevent_signal() {
  cLibevent_Signal = rb_define_class_under(mLibevent, "Signal", rb_cObject);
  
  rb_define_alloc_func(cLibevent_Signal, t_allocate);

  rb_define_method(cLibevent_Signal, "initialize", t_initialize, 3);
  rb_define_method(cLibevent_Signal, "destroy", t_destroy, 0);
}

/*
 * Allocate memory
 */
static VALUE t_allocate(VALUE klass) {
  Libevent_Signal *signal;

  signal = ALLOC(Libevent_Signal);
  return Data_Wrap_Struct(klass, 0, t_free, signal); 
}

/*
 * Free memory
 */
static void t_free(Libevent_Signal *signal) {
  if ( signal->ev_event ) {
    event_free(signal->ev_event);
  }

  xfree(signal);
}

/*
 * Create and add signal to specified event base with handler block
 *
 * @note method allocates memory for <b>struct event </b>
 *   that will be freed when object will be freed by ruby' GC
 *
 * @param [Base] base event base instance
 * @param [String] name a name of signal
 * @param [Object] handler object that perform signal handling. Any object that responds to :call method
 *
 */
static VALUE t_initialize(VALUE self, VALUE base, VALUE name, VALUE handler) {
  Libevent_Signal *le_signal;
  Libevent_Base *le_base;
  VALUE signal_list;
  VALUE signal_number;

  Data_Get_Struct(self, Libevent_Signal, le_signal);
  Data_Get_Struct(base, Libevent_Base, le_base);

  // check name
  signal_list = rb_funcall( rb_const_get(rb_cObject, rb_intern("Signal")), rb_intern("list"), 0);
  signal_number = rb_hash_aref(signal_list, name);
  if ( signal_number == Qnil )
    rb_raise(rb_eArgError, "unknown signal name given");
  rb_iv_set(self, "@name", name);

  // check handler
  if ( !rb_respond_to(handler, rb_intern("call")))
    rb_raise(rb_eArgError, "handler does not response to call method");
  rb_iv_set(self, "@handler", handler);

  // create signal event
  le_signal->ev_event = evsignal_new(le_base->ev_base, FIX2INT(signal_number), t_handler, (void *)handler);
  if ( !le_signal->ev_event )
    rb_fatal("Could not create a signal event");
  if ( event_add(le_signal->ev_event, NULL) < 0 )
    rb_fatal("Could not add a signal event");

  return self;
}

/*
 * Delete signal from event base
 * @return [true] on success
 * @return [false] on failure
 */
static VALUE t_destroy(VALUE self) {
  Libevent_Signal *le_signal;
  int status;

  Data_Get_Struct(self, Libevent_Signal, le_signal);
  status = event_del(le_signal->ev_event);

  return( status == -1 ? Qfalse : Qtrue);
}

/*
 * C callback function that invokes call method on  Ruby object.
 */
static void t_handler(evutil_socket_t signal_number, short events, void *context) {
  VALUE handler = (VALUE)context;

  rb_funcall(handler, rb_intern("call"), 0);
}
