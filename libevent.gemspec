# -*- encoding: utf-8 -*-
$:.push File.expand_path("../lib", __FILE__)
require "libevent/version"

Gem::Specification.new do |s|
  s.name        = "libevent"
  s.version     = Libevent::VERSION
  s.authors     = ["Andriy Yanko"]
  s.email       = ["andriy.yanko@gmail.com"]
  s.homepage    = "https://github.com/ayanko/libevent"
  s.summary     = %q{C extension for libevent}
  s.description = %q{C extension for libevent}

  s.rubyforge_project = "libevent"

  s.files         = `git ls-files`.split("\n")
  s.test_files    = `git ls-files -- {test,spec,features}/*`.split("\n")
  s.executables   = `git ls-files -- bin/*`.split("\n").map{ |f| File.basename(f) }
  s.require_paths = ["lib"]

  s.extensions = ["ext/libevent_ext/extconf.rb"]
end
