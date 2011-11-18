module Libevent
  class Http

    attr_reader :base

    # Create virtual http server
    # @param [String] domain a domain for virtual server
    # @return [Http] http instance
    def vhost(domain)
      http = self.class.new(self.base)
      add_virtual_host(domain, http)
      yield(http) if block_given?
      http
    end

    # Set request handler for current http instance
    # @param block
    def handler(&block)
      set_request_handler(block)
    end

  end
end
