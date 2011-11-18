#include "ext.h"

static VALUE t_allocate(VALUE klass);

static void t_free(Libevent_Base *base);

static VALUE t_dispatch(VALUE self);

static VALUE t_exit_loop(VALUE self);

static VALUE t_break_loop(VALUE self);

void Init_libevent_base() {
  cLibevent_Base = rb_define_class_under(mLibevent, "Base", rb_cObject);
  
  rb_define_alloc_func(cLibevent_Base, t_allocate);

  rb_define_method(cLibevent_Base, "dispatch", t_dispatch, 0);
  rb_define_method(cLibevent_Base, "exit_loop", t_exit_loop, 0);
  rb_define_method(cLibevent_Base, "break_loop", t_break_loop, 0);
}

/*
 * Allocate memmory
 */
static VALUE t_allocate(VALUE klass) {
  Libevent_Base *base;

  base = ALLOC(Libevent_Base);
  base->ev_base = event_base_new();

  if ( !base->ev_base ) {
    rb_fatal("Couldn't get an event base");
  }

  return Data_Wrap_Struct(klass, 0, t_free, base); 
}

/*
 * Free memmory
 */
static void t_free(Libevent_Base *base) {
  event_base_free(base->ev_base);
}

/*
 * Event dispatching loop.
 *
 * This loop will run the event base until either there are no more added events, 
 * or until something calls Libevent::Base#break_loop or Base#exit_loop.
 * @see #break_loop
 * @see #exit_loop
*/
static VALUE t_dispatch(VALUE self) {
  Libevent_Base *base;
  int status;

  Data_Get_Struct(self, Libevent_Base, base);
  status = event_base_dispatch(base->ev_base);

  return INT2FIX(status);
}

/*
 * Exit the event loop after the specified time
 *
 * @todo specified time is not implemented
 */
static VALUE t_exit_loop(VALUE self) {
  Libevent_Base *base;
  int status;

  Data_Get_Struct(self, Libevent_Base, base);
  status = event_base_loopexit(base->ev_base, NULL);

  return (status == -1 ? Qfalse : Qtrue);
}
/*
 * Abort the active event_base loop immediately.
 *
 * It will abort the loop after the next event is completed;
 * event_base_loopbreak() is typically invoked from this event's callback. 
 * This behavior is analogous to the "break;" statement.
 * Subsequent invocations of event_loop() will proceed normally.
 */
static VALUE t_break_loop(VALUE self) {
  Libevent_Base *base;
  int status;

  Data_Get_Struct(self, Libevent_Base, base);
  status = event_base_loopbreak(base->ev_base);

  return (status == -1 ? Qfalse : Qtrue);
}
