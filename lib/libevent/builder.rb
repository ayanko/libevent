module Libevent
  class Builder
    def initialize(options = {}, &block)
      @base = Base.new
      instance_eval(&block) if block_given?
    end

    attr_reader :base

    # Create new Http instance, bind socket and options yield http object
    # @param [String] host
    # @param [Fixnum] port
    # @return [Http] instance
    def server(host, port, &block)
      http = Http.new(@base)
      http.bind_socket(host, port) or raise RuntimeError, "can't bind socket #{host}:#{port}"
      yield(http) if block_given?
      http
    end

    # Trap signal using event base
    # @param [String] name a signal name
    def signal(name, &block)
      base.trap_signal(name, &block)
    end

    # Start event base loop
    def dispatch
      base.dispatch
    end
  end
end
