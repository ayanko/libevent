require "mkmf"

$CFLAGS << ' -Wall '

$LDFLAGS << ' ' << `pkg-config --libs libevent`

create_makefile('libevent_ext')
