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
    Rake::Task['hours:publish'].invoke
    Rake::Task['pa:publish'].invoke
end

task :test do
    Rake::Task['pa:test'].invoke
end

namespace :pa do
    pa = nil
    task :setup do
        pa = Build::Executable.new('pa')
        pa.add_define('DEBUG')
        pa.add_option('g')
        pa.add_include_path(shared_dir('include'))
        pa.add_include_path('src/app')
        pa.add_sources(FileList.new('src/app/pa/**/*.cpp'))
        pa.add_sources(FileList.new('src/app/pa/**/*.hpp'))
        pa.add_sources(FileList.new(shared('include/**/*.hpp')))
        pa.add_library_path(shared_dir('lib'))
        pa.add_library('dl', 'gubg.io', 'pthread')
    end
    task :build => :setup do
        pa.build
    end
    task :publish => :build do
        publish(pa.exe_filename, dst: 'bin')
    end
    task :test => :publish do
        input_fn = 'src/app/test/pa/TODO.mm'
        workers_fn = 'src/app/test/pa/workers.chai'
        output_fn = 'TODO.txt'
        options = []
        # options += ['-h']
        options += ['-i', input_fn]
        options += ['-w', workers_fn]
        options += ['-t', 'estimate']
        options += ['-f', 'todo']
        options += ['-l', '/Product A']
        options += ['-l', '/Product B']
        options += ['-o', output_fn]
        options += ['-P']
        sh shared_file(%w[bin pa.exe]), *options
    end
end

namespace :hours do
    hours = nil
    task :setup do
        hours = Build::Executable.new('hours')
        hours.add_include_path(shared_dir('include'))
        hours.add_sources('src/app/hours/hours.cpp')
        hours.add_library_path(shared_dir('lib'))
        hours.add_library('gubg.io')
    end
    task :build => :setup do
        hours.build
    end
    task :publish => :build do
        publish(hours.exe_filename, dst: 'bin')
    end
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
