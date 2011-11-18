## Libevent

C extension to [libevent](http://libevent.org/) library.

## Dependencies

* libevent v2

## Documentation

Please read [libevent rubydoc](http://rubydoc.info/github/ayanko/libevent/frames)

## Installation

    gem install libevent

## Using Libevent HTTP server

### From scratch

Simple server

    require "libevent" 

    # create event base
    base = Libevent::Base.new

    # create http server instance
    http = Libevent::Http.new(base)

    # bind socket
    http.bind_socket("0.0.0.0", 15015)

    # set handler
    http.handler do |request|
      request.send_reply(200, {}, ["Hello World\n"])
    end

    # catch SIGINT
    base.trap_signal("INT") { base.exit_loop }
 
    # start libevent loop
    base.dispatch

Check with curl

    $ curl -v http://localhost:15015
    > GET / HTTP/1.1
    > User-Agent: curl/7.22.0 (x86_64-unknown-linux-gnu) libcurl/7.22.0 OpenSSL/1.0.0e zlib/1.2.5 libssh2/1.3.0
    > Host: localhost:15015
    > Accept: */*
    > 
    < HTTP/1.1 200 OK
    < Transfer-Encoding: chunked
    < Date: Fri, 18 Nov 2011 19:09:04 GMT
    < Content-Type: text/html; charset=ISO-8859-1
    < 
    Hello World


See also `samples` directory
