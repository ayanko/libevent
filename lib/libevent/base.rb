module Libevent
  class Base
    # Create new event base
    def initialize
      @signals = []
    end

    attr_reader :signals

    # Create new signal with handler as block and add signal to event base
    #
    # @param [String] name of signal
    def trap_signal(name, &block)
      @signals << Signal.new(self, name, block)
    end

  end
end
