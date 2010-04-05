/*
Copyright (C) 2010, Oleg Efimov <efimovov@gmail.com>

See license text in LICENSE file
*/

#include "./mysql_bindings_connection.h"
#include "./mysql_bindings_result.h"

Persistent<FunctionTemplate> MysqlConn::MysqlResult::constructor_template;

void MysqlConn::MysqlResult::Init(Handle<Object> target) {
    HandleScope scope;

    Local<FunctionTemplate> t = FunctionTemplate::New(New);

    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->Inherit(EventEmitter::constructor_template);
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    constructor_template->SetClassName(String::NewSymbol("MysqlResult"));

    ADD_PROTOTYPE_METHOD(result, fetchAll, FetchAll);
    ADD_PROTOTYPE_METHOD(result, fetchArray, FetchArray);
    ADD_PROTOTYPE_METHOD(result, fetchFields, FetchFields);
    ADD_PROTOTYPE_METHOD(result, fetchLengths, FetchLengths);
    ADD_PROTOTYPE_METHOD(result, fetchObject, FetchObject);
    ADD_PROTOTYPE_METHOD(result, fieldCount, FieldCount);
    ADD_PROTOTYPE_METHOD(result, numRows, NumRows);
}

MysqlConn::MysqlResult::MysqlResult(): EventEmitter() {}

MysqlConn::MysqlResult::~MysqlResult() {}

Handle<Value> MysqlConn::MysqlResult::New(const Arguments& args) {
    HandleScope scope;

    REQ_EXT_ARG(0, js_res);
    uint32_t field_count = args[1]->IntegerValue();
    MYSQL_RES *res = static_cast<MYSQL_RES*>(js_res->Value());
    MysqlResult *my_res = new MysqlResult(res, field_count);
    my_res->Wrap(args.This());

    return args.This();
}

Handle<Value> MysqlConn::MysqlResult::FetchAll(const Arguments& args) {
    HandleScope scope;

    MysqlResult *res = OBJUNWRAP<MysqlResult>(args.This());

    // TODO(Sannis): Is it possible?
    if (!res->_res) {
        return scope.Close(False());
    }

    MYSQL_FIELD *fields = mysql_fetch_fields(res->_res);
    uint32_t num_fields = mysql_num_fields(res->_res);
    MYSQL_ROW result_row;
    uint32_t i = 0, j = 0;

    Local<Array> js_result = Array::New();
    Local<Object> js_result_row;
    Local<Value> js_field;

    i = 0;
    while ( (result_row = mysql_fetch_row(res->_res)) ) {
        js_result_row = Object::New();

        for ( j = 0; j < num_fields; j++ ) {
            switch (fields[j].type) {
              MYSQL_TYPE_BIT:

              MYSQL_TYPE_TINY:
              MYSQL_TYPE_SHORT:
              MYSQL_TYPE_LONG:

              MYSQL_TYPE_LONGLONG:
              MYSQL_TYPE_INT24:
                js_field = String::New(result_row[j])->ToInteger();
                break;
              MYSQL_TYPE_DECIMAL:
              MYSQL_TYPE_FLOAT:
              MYSQL_TYPE_DOUBLE:
                js_field = String::New(result_row[j])->ToNumber();
                break;
              // TODO(Sannis): Handle other types, dates in first order
              /*  MYSQL_TYPE_NULL,   MYSQL_TYPE_TIMESTAMP,
                MYSQL_TYPE_DATE,   MYSQL_TYPE_TIME,
                MYSQL_TYPE_DATETIME, MYSQL_TYPE_YEAR,
                MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,*/
                /*MYSQL_TYPE_NEWDECIMAL=246,
                MYSQL_TYPE_ENUM=247,
                MYSQL_TYPE_SET=248,
                MYSQL_TYPE_TINY_BLOB=249,
                MYSQL_TYPE_MEDIUM_BLOB=250,
                MYSQL_TYPE_LONG_BLOB=251,
                MYSQL_TYPE_BLOB=252,*/
              MYSQL_TYPE_VAR_STRING:
              MYSQL_TYPE_STRING:
                js_field = String::New(result_row[j]);
                break;
                /*MYSQL_TYPE_GEOMETRY=255*/
              default:
                js_field = String::New(result_row[j]);
            }

            js_result_row->Set(String::New(fields[j].name), js_field);
        }

        js_result->Set(Integer::New(i), js_result_row);

        i++;
    }

    return scope.Close(js_result);
}

