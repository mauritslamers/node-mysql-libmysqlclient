import Options, Utils
from os import unlink, symlink, chdir
from os.path import exists

srcdir = "."
blddir = "build"
VERSION = "0.0.6"

def set_options(opt):
  opt.tool_options("compiler_cxx")
  opt.add_option('--mysql-config', action='store', default='mysql_config', help='Path to mysql_config, e.g. /usr/bin/mysql_config')

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("node_addon")
  
  conf.env.append_unique('CXXFLAGS', ["-g", "-D_FILE_OFFSET_BITS=64","-D_LARGEFILE_SOURCE", "-Wall"])
  
  conf.env.append_unique('CXXFLAGS', Utils.cmd_output(Options.options.mysql_config + ' --include').split())
  conf.env.append_unique('LINKFLAGS', Utils.cmd_output(Options.options.mysql_config + ' --libs_r').split())
  
  if not conf.check_cxx(header_name='mysql.h'):
    conf.fatal("Missing mysql.h header from libmysqlclient-devel or mysql-devel package")
  
  if not conf.check_cxx(lib="mysqlclient_r"):
    conf.fatal("Missing libmysqlclient library")
  
  #if not conf.check_cfg(package="mysqlclient_r", args="--cflags --libs", uselib_store="MYSQLCLIENT"):
  #  if not conf.check_cxx(lib="mysqlclient_r", linkflags="-L/usr/lib/mysql", uselib_store="MYSQLCLIENT"):
  #    conf.fatal("Missing libmysqlclient library")
  #if conf.check_cxx(header_name='mysql/mysql.h'):
  #  conf.env.MYSQL_H_PATH_FLAG = "-I/usr/include/mysql"
  #else:
  #  if conf.check_cxx(header_name='mysql.h'):
  #    conf.env.MYSQL_H_PATH_FLAG = "-I/usr/include"
  #  else:
  #    conf.fatal("Missing mysql.h header from libmysqlclient-devel or mysql-devel package")

def build(bld):
  obj = bld.new_task_gen("cxx", "shlib", "node_addon")
  obj.target = "mysql_bindings"
  obj.source = "./src/mysql_bindings_connection.cc ./src/mysql_bindings_result.cc ./src/mysql_bindings_statement.cc"
  obj.uselib = "MYSQLCLIENT"

def test(tst):
  if not exists('./tests/nodeunit/lib/testrunner.js'):
    print("\033[31mNodeunit doesn't exists.\033[39m\nYou should run `git submodule update --init` before run tests.");
    exit(1);
  else:
    Utils.exec_command('./tests/run-tests.sh')

def lint(lnt):
  # Bindings C++ source code
  Utils.exec_command('cpplint ./src/*.h ./src/*.cc')
  # Bindings javascript code
  Utils.exec_command('nodelint ./mysql-libmysqlclient.js')
  # Bindings tests
  Utils.exec_command('nodelint ./tests/*.js')
  # Bindings benchmarks
  Utils.exec_command('nodelint ./benchmark/benchmark.js')

def shutdown():
  # HACK to get bindings.node out of build directory.
  # better way to do this?
  t = 'mysql_bindings.node'
  if Options.commands['clean']:
    if exists(t): unlink(t)
  else:
    if exists('build/default/' + t) and not exists(t):
      symlink('build/default/' + t, t)

