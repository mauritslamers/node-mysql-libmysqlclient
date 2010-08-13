/*
Copyright (C) 2010, Oleg Efimov <efimovov@gmail.com>

See license text in LICENSE file
*/


var binding = require("./mysql_bindings");

var iDispatchQuery = 0;

// libmysqlclient cannot handle multiple queries at the same time.
// thus we must queue them internally and dispatch them as
// others come in. 
binding.MysqlConn.prototype.maybeDispatchQuery = function () {
  // If queries queue empty, do not dispatch. 
  if (!this.queries_queue || (this.queries_queue.length <= 0)) {
    return;
  }
  
  // If not connected, do not dispatch. 
  if (!this.connected()) {
    return;
  }
  
  iDispatchQuery += 1;
  require("sys").debug("Before dispatchQuery #" + iDispatchQuery);
  this.current_query = this.queries_queue.shift();
  this.dispatchQuery(this.current_query[0], this.current_query[1]);
};

binding.MysqlConn.prototype.queryAsync = function (query, callback) {
  this.queries_queue = this.queries_queue || [];
  this.queries_queue.push([query, callback]);
  
  var self = this;
  //process.nextTick(function () {
  setTimeout(function () {
    self.maybeDispatchQuery();
  }, 0);
  //});
};

exports.createConnection = function (servername, user, password, dbname, port, socket)
{
  var db = new binding.MysqlConn();
  if (arguments.length > 0) {
    db.connect.apply(db, Array.prototype.slice.call(arguments, 0, 6));
  }
  
  db.addListener("ready", function () {
    //process.nextTick(function () {
    setTimeout(function () {
      db.maybeDispatchQuery();
    }, 0);
    //});
  });
  
  return db;
};

