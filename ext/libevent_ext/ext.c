#include "ext.h"

void Init_libevent_ext() {
  mLibevent = rb_define_module("Libevent");

  Init_libevent_base();
  Init_libevent_signal();
  Init_libevent_http();
  Init_libevent_http_request();
}