Handle<Value> MysqlConn::MysqlResult::FetchArray(const Arguments& args) {
    HandleScope scope;

    MysqlResult *res = OBJUNWRAP<MysqlResult>(args.This());

    // TODO(Sannis): Is it possible?
    if (!res->_res) {
        return scope.Close(False());
    }

    MYSQL_FIELD *fields = mysql_fetch_fields(res->_res);
    uint32_t num_fields = mysql_num_fields(res->_res);
    MYSQL_ROW result_row;
    uint32_t j = 0;

    Local<Array> js_result_row;
    Local<Value> js_field;

    result_row = mysql_fetch_row(res->_res);

    if (!result_row) {
        return scope.Close(False());
    }

    js_result_row = Array::New();

    for ( j = 0; j < num_fields; j++ ) {
        switch (fields[j].type) {
          MYSQL_TYPE_BIT:

          MYSQL_TYPE_TINY:
          MYSQL_TYPE_SHORT:
          MYSQL_TYPE_LONG:

          MYSQL_TYPE_LONGLONG:
          MYSQL_TYPE_INT24:
            js_field = String::New(result_row[j])->ToInteger();
            break;
          MYSQL_TYPE_DECIMAL:
          MYSQL_TYPE_FLOAT:
          MYSQL_TYPE_DOUBLE:
            js_field = String::New(result_row[j])->ToNumber();
            break;
          // TODO(Sannis): Handle other types, dates in first order
          /*  MYSQL_TYPE_NULL,   MYSQL_TYPE_TIMESTAMP,
            MYSQL_TYPE_DATE,   MYSQL_TYPE_TIME,
            MYSQL_TYPE_DATETIME, MYSQL_TYPE_YEAR,
            MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,*/
            /*MYSQL_TYPE_NEWDECIMAL=246,
            MYSQL_TYPE_ENUM=247,
            MYSQL_TYPE_SET=248,
            MYSQL_TYPE_TINY_BLOB=249,
            MYSQL_TYPE_MEDIUM_BLOB=250,
            MYSQL_TYPE_LONG_BLOB=251,
            MYSQL_TYPE_BLOB=252,*/
          MYSQL_TYPE_VAR_STRING:
          MYSQL_TYPE_STRING:
            js_field = String::New(result_row[j]);
            break;
            /*MYSQL_TYPE_GEOMETRY=255*/
          default:
            js_field = String::New(result_row[j]);
        }

        js_result_row->Set(Integer::New(j), js_field);
    }

    return scope.Close(js_result_row);
}

void v8_add_field_properties(Local<Object> &obj, const MYSQL_FIELD *field)
{
    obj->Set(String::New("name"), String::New(field->name ? field->name : ""));
    obj->Set(String::New("orgname"), String::New(field->org_name ? field->org_name : ""));
    obj->Set(String::New("table"), String::New(field->table ? field->table : ""));
    obj->Set(String::New("orgtable"), String::New(field->org_table ? field->org_table : ""));
    obj->Set(String::New("def"), String::New(field->def ? field->def : ""));

    obj->Set(String::New("max_length"), Integer::New(field->max_length));
    obj->Set(String::New("length"), Integer::New(field->length));
    obj->Set(String::New("charsetnr"), Integer::New(field->charsetnr));
    obj->Set(String::New("flags"), Integer::New(field->flags));
    obj->Set(String::New("type"), Integer::New(field->type));
    obj->Set(String::New("decimals"), Integer::New(field->decimals));
}

Handle<Value> MysqlConn::MysqlResult::FetchFields(const Arguments& args) {
    HandleScope scope;

    MysqlResult *res = OBJUNWRAP<MysqlResult>(args.This());

    // TODO(Sannis): Is it possible?
    if (!res->_res) {
        return scope.Close(False());
    }

    uint32_t num_fields = mysql_num_fields(res->_res);
    const MYSQL_FIELD *field;
    uint32_t i = 0;

    Local<Array> js_result = Array::New();
    Local<Object> js_result_obj;

	for (i = 0; i < num_fields; i++) {
		field = mysql_fetch_field_direct(res->_res, i);

        js_result_obj = Object::New();
		v8_add_field_properties(js_result_obj, field);

		js_result->Set(Integer::New(i), js_result_obj);
	}

    return scope.Close(js_result);
}

