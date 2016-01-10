require(File.join(ENV['gubg'], 'shared'))
require('gubg/build/Executable')
include GUBG

task :default => :help
task :help do
    puts("The following tasks can be specified:")
    puts("* declare: installs bash, vim and git scripts to GUBG.shared")
    puts("* define: does nothing for now")
end

task :declare do
	case os
	when :linux, :osx
		publish('src/bash', dst: 'bin', mode: 0755)
	when :windows
		# publish('src/bat', dst: 'bin')
	else raise("Unknown os #{os}") end

    Rake::Task['task_warrior:declare'].invoke
end

task :define => :declare do
    hours = Build::Executable.new('hours')
    hours.add_include_path(shared_dir('include'))
    hours.add_sources('src/app/hours/hours.cpp')
    hours.add_library_path(shared_dir('lib'))
    hours.add_library('gubg.io')
    hours.build
    publish(hours.exe_filename, dst: 'bin')
end

namespace :task_warrior do
    task :declare do
        build_ok_fn = 'gubg.build.ok'
        Dir.chdir(shared_dir('extern')) do
            case os
            when :linux, :osx
                #Depends on the following ubuntu packages:
                #sudo apt-get install libgnutls-dev uuid-dev
                git_clone('https://git.tasktools.org/scm/tm', 'task') do
                    if !File.exist?('build')
                        Dir.mkdir('trybuild')
                        Dir.chdir('trybuild') do
                            sh 'cmake ..'
                            sh 'make -j 4'
                            %w[calc lex task].each{|exe|cp "src/#{exe}", shared_dir('bin')}
                        end
                        mv('trybuild', 'build')
                    end
                end if which('cmake')
            when :windows
            else raise("Unknown os #{os}") end
        end
    end
end

task :test
