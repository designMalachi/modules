#include "sqlite.h"

void just::sqlite::Error(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Object> obj = args[0].As<Object>();
  sqlite3* db = (sqlite3*)obj->GetAlignedPointerFromInternalField(0);
  const char* errmsg = sqlite3_errmsg(db);
  args.GetReturnValue().Set(String::NewFromOneByte(isolate, (const uint8_t*)errmsg, 
    NewStringType::kNormal, strnlen(errmsg, 1024)).ToLocalChecked());
}

void just::sqlite::Version(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  const char* version = sqlite3_libversion();
  args.GetReturnValue().Set(String::NewFromOneByte(isolate, (const uint8_t*)version, 
    NewStringType::kNormal, strnlen(version, 1024)).ToLocalChecked());
}

void just::sqlite::Open(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  HandleScope handleScope(isolate);
  sqlite3* db;

  Local<ObjectTemplate> dbTemplate = ObjectTemplate::New(isolate);
  dbTemplate->SetInternalFieldCount(1);
  Local<Object> database = dbTemplate->NewInstance(context).ToLocalChecked();

  String::Utf8Value filename(isolate, args[0]);
  int status = sqlite3_open(*filename, &db);
  if (status != SQLITE_OK) {
    // todo: set error
    return;
  }
  database->SetAlignedPointerInInternalField(0, db);
  args.GetReturnValue().Set(database);
}

void just::sqlite::Prepare(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  HandleScope handleScope(isolate);

  Local<Object> obj = args[0].As<Object>();
  sqlite3* db = (sqlite3*)obj->GetAlignedPointerFromInternalField(0);

  sqlite3_stmt *res;
  String::Utf8Value sql(isolate, args[1]);
  int status = sqlite3_prepare_v2(db, *sql, -1, &res, 0);
  if (status != SQLITE_OK) {
    fprintf(stderr, "%i\n", status);
    // todo: set error
    return;
  }
  Local<ObjectTemplate> stmtTemplate = ObjectTemplate::New(isolate);
  stmtTemplate->SetInternalFieldCount(1);
  Local<Object> stmt = stmtTemplate->NewInstance(context).ToLocalChecked();
  stmt->SetAlignedPointerInInternalField(0, res);
  args.GetReturnValue().Set(stmt);
}

