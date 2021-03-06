/*
Copyright by node-mysql-libmysqlclient contributors.
See contributors list in README

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

    ADD_PROTOTYPE_METHOD(result, dataSeekSync, DataSeekSync);
    ADD_PROTOTYPE_METHOD(result, fetchAllSync, FetchAllSync);
    ADD_PROTOTYPE_METHOD(result, fetchArraySync, FetchArraySync);
    ADD_PROTOTYPE_METHOD(result, fetchFieldSync, FetchFieldSync);
    ADD_PROTOTYPE_METHOD(result, fetchFieldDirectSync, FetchFieldDirectSync);
    ADD_PROTOTYPE_METHOD(result, fetchFieldsSync, FetchFieldsSync);
    ADD_PROTOTYPE_METHOD(result, fetchLengthsSync, FetchLengthsSync);
    ADD_PROTOTYPE_METHOD(result, fetchObjectSync, FetchObjectSync);
    ADD_PROTOTYPE_METHOD(result, fieldCountSync, FieldCountSync);
    ADD_PROTOTYPE_METHOD(result, fieldSeekSync, FieldSeekSync);
    ADD_PROTOTYPE_METHOD(result, fieldTellSync, FieldTellSync);
    ADD_PROTOTYPE_METHOD(result, freeSync, FreeSync);
    ADD_PROTOTYPE_METHOD(result, numRowsSync, NumRowsSync);
}

MysqlConn::MysqlResult::MysqlResult(): EventEmitter() {}

MysqlConn::MysqlResult::~MysqlResult() {
    this->Free();
}

void MysqlConn::MysqlResult::AddFieldProperties(
                                        Local<Object> &js_field_obj,
                                        MYSQL_FIELD *field) {
    js_field_obj->Set(String::New("name"),
                        String::New(field->name ? field->name : ""));
    js_field_obj->Set(String::New("orgname"),
                        String::New(field->org_name ? field->org_name : ""));
    js_field_obj->Set(String::New("table"),
                        String::New(field->table ? field->table : ""));
    js_field_obj->Set(String::New("orgtable"),
                        String::New(field->org_table ? field->org_table : ""));
    js_field_obj->Set(String::New("def"),
                        String::New(field->def ? field->def : ""));

    js_field_obj->Set(String::New("max_length"),
                        Integer::New(field->max_length));
    js_field_obj->Set(String::New("length"),
                        Integer::New(field->length));
    js_field_obj->Set(String::New("charsetnr"),
                        Integer::New(field->charsetnr));
    js_field_obj->Set(String::New("flags"),
                        Integer::New(field->flags));
    js_field_obj->Set(String::New("type"),
                        Integer::New(field->type));
    js_field_obj->Set(String::New("decimals"),
                        Integer::New(field->decimals));
}

void MysqlConn::MysqlResult::SetFieldValue(
                                        Handle<Value> &js_field,
                                        MYSQL_FIELD field,
                                        char* field_value) {
    js_field = Null();
    switch (field.type) {
        case MYSQL_TYPE_NULL:  // NULL-type field
            break;
        case MYSQL_TYPE_TINY:  // TINYINT field
        case MYSQL_TYPE_BIT:  // BIT field (MySQL 5.0.3 and up)
        case MYSQL_TYPE_SHORT:  // SMALLINT field
        case MYSQL_TYPE_LONG:  // INTEGER field
        case MYSQL_TYPE_INT24:  // MEDIUMINT field
        case MYSQL_TYPE_LONGLONG:  // BIGINT field
        case MYSQL_TYPE_YEAR:  // YEAR field
            if (field_value) {
              js_field = String::New(field_value)->ToInteger();
            }
            break;
        case MYSQL_TYPE_DECIMAL:  // DECIMAL or NUMERIC field
        case MYSQL_TYPE_NEWDECIMAL:  // Precision math DECIMAL or NUMERIC field
        case MYSQL_TYPE_FLOAT:  // FLOAT field
        case MYSQL_TYPE_DOUBLE:  // DOUBLE or REAL field
            // TODO(Sannis): Read about MySQL datatypes and javascript data
            if (field_value) {
              js_field = String::New(field_value)->ToNumber();
            }
            break;
        case MYSQL_TYPE_TIME:  // TIME field
            // TODO(Sannis): Read about MySQL datatypes and javascript data
            if (field_value) {
              js_field = String::New(field_value);
            }
            break;
        case MYSQL_TYPE_TIMESTAMP:  // TIMESTAMP field
        case MYSQL_TYPE_DATETIME:  // DATETIME field
            // TODO(Sannis): Read about MySQL datatypes and javascript data
            if (field_value) {
              js_field = String::New(field_value);
            }
            break;
        case MYSQL_TYPE_DATE:  // DATE field
        case MYSQL_TYPE_NEWDATE:  // Newer const used > 5.0
            // TODO(Sannis): Read about MySQL datatypes and javascript data
            if (field_value) {
              js_field = String::New(field_value);
            }
            break;
        case MYSQL_TYPE_TINY_BLOB:
        case MYSQL_TYPE_MEDIUM_BLOB:
        case MYSQL_TYPE_LONG_BLOB:
        case MYSQL_TYPE_BLOB:
        case MYSQL_TYPE_VAR_STRING:
        case MYSQL_TYPE_VARCHAR:
        case MYSQL_TYPE_SET:  // SET field
        case MYSQL_TYPE_ENUM:  // ENUM field
        case MYSQL_TYPE_GEOMETRY:  // Spatial fielda
        default:
            if (field_value) {
                js_field = String::New(field_value);
            }
    }
}

Handle<Value> MysqlConn::MysqlResult::New(const Arguments& args) {
    HandleScope scope;

    REQ_EXT_ARG(0, js_res);
    uint32_t field_count = args[1]->IntegerValue();
    MYSQL_RES *res = static_cast<MYSQL_RES*>(js_res->Value());
    MysqlResult *my_res = new MysqlResult(res, field_count);
    my_res->Wrap(args.This());

    return args.This();
}

Handle<Value> MysqlConn::MysqlResult::DataSeekSync(const Arguments& args) {
    HandleScope scope;

    MysqlResult *res = OBJUNWRAP<MysqlResult>(args.This());

    // TODO(Sannis): Is it possible?
    if (!res->_res) {
        return scope.Close(False());
    }

    if (args.Length() == 0 || !args[0]->IsNumber()) {
        return THREXC("First arg of res.fieldSeek() must be a number");
    }

    uint32_t row_num = args[0]->IntegerValue();

    if (mysql_result_is_unbuffered(res->_res)) {
        return THREXC("Function cannot be used with MYSQL_USE_RESULT");
    }

    if (row_num < 0 || row_num >= mysql_num_rows(res->_res)) {
        return THREXC("Invalid row offset");
    }

    mysql_data_seek(res->_res, row_num);

    return Undefined();
}

Handle<Value> MysqlConn::MysqlResult::FetchAllSync(const Arguments& args) {
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
            SetFieldValue(js_field, fields[j], result_row[j]);

            js_result_row->Set(String::New(fields[j].name), js_field);
        }

        js_result->Set(Integer::New(i), js_result_row);

        i++;
    }

    return scope.Close(js_result);
}

Handle<Value> MysqlConn::MysqlResult::FetchArraySync(const Arguments& args) {
    HandleScope scope;

    MysqlResult *res = OBJUNWRAP<MysqlResult>(args.This());

    // TODO(Sannis): Is it possible?
    if (!res->_res) {
        return scope.Close(False());
    }
    MYSQL_FIELD *fields = mysql_fetch_fields(res->_res);
    uint32_t num_fields = mysql_num_fields(res->_res);
    uint32_t j = 0;

    Local<Array> js_result_row;
    Local<Value> js_field;

    MYSQL_ROW result_row = mysql_fetch_row(res->_res);

    if (!result_row) {
        return scope.Close(False());
    }

    js_result_row = Array::New();

    for ( j = 0; j < num_fields; j++ ) {
        SetFieldValue(js_field, fields[j], result_row[j]);

        js_result_row->Set(Integer::New(j), js_field);
    }

    return scope.Close(js_result_row);
}

Handle<Value> MysqlConn::MysqlResult::FetchFieldSync(const Arguments& args) {
    HandleScope scope;

    MysqlResult *res = OBJUNWRAP<MysqlResult>(args.This());

    // TODO(Sannis): Is it possible?
    if (!res->_res) {
        return scope.Close(False());
    }

    MYSQL_FIELD *field;

    Local<Object> js_result;

    field = mysql_fetch_field(res->_res);

    if (!field) {
        return scope.Close(False());
    }

    js_result = Object::New();
    AddFieldProperties(js_result, field);

    return scope.Close(js_result);
}

Handle<Value> MysqlConn::MysqlResult::FetchFieldDirectSync(const Arguments& args) { // NOLINT
    HandleScope scope;

    MysqlResult *res = OBJUNWRAP<MysqlResult>(args.This());

    // TODO(Sannis): Is it possible?
    if (!res->_res) {
        return scope.Close(False());
    }

    if (args.Length() == 0 || !args[0]->IsNumber()) {
        return THREXC("First arg of res.fetchFieldDirect() must be a number");
    }

    uint32_t field_num = args[0]->IntegerValue();

    MYSQL_FIELD *field;

    Local<Object> js_result;

    field = mysql_fetch_field_direct(res->_res, field_num);

    if (!field) {
        return scope.Close(False());
    }

    js_result = Object::New();
    AddFieldProperties(js_result, field);

    return scope.Close(js_result);
}

Handle<Value> MysqlConn::MysqlResult::FetchFieldsSync(const Arguments& args) {
    HandleScope scope;

    MysqlResult *res = OBJUNWRAP<MysqlResult>(args.This());

    // TODO(Sannis): Is it possible?
    if (!res->_res) {
        return scope.Close(False());
    }

    uint32_t num_fields = mysql_num_fields(res->_res);
    MYSQL_FIELD *field;
    uint32_t i = 0;

    Local<Array> js_result = Array::New();
    Local<Object> js_result_obj;

    for (i = 0; i < num_fields; i++) {
        field = mysql_fetch_field_direct(res->_res, i);

        js_result_obj = Object::New();
        AddFieldProperties(js_result_obj, field);

        js_result->Set(Integer::New(i), js_result_obj);
    }

    return scope.Close(js_result);
}

Handle<Value> MysqlConn::MysqlResult::FetchLengthsSync(const Arguments& args) {
    HandleScope scope;

    MysqlResult *res = OBJUNWRAP<MysqlResult>(args.This());

    // TODO(Sannis): Is it possible?
    if (!res->_res) {
        return scope.Close(False());
    }

    uint32_t num_fields = mysql_num_fields(res->_res);
    unsigned long int *lengths;
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

Handle<Value> MysqlConn::MysqlResult::FetchObjectSync(const Arguments& args) {
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
        SetFieldValue(js_field, fields[j], result_row[j]);

        js_result_row->Set(String::New(fields[j].name), js_field);
    }

    return scope.Close(js_result_row);
}

Handle<Value> MysqlConn::MysqlResult::FieldCountSync(const Arguments& args) {
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

Handle<Value> MysqlConn::MysqlResult::FieldSeekSync(const Arguments& args) {
    HandleScope scope;

    MysqlResult *res = OBJUNWRAP<MysqlResult>(args.This());

    // TODO(Sannis): Is it possible?
    if (!res->_res) {
        return scope.Close(False());
    }

    if (args.Length() == 0 || !args[0]->IsNumber()) {
        return THREXC("First arg of res.fieldSeek() must be a number");
    }

    uint32_t field_num = args[0]->IntegerValue();

    if (field_num < 0 || field_num >= res->field_count) {
        return THREXC("Invalid field offset");
    }

    mysql_field_seek(res->_res, field_num);

    return Undefined();
}

Handle<Value> MysqlConn::MysqlResult::FieldTellSync(const Arguments& args) {
    HandleScope scope;

    MysqlResult *res = OBJUNWRAP<MysqlResult>(args.This());

    // TODO(Sannis): Is it possible?
    if (!res->_res) {
        return scope.Close(False());
    }

    Local<Value> js_result = Integer::New(mysql_field_tell(res->_res));

    return scope.Close(js_result);
}

void MysqlConn::MysqlResult::Free() {
    if (_res) {
        mysql_free_result(_res);
        _res = NULL;
    }
}

Handle<Value> MysqlConn::MysqlResult::FreeSync(const Arguments& args) {
    HandleScope scope;

    MysqlResult *res = OBJUNWRAP<MysqlResult>(args.This());

    // TODO(Sannis): Is it possible? Yes!
    if (!res->_res) {
        return scope.Close(False());
    }

    res->Free();

    return scope.Close(True());
}

Handle<Value> MysqlConn::MysqlResult::NumRowsSync(const Arguments& args) {
    HandleScope scope;

    MysqlResult *res = OBJUNWRAP<MysqlResult>(args.This());

    // TODO(Sannis): Is it possible?
    if (!res->_res) {
        return scope.Close(False());
    }

    if (mysql_result_is_unbuffered(res->_res)) {
        return THREXC("Function cannot be used with MYSQL_USE_RESULT");
    }

    Local<Value> js_result = Integer::New(mysql_num_rows(res->_res));

    return scope.Close(js_result);
}

