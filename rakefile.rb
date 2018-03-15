require_relative("../gubg.build/bootstrap.rb")
require("gubg/shared")
include GUBG

task :prepare do
	case os
	when :linux, :osx
		publish('src/bash', dst: 'bin', mode: 0755)
	when :windows
		# publish('src/bat', dst: 'bin')
	else raise("Unknown os #{os}") end

    Rake::Task['task_warrior:declare'].invoke
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
                        rm_rf('trybuild') if File.exist?('trybuild')
                        Dir.mkdir('trybuild')
                        Dir.chdir('trybuild') do
                            sh 'cmake ..'
                            sh 'make -j 4'
                        end
                        mv('trybuild', 'build')
                    end
                    %w[calc lex task].each{|exe|publish('build/src', pattern: exe, dst: 'bin')}
                end if which('cmake')
            when :windows
            else raise("Unknown os #{os}") end
        end if false
    end
end