int callback(void *NotUsed, int argc, char **argv, char **azColName) {
  NotUsed = 0;
  for (int i = 0; i < argc; i++) {
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

void just::sqlite::Exec(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);

  Local<Object> obj = args[0].As<Object>();
  sqlite3* db = (sqlite3*)obj->GetAlignedPointerFromInternalField(0);

  char *err_msg = 0;
  String::Utf8Value sql(isolate, args[1]);
  int status = sqlite3_exec(db, *sql, callback, 0, &err_msg);
  //int status = sqlite3_exec(db, *sql, 0, 0, &err_msg);
  args.GetReturnValue().Set(Integer::New(isolate, status));
}

void just::sqlite::Step(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Object> obj = args[0].As<Object>();
  sqlite3_stmt* stmt = (sqlite3_stmt*)obj->GetAlignedPointerFromInternalField(0);
  int status = sqlite3_step(stmt);
  args.GetReturnValue().Set(Integer::New(isolate, status));
}

void just::sqlite::ParamIndex(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Object> obj = args[0].As<Object>();
  sqlite3_stmt* stmt = (sqlite3_stmt*)obj->GetAlignedPointerFromInternalField(0);
  String::Utf8Value fieldname(isolate, args[1]);
  int index = sqlite3_bind_parameter_index(stmt, *fieldname);
  args.GetReturnValue().Set(Integer::New(isolate, index));
}

void just::sqlite::BindInt(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Object> obj = args[0].As<Object>();
  sqlite3_stmt* stmt = (sqlite3_stmt*)obj->GetAlignedPointerFromInternalField(0);
  int index = Local<Integer>::Cast(args[1])->Value();
  int value = Local<Integer>::Cast(args[2])->Value();
  int status = sqlite3_bind_int(stmt, index, value);
  args.GetReturnValue().Set(Integer::New(isolate, status));
}

void just::sqlite::BindInt64(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Object> obj = args[0].As<Object>();
  sqlite3_stmt* stmt = (sqlite3_stmt*)obj->GetAlignedPointerFromInternalField(0);
  int index = Local<Integer>::Cast(args[1])->Value();
  Local<BigInt> value = Local<BigInt>::Cast(args[2]);
  int status = sqlite3_bind_int64(stmt, index, value->Uint64Value());
  args.GetReturnValue().Set(Integer::New(isolate, status));
}

void just::sqlite::BindBlob(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Object> obj = args[0].As<Object>();
  sqlite3_stmt* stmt = (sqlite3_stmt*)obj->GetAlignedPointerFromInternalField(0);
  int index = Local<Integer>::Cast(args[1])->Value();
  Local<ArrayBuffer> value = args[2].As<ArrayBuffer>();
  std::shared_ptr<BackingStore> backing = value->GetBackingStore();
  void* data = backing->Data();
  int size = backing->ByteLength();
  int status = sqlite3_bind_blob(stmt, index, data, size, SQLITE_STATIC);
  args.GetReturnValue().Set(Integer::New(isolate, status));
}

void just::sqlite::BindText(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Object> obj = args[0].As<Object>();
  sqlite3_stmt* stmt = (sqlite3_stmt*)obj->GetAlignedPointerFromInternalField(0);
  int index = Local<Integer>::Cast(args[1])->Value();
  String::Utf8Value text(isolate, args[2]);
  int status = sqlite3_bind_text(stmt, index, *text, text.length(), SQLITE_TRANSIENT);
  args.GetReturnValue().Set(Integer::New(isolate, status));
}

void just::sqlite::ColumnText(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Object> obj = args[0].As<Object>();
  sqlite3_stmt* stmt = (sqlite3_stmt*)obj->GetAlignedPointerFromInternalField(0);
  int column = Local<Integer>::Cast(args[1])->Value();
  const unsigned char* text = sqlite3_column_text(stmt, column);
  if (text == NULL) {
    return;
  }
  args.GetReturnValue().Set(String::NewFromUtf8(isolate, 
    (const char*)text, v8::NewStringType::kNormal).ToLocalChecked());
}

void just::sqlite::ColumnName(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Object> obj = args[0].As<Object>();
  sqlite3_stmt* stmt = (sqlite3_stmt*)obj->GetAlignedPointerFromInternalField(0);
  int column = Local<Integer>::Cast(args[1])->Value();
  const char* text = sqlite3_column_name(stmt, column);
  if (text == NULL) {
    return;
  }
  args.GetReturnValue().Set(String::NewFromUtf8(isolate, 
    (const char*)text, v8::NewStringType::kNormal).ToLocalChecked());
}

void just::sqlite::ColumnBytes(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Object> obj = args[0].As<Object>();
  sqlite3_stmt* stmt = (sqlite3_stmt*)obj->GetAlignedPointerFromInternalField(0);
  int column = Local<Integer>::Cast(args[1])->Value();
  int bytes = sqlite3_column_bytes(stmt, column);
  args.GetReturnValue().Set(Integer::New(isolate, bytes));
}

void just::sqlite::ColumnType(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Object> obj = args[0].As<Object>();
  sqlite3_stmt* stmt = (sqlite3_stmt*)obj->GetAlignedPointerFromInternalField(0);
  int column = Local<Integer>::Cast(args[1])->Value();
  int type = sqlite3_column_type(stmt, column);
  args.GetReturnValue().Set(Integer::New(isolate, type));
}

void just::sqlite::ColumnCount(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Object> obj = args[0].As<Object>();
  sqlite3_stmt* stmt = (sqlite3_stmt*)obj->GetAlignedPointerFromInternalField(0);
  int count = sqlite3_column_count(stmt);
  args.GetReturnValue().Set(Integer::New(isolate, count));
}

void just::sqlite::ColumnBlob(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Object> obj = args[0].As<Object>();
  sqlite3_stmt* stmt = (sqlite3_stmt*)obj->GetAlignedPointerFromInternalField(0);
  int column = Local<Integer>::Cast(args[1])->Value();
  int len = sqlite3_column_bytes(stmt, column);
  const void* rodata = sqlite3_column_blob(stmt, column);
  void* data = calloc(1, len);
  memcpy(data, rodata, len);
  std::unique_ptr<BackingStore> backing = ArrayBuffer::NewBackingStore(data, len, 
      [](void*, size_t, void*){}, nullptr);
  Local<ArrayBuffer> ab =
      ArrayBuffer::New(args.GetIsolate(), std::move(backing));
  args.GetReturnValue().Set(ab);
}

void just::sqlite::Finalize(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Object> obj = args[0].As<Object>();
  sqlite3_stmt* stmt = (sqlite3_stmt*)obj->GetAlignedPointerFromInternalField(0);
  int status = sqlite3_finalize(stmt);
  args.GetReturnValue().Set(Integer::New(isolate, status));
}

void just::sqlite::Reset(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Object> obj = args[0].As<Object>();
  sqlite3_stmt* stmt = (sqlite3_stmt*)obj->GetAlignedPointerFromInternalField(0);
  int status = sqlite3_reset(stmt);
  args.GetReturnValue().Set(Integer::New(isolate, status));
}

void just::sqlite::Close(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Object> obj = args[0].As<Object>();
  sqlite3* db = (sqlite3*)obj->GetAlignedPointerFromInternalField(0);
  sqlite3_close(db);
}

void just::sqlite::LastId(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Object> obj = args[0].As<Object>();
  sqlite3* db = (sqlite3*)obj->GetAlignedPointerFromInternalField(0);
  int id = sqlite3_last_insert_rowid(db);
  args.GetReturnValue().Set(Integer::New(isolate, id));
}

void just::sqlite::AutoCommit(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Object> obj = args[0].As<Object>();
  sqlite3* db = (sqlite3*)obj->GetAlignedPointerFromInternalField(0);
  int ac = sqlite3_get_autocommit(db);
  args.GetReturnValue().Set(Integer::New(isolate, ac));
}

void just::sqlite::Init(Isolate* isolate, Local<ObjectTemplate> target) {
  Local<ObjectTemplate> module = ObjectTemplate::New(isolate);

  SET_METHOD(isolate, module, "version", Version);
  SET_METHOD(isolate, module, "open", Open);
  SET_METHOD(isolate, module, "prepare", Prepare);
  SET_METHOD(isolate, module, "step", Step);
  SET_METHOD(isolate, module, "error", Error);
  SET_METHOD(isolate, module, "finalize", Finalize);
  SET_METHOD(isolate, module, "reset", Reset);
  SET_METHOD(isolate, module, "close", Close);
  SET_METHOD(isolate, module, "columnText", ColumnText);
  SET_METHOD(isolate, module, "columnName", ColumnName);
  SET_METHOD(isolate, module, "columnType", ColumnType);
  SET_METHOD(isolate, module, "columnBlob", ColumnBlob);
  SET_METHOD(isolate, module, "columnBytes", ColumnBytes);
  SET_METHOD(isolate, module, "columnCount", ColumnCount);
  SET_METHOD(isolate, module, "lastId", LastId);
  SET_METHOD(isolate, module, "exec", Exec);
  SET_METHOD(isolate, module, "bindInt", BindInt);
  SET_METHOD(isolate, module, "bindInt64", BindInt64);
  SET_METHOD(isolate, module, "bindBlob", BindBlob);
  SET_METHOD(isolate, module, "bindText", BindText);
  SET_METHOD(isolate, module, "paramIndex", ParamIndex);
  SET_METHOD(isolate, module, "autoCommit", AutoCommit);

  SET_MODULE(isolate, target, "sqlite", module);
}
