#include <node.h>

#define NAMESPACE_FOR_HASH_FUNCTIONS farmhash
#include "./deps/farmhash/src/farmhash.cc"

using namespace v8;

#define ENSURE_GTE_ONE_ARG(isolate) \
  if (args.Length() < 1) { \
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Invalid number of arguments"))); \
    return; \
  }

#define THROW_WRONG_ARG_TYPE(isolate) \
  isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "First argument must be String or Uint8Array"))); \
  return;

#define HASH_STRING(out, hashfn, v) \
  do { \
    String* str = *Local<String>::Cast(v); \
    int length = str->Utf8Length(); \
    char* buffer = new char[length]; \
    str->WriteUtf8(buffer, -1, NULL, String::NO_NULL_TERMINATION); \
    out = hashfn(buffer, (size_t) length); \
    delete buffer; \
  } while(false);

#define HASH_UINT8ARRAY(out, hashfn, v) \
  do { \
    ArrayBuffer::Contents contents = Local<Uint8Array>::Cast(v)->Buffer()->GetContents(); \
    out = hashfn((char*) contents.Data(), contents.ByteLength()); \
  } while(false);

#define TO_HEX(out, isolate, format, v) \
  do { \
    char* stringbuffer = new char[9]; \
    sprintf(stringbuffer, format, v); \
    out = String::NewFromUtf8(isolate, stringbuffer); \
    delete stringbuffer; \
  } while(false);


void fingerprint32(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ENSURE_GTE_ONE_ARG(isolate);
  if (args[0]->IsString()) {
    uint32_t hash;
    HASH_STRING(hash, farmhash::Fingerprint32, args[0]);
    args.GetReturnValue().Set(hash);
    return;
  }
  if (args[0]->IsUint8Array()) {
    uint32_t hash;
    HASH_UINT8ARRAY(hash, farmhash::Fingerprint32, args[0]);
    args.GetReturnValue().Set(hash);
    return;
  }
  THROW_WRONG_ARG_TYPE(isolate);
}

void fingerprint32asHex(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ENSURE_GTE_ONE_ARG(isolate);
  if (args[0]->IsString()) {
    uint32_t hash;
    HASH_STRING(hash, farmhash::Fingerprint32, args[0]);
    Local<String> str;
    TO_HEX(str, isolate, "%08x", hash);
    args.GetReturnValue().Set(str);
    return;
  }
  if (args[0]->IsUint8Array()) {
    uint32_t hash;
    HASH_UINT8ARRAY(hash, farmhash::Fingerprint32, args[0]);
    Local<String> str;
    TO_HEX(str, isolate, "%08x", hash);
    args.GetReturnValue().Set(str);
    return;
  }
  THROW_WRONG_ARG_TYPE(isolate);
}

void init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "fingerprint32", fingerprint32);
  NODE_SET_METHOD(exports, "fingerprint32asHex", fingerprint32asHex);
}

NODE_MODULE(farmhash, init)
