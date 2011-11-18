## Libevent

C extension to [libevent](http://libevent.org/) library.

## Description

The nice feature of libevent is it already contains build in HTTP server (evhttp).

Currently libevent extension implements mostly http server.

## Dependencies

* libevent v2

## Documentation

Please read [libevent rubydoc](http://rubydoc.info/github/ayanko/libevent/frames)

## Installation

    gem install libevent

## Using Libevent HTTP server

Check `samples` directory

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

### Server with virtual hosts

    require "libevent"

    Libevent::Builder.new do

      server "0.0.0.0", 3000 do |http|

	http.handler do |request|
	  case request.get_uri_path
	  when '/hello'
	    request.send_reply 200, { 'Content->Type' => 'text/plain'},  [ "Hello World" ]
	  when '/api'
	    request.send_reply 200, { 'Content->Type' => 'application/json'},  [ "{\"version\":\"1.0\"}" ]
	  else
	    request.send_error 404, "Nothing Found"
	  end
	end

	http.vhost "blog.local" do |host|
	  host.handler do |request|
	    request.send_reply 200, {}, ["It's blog"]
	  end
	end

	http.vhost "wiki.local" do |host|
	  host.handler do |request|
	    request.send_reply 200, {}, ["It's wiki"]
	  end
	end

	http.vhost "*.local" do |host|
	  host.handler do |request|
	    request.send_error 404, "Please use blog.local or wiki.local"
	  end
	end

      end

      server "0.0.0.0", 3001 do |http|
	http.handler do |request|
	  request.send_reply 200, { 'Content->Type' => 'text/plain'},  [ "Hello World 3001" ]
	end
      end

      signal("INT") do
	base.exit_loop
      end

      signal("HUP") do
	Kernel.puts "HUP received ..."
      end

      dispatch

    end

### Serve Rails application

Add to `Gemfile`

    gem "libevent", :require => false

Update gems

    $ bundle install

Run application

    $ script/rails s Libevent

Or via rackup

    $ bundle exec rackup -s Libevent -p 3000

### Serve Rack application

Check rack handler `rack/handler/libevent.rb`

