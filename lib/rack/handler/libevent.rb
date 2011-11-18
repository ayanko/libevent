require "libevent"
require "stringio"

module Rack
  module Handler
    class Libevent

      def self.run(app, options)
        server = new(app, options)
        server.start
      end

      def self.valid_options
        {
          "timeout=TIMEOUT" => "Set the timeout for an HTTP request"
        }
      end

      def initialize(app, options)
        @app = app
        
        @host = options[:Host] or raise ArgumentError, "Host option required"
        @port = options[:Port] or raise ArgumentError, "Port option required"

        @base = ::Libevent::Base.new
        @http = ::Libevent::Http.new(@base)

        @http.set_timeout(options[:timeout].to_i) if options[:timeout]
      end

      def start
        @http.bind_socket(@host, @port) or raise RuntimeError, "Can't bind to #{@host}:#{@port}"
        @http.set_request_handler(self.method(:process))

        @base.trap_signal("INT")  { self.stop }
        @base.trap_signal("TERM") { self.stop }

        @base.dispatch
      end

      def stop
        @base.exit_loop
      end

      protected

      def process(request)
        env = {}

        env['REQUEST_METHOD']    = request.get_command
        env['SCRIPT_NAME']       = ''
        env['REQUEST_PATH']      = '/'
        env['PATH_INFO']         = request.get_uri_path  || '/'
        env['QUERY_STRING']      = request.get_uri_query || ''
        env['SERVER_NAME']       = request.get_host || @host
        env['SERVER_PORT']       = @port.to_s
        env['SERVER_SOFTWARE']   = "libevent/#{::Libevent::VERSION}"
        env['SERVER_PROTOCOL']   = 'HTTP/1.1'
        env['REMOTE_ADDR']       = request.get_remote_host
        env['HTTP_VERSION']      = request.get_http_version
        env['rack.version']      = [1, 1]
        env['rack.url_scheme']   = request.get_uri_scheme || 'http'
        env['rack.input']        = StringIO.new(request.get_body)
        env['rack.errors']       = STDERR
        env['rack.multithread']  = false
        env['rack.multiprocess'] = false
        env['rack.run_once']     = false

        request.get_input_headers.each do |key, val|
          env_key = ""
          env_key << "HTTP_" unless key =~ /^content(_|-)(type|length)$/i
          env_key << key
          env_key.gsub!('-','_')
          env_key.upcase!
          env[env_key] = val
        end

        code, headers, body = @app.call(env)

        begin
          headers.each do |key, values|
            values.split("\n").each { |value| request.add_output_header(key, value) }
          end
          request.send_reply_start(code, nil)
          body.each { |chunk| request.send_reply_chunk(chunk) }
          request.send_reply_end
        ensure
          body.close if body.respond_to?(:close)
        end
      end
    end

  end
end
