require("gubg/shared")
include Gubg

here = File.dirname(__FILE__)

namespace :"tools.pm" do
	task :prepare do
		case os()
		when :linux, :macos
			publish(here, 'src/sh', dst: 'bin', mode: 0755)
		when :windows
			publish(here, 'src/bat', dst: 'bin')
		else raise("Unknown os #{os}")
		end
	end
	
	task :install do |t, args|
		require("gubg/build/Cooker")

		mode = :release
		mode = :debug
		
		cooker = Build::Cooker.new().option("c++.std", 20).option(mode).output("bin")

		recipes = filter_recipes(args, %w[time_track pit pigr org])

		cooker.generate(:ninja, *recipes).ninja()
	end
end
