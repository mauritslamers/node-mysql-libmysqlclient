/*
Copyright (C) 2010, Oleg Efimov <efimovov@gmail.com>

See license text in LICENSE file
*/

// Load configuration
var cfg = require("./config").cfg;

// Require modules
var
  sys = require("sys"),
  mysql_libmysqlclient = require("../mysql-libmysqlclient");

exports.mysql_libmysqlclient_createConnectionSync_0 = function (test) {
  test.expect(1);
  
  var conn = mysql_libmysqlclient.createConnectionSync();
  test.ok(conn, "mysql_libmysqlclient.createConnectionSync()");
  conn.closeSync();
  
  test.done();
};

exports.mysql_libmysqlclient_createConnectionSync_1 = function (test) {
  test.expect(2);
  
  var conn = mysql_libmysqlclient.createConnectionSync(cfg.host);
  test.ok(conn, "mysql_libmysqlclient.createConnectionSync(host)");
  test.ok(conn.connectedSync(), "mysql_libmysqlclient.createConnectionSync(host).connectedSync()");
  if (!conn.connectedSync()) {
    sys.puts("Error:" + conn.connectErrorSync());
  }
  conn.closeSync();
  
  test.done();
};

exports.mysql_libmysqlclient_createConnectionSync_2 = function (test) {
  test.expect(2);
  
  var conn = mysql_libmysqlclient.createConnectionSync(cfg.host, cfg.user);
  test.ok(conn, "mysql_libmysqlclient.createConnectionSync(host, user)");
  test.ok(conn.connectedSync(), "mysql_libmysqlclient.createConnectionSync(host, user).connectedSync()");
  if (!conn.connectedSync()) {
    sys.puts("Error:" + conn.connectErrorSync());
  }
  conn.closeSync();
  
  test.done();
};

exports.mysql_libmysqlclient_createConnectionSync_3 = function (test) {
  test.expect(2);
  
  var conn = mysql_libmysqlclient.createConnectionSync(cfg.host, cfg.user, cfg.password);
  test.ok(conn, "mysql_libmysqlclient.createConnectionSync(host, user, password)");
  test.ok(conn.connectedSync(), "mysql_libmysqlclient.createConnectionSync(host, user, password).connectedSync()");
  if (!conn.connectedSync()) {
    sys.puts("Error:" + conn.connectErrorSync());
  }
  conn.closeSync();
  
  test.done();
};

exports.mysql_libmysqlclient_createConnectionSync_4_allowed = function (test) {
  test.expect(2);
  
  var conn = mysql_libmysqlclient.createConnectionSync(cfg.host, cfg.user, cfg.password, cfg.database);
  test.ok(conn, "mysql_libmysqlclient.createConnectionSync(host, user, password, database)");
  test.ok(conn.connectedSync(), "mysql_libmysqlclient.createConnectionSync(host, user, password, database).connectedSync()");
  if (!conn.connectedSync()) {
    sys.puts("Error:" + conn.connectErrorSync());
  }
  conn.closeSync();
  
  test.done();
};

exports.mysql_libmysqlclient_createConnectionSync_4_denied = function (test) {
  test.expect(1);
  
  var conn = mysql_libmysqlclient.createConnectionSync(cfg.host, cfg.user, cfg.password, cfg.database_denied);
  test.ok(!conn.connectedSync(), "!mysql_libmysqlclient.createConnectionSync(host, user, password, database_denied).connectedSync()");
  
  test.done();
};
