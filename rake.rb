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
		# mode = :debug
		
		cooker = Build::Cooker.new().option("c++.std", 20).option(mode).output("bin")

		recipes = filter_recipes(args, %w[time_track pit pigr org/app org/ut])

		cooker.generate(:ninja, *recipes).ninja()
	end
end

require('open3')
def exp_ok(*cmd, print: false, &block)
	puts("\n>> Running '#{cmd*' '}'") if print
	start_ts = Time.now()
	output, status = Open3.capture2e(*cmd)
	stop_ts = Time.now()
	puts("Duration: #{stop_ts-start_ts}") if print
	if !status.success?()
		puts(output)
		raise("Error: Expected '#{cmd*' '}' to pass")
	end
	puts(output) if print
	block.(output) if block
	puts('<<') if print
end

namespace :org do
	desc "Test"
	task :test => :"tools.pm:install" do
		sh('org.ut')
		exp_ok('org', '-h', print: true)
		exp_ok('org', '-i', "#{here}/test/org/a.md", print: true)
		# exp_ok('org', '-i', "/home/geertf/Downloads/consoleText", print: true)
	end
end
