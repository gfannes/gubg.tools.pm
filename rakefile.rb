require(File.join(ENV['gubg'], 'shared'))
require('gubg/build/Executable')
include GUBG

task :default do
    sh "rake -T"
end

task :clean do
    rm_rf '.cache'
end

task :prepare do
	case os
	when :linux, :osx
		publish('src/bash', dst: 'bin', mode: 0755)
	when :windows
		# publish('src/bat', dst: 'bin')
	else raise("Unknown os #{os}") end

    Rake::Task['task_warrior:declare'].invoke
end

task :run do
    Rake::Task['hours:publish'].invoke
    # Rake::Task['pa:publish'].invoke
    # Rake::Task['tt:publish'].invoke
    Rake::Task['timeline:publish'].invoke
end

task :test do
    Rake::Task['pa:test'].invoke
end

namespace :timeline do
    timeline_dir = shared('extern/timeline')
    file timeline_dir do
        Dir.chdir(shared_dir('extern')) do
            url = 'https://sourceforge.net/projects/thetimelineproj/files/thetimelineproj/1.10.0/timeline-1.10.0.zip'
            sh "wget #{url}"
            base = 'timeline-1.10.0'
            zip_fn = "#{base}.zip"
            sh "unzip #{zip_fn}"
            sh "mv #{base} timeline"
        end
    end
    task :install_deps do
        sh 'sudo apt install python-wxversion'
        sh 'sudo apt install python-wxtools'
        sh 'pip install --user git+https://github.com/thetimelineproj/humblewx.git'
    end
    task :publish => timeline_dir do
    end
end

namespace :pa do
    pa = nil
    task :setup do
        pa = Build::Executable.new('pa')
        case :debug
        when :debug
            pa.add_option('g')
        else
            pa.add_define('NDEBUG')
        end
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

namespace :tt do
    tt = nil
    task :setup do
        tt = Build::Executable.new('tt')
        case :release
        when :debug
            tt.add_option('g')
        else
            tt.add_define('NDEBUG')
        end
        tt.add_include_path(shared_dir('include'))
        tt.add_include_path('src/app')
        tt.add_sources(FileList.new('src/app/tt/**/*.cpp'))
        tt.add_sources(FileList.new('src/app/tt/**/*.hpp'))
        tt.add_sources(FileList.new(shared('include/**/*.hpp')))
        tt.add_library_path(shared_dir('lib'))
        tt.add_library('dl', 'gubg.io', 'pthread')
    end
    task :build => :setup do
        tt.build
    end
    task :publish => :build do
        publish(tt.exe_filename, dst: 'bin')
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
