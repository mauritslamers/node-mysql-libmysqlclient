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

exports.async = function (test) {
  var conn = mysql_libmysqlclient.createConnection(cfg.host, cfg.user, cfg.password, cfg.database);
  
  conn.async(function () {
    conn.close();
    test.done();
  });
};

exports.queryAsync = function (test) {
  var conn = mysql_libmysqlclient.createConnection(cfg.host, cfg.user, cfg.password, cfg.database);
  
  conn.queryAsync("SHOW TABLES;", function (result) {
    conn.close();
    test.done();
  });
};

exports.queryAsyncMany = function (test) {
  var conn = mysql_libmysqlclient.createConnection(cfg.host, cfg.user, cfg.password, cfg.database),
    res,
    random_number,
    random_boolean,
    last_insert_id,
    i;
  
  var irc = cfg.insert_rows_count
  irc = 100;
  
  res = conn.query("DROP TABLE IF EXISTS " + cfg.test_table + ";");
  res = conn.query("CREATE TABLE " + cfg.test_table +
    " (autoincrement_id BIGINT NOT NULL AUTO_INCREMENT," +
    " random_number INT(8) NOT NULL, random_boolean BOOLEAN NOT NULL," +
    " PRIMARY KEY (autoincrement_id)) TYPE=MEMORY;");

  test.ok(res, "conn.query('DELETE FROM cfg.test_table')");
  
  for (i = 0; i < irc; i += 1)
  {
    random_number = Math.round(Math.random() * 1000000);
    random_boolean = (Math.random() > 0.5) ? 1 : 0;
    sys.puts("Before #" + i);
    j = i;
    conn.queryAsync("INSERT INTO " + cfg.test_table +
      " (random_number, random_boolean) VALUES ('" + random_number +
      "', '" + random_boolean + "');", function (result) {sys.puts(j);}, false);
  }

  setTimeout( function(){
    last_insert_id = conn.lastInsertId();
    test.equals(last_insert_id, irc, "conn.lastInsertId() after " + irc + " acynchronous queries");
    conn.close();
    test.done();
  }, 1000);
};

