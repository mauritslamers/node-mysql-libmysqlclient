/*
Copyright (C) 2010, Oleg Efimov <efimovov@gmail.com>

See license text in LICENSE file
*/


var binding = require("./mysql_bindings");
  
// libmysqlclient cannot handle multiple queries at the same time.
// thus we must queue them internally and dispatch them as
// others come in. 
binding.MysqlConn.prototype.maybeDispatchQuery = function () {
  // If queries queue empty, do not dispatch. 
  if (!this.queries_queue || !(this.queries_queue.length > 0)) return;
  
  // If not connected, do not dispatch. 
  if (!this.connected()) return;
  
  //if (!this.current_query) {
    this.current_query = this.queries_queue.shift();
    this.dispatchQuery(this.current_query[0], this.current_query[1], this.current_query[2]);
  //}
};

binding.MysqlConn.prototype.queryAsync = function (query, callback, result_mode_store) {
  if (typeof result_mode_store == 'undefined') {
    result_mode_store = false;
  }
  
  this.queries_queue = this.queries_queue || [];
  this.queries_queue.push([query, callback, result_mode_store]);
  
  this.maybeDispatchQuery();
};

exports.createConnection = function(servername, user, password, dbname, port, socket)
{
  var db = new binding.MysqlConn();
  if (arguments.length > 0) {
    db.connect.apply(db, Array.prototype.slice.call(arguments, 0, 6));
  }

  /*
  db.addListener("connect", function () {
    c.maybeDispatchQuery();
  });

  db.addListener("result", function () {
    process.assert(db.currentQuery);
    var callback = db.currentQuery[1];
    db.currentQuery = null;
    if (callback) {
      callback.apply(db, arguments);
    }
  });
  */
  
  db.addListener("ready", function () {
    require("sys").puts("ready event");
    db.maybeDispatchQuery();
  });
  
  return db;
}
