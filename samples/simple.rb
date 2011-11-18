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