Handle<Value> MysqlConn::MysqlResult::FetchLengths(const Arguments& args) {
    HandleScope scope;

    MysqlResult *res = OBJUNWRAP<MysqlResult>(args.This());

    // TODO(Sannis): Is it possible?
    if (!res->_res) {
        return scope.Close(False());
    }

    uint32_t num_fields = mysql_num_fields(res->_res);
    unsigned long *lengths;
    uint32_t i = 0;

    Local<Array> js_result = Array::New();

    lengths = mysql_fetch_lengths(res->_res);

    if (!lengths) {
        return scope.Close(False());
    }

    for (i = 0; i < num_fields; i++) {
        js_result->Set(Integer::New(i), Integer::New(lengths[i]));
    }

    return scope.Close(js_result);
}

Handle<Value> MysqlConn::MysqlResult::FetchObject(const Arguments& args) {
    HandleScope scope;

    MysqlResult *res = OBJUNWRAP<MysqlResult>(args.This());

    // TODO(Sannis): Is it possible?
    if (!res->_res) {
        return scope.Close(False());
    }

    MYSQL_FIELD *fields = mysql_fetch_fields(res->_res);
    uint32_t num_fields = mysql_num_fields(res->_res);
    MYSQL_ROW result_row;
    uint32_t j = 0;

    Local<Object> js_result_row;
    Local<Value> js_field;

    result_row = mysql_fetch_row(res->_res);

    if (!result_row) {
        return scope.Close(False());
    }

    js_result_row = Object::New();

    for ( j = 0; j < num_fields; j++ ) {
        switch (fields[j].type) {
          MYSQL_TYPE_BIT:

          MYSQL_TYPE_TINY:
          MYSQL_TYPE_SHORT:
          MYSQL_TYPE_LONG:

          MYSQL_TYPE_LONGLONG:
          MYSQL_TYPE_INT24:
            js_field = String::New(result_row[j])->ToInteger();
            break;
          MYSQL_TYPE_DECIMAL:
          MYSQL_TYPE_FLOAT:
          MYSQL_TYPE_DOUBLE:
            js_field = String::New(result_row[j])->ToNumber();
            break;
          // TODO(Sannis): Handle other types, dates in first order
          /*  MYSQL_TYPE_NULL,   MYSQL_TYPE_TIMESTAMP,
            MYSQL_TYPE_DATE,   MYSQL_TYPE_TIME,
            MYSQL_TYPE_DATETIME, MYSQL_TYPE_YEAR,
            MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,*/
            /*MYSQL_TYPE_NEWDECIMAL=246,
            MYSQL_TYPE_ENUM=247,
            MYSQL_TYPE_SET=248,
            MYSQL_TYPE_TINY_BLOB=249,
            MYSQL_TYPE_MEDIUM_BLOB=250,
            MYSQL_TYPE_LONG_BLOB=251,
            MYSQL_TYPE_BLOB=252,*/
          MYSQL_TYPE_VAR_STRING:
          MYSQL_TYPE_STRING:
            js_field = String::New(result_row[j]);
            break;
            /*MYSQL_TYPE_GEOMETRY=255*/
          default:
            js_field = String::New(result_row[j]);
        }

        js_result_row->Set(String::New(fields[j].name), js_field);
    }

    return scope.Close(js_result_row);
}

Handle<Value> MysqlConn::MysqlResult::FieldCount(const Arguments& args) {
    HandleScope scope;

    MysqlResult *res = OBJUNWRAP<MysqlResult>(args.This());

    // TODO(Sannis): Is it possible?
    if (!res->_res) {
        return scope.Close(False());
    }

    if (res->field_count > 0) {
        return scope.Close(Integer::New(res->field_count));
    } else {
        return Undefined();
    }
}

Handle<Value> MysqlConn::MysqlResult::NumRows(const Arguments& args) {
    HandleScope scope;

    MysqlResult *res = OBJUNWRAP<MysqlResult>(args.This());

    // TODO(Sannis): Is it possible?
    if (!res->_res) {
        return scope.Close(False());
    }

    if (mysqli_result_is_unbuffered(res->_res)) {
        return THREXC("Function cannot be used with MYSQL_USE_RESULT");
    }

    Local<Value> js_result = Integer::New(mysql_num_rows(res->_res));

    return scope.Close(js_result);
}

