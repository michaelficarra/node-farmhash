#include <node.h>
#include <node_buffer.h>
#include <functional>

#define NAMESPACE_FOR_HASH_FUNCTIONS farmhash

#include "./deps/farmhash/src/farmhash.cc"

namespace node_farmhash {

template<int Bits>
class UintTrait {
};

template<>
struct UintTrait<32> {
  typedef uint32_t Type;
};

template<>
struct UintTrait<64> {
  typedef uint64_t Type;
};

template<>
struct UintTrait<128> {
  typedef uint128_t Type;
};

template<int Bits>
static inline typename UintTrait<Bits>::Type genFingerPrint(const char* str, size_t length);

template<>
inline uint32_t genFingerPrint<32>(const char* str, size_t length) {
  return farmhash::Fingerprint32(str, length);
}

template<>
inline uint64_t genFingerPrint<64>(const char* str, size_t length) {
  return farmhash::Fingerprint64(str, length);
}

template<>
inline uint128_t genFingerPrint<128>(const char* str, size_t length) {
  return farmhash::Fingerprint128(str, length);
};

template<int BITS>
static inline typename UintTrait<BITS>::Type genFingerPrint(v8::String* str) {
  size_t length = (size_t) str->Utf8Length();
  unique_ptr<char[]> buffer(new char[length]);
  str->WriteUtf8(buffer.get(), -1, NULL, v8::String::NO_NULL_TERMINATION);
  return genFingerPrint<BITS>(buffer.get(), length);
};


template<int BITS>
static inline typename UintTrait<BITS>::Type genFingerPrintFromArrayBuffer(v8::ArrayBuffer::Contents contents) {
  return genFingerPrint<BITS>((char*) contents.Data(), contents.ByteLength());
};

template<int BITS>
static inline typename UintTrait<BITS>::Type genFingerPrintFromNodeBuffer(v8::Local<v8::Value> buffer) {
  return genFingerPrint<BITS>(node::Buffer::Data(buffer), node::Buffer::Length(buffer));
};

template<int BITS>
static inline void toNumber(typename UintTrait<BITS>::Type bits, v8::ReturnValue<v8::Value> returnValue) {
  return returnValue.Set(v8::Number::New(returnValue.GetIsolate(), (double) bits));
}

template<int BITS>
static inline void toHex(typename UintTrait<BITS>::Type bits, v8::ReturnValue<v8::Value> returnValue) {
  const uint32_t* raw = reinterpret_cast<const uint32_t*>(&bits);
  const int SIZE = BITS / 32;
  char stringbuffer[BITS / 4 + 1] = {0};
  for (int i = 0; i < SIZE; i++) {
    snprintf(stringbuffer + i * 8, 9, "%08x", raw[SIZE - 1 - i]);
  }
  return returnValue.Set(v8::String::NewFromUtf8(returnValue.GetIsolate(), stringbuffer));
}

template<int BITS>
static inline void toArrayBuffer(typename UintTrait<BITS>::Type bits, v8::ReturnValue<v8::Value> returnValue) {
  size_t bytes = (size_t) (BITS / 8);
  auto arrayBuffer = v8::ArrayBuffer::New(returnValue.GetIsolate(), bytes);
  memcpy(arrayBuffer->GetContents().Data(), &bits, bytes);
  return returnValue.Set(arrayBuffer);
}

template<int BITS, void formatter(typename UintTrait<BITS>::Type bits, v8::ReturnValue<v8::Value> returnValue)>
static void genFP(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  if (args.Length() < 1) {
    isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Invalid number of arguments")));
    return;
  }
  if (args[0]->IsString()) {
    v8::String* str = *v8::Local<v8::String>::Cast(args[0]);
    formatter(genFingerPrint<BITS>(str), args.GetReturnValue());
  } else if (args[0]->IsArrayBuffer()) {
    v8::ArrayBuffer::Contents contents = v8::Local<v8::ArrayBuffer>::Cast(args[0])->GetContents();
    formatter(genFingerPrintFromArrayBuffer<BITS>(contents), args.GetReturnValue());
  } else if (args[0]->IsArrayBufferView()) {
    v8::ArrayBuffer::Contents contents = v8::Local<v8::ArrayBufferView>::Cast(args[0])->Buffer()->GetContents();
    formatter(genFingerPrintFromArrayBuffer<BITS>(contents), args.GetReturnValue());
  } else if (node::Buffer::HasInstance(args[0])) {
    formatter(genFingerPrintFromNodeBuffer<BITS>(args[0]), args.GetReturnValue());
  } else {
    isolate->ThrowException(
        v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "First argument must be ArrayBuffer, ArrayBufferView, Buffer, or String")));
  }
}

}  // namespace node_farmhash

void init(v8::Local<v8::Object> exports) {
  using namespace node_farmhash;

  NODE_SET_METHOD(exports, "fingerprint32", genFP<32, toNumber<32>>);
  NODE_SET_METHOD(exports, "fingerprint32asHex", genFP<32, toHex<32>>);
  NODE_SET_METHOD(exports, "fingerprint32asArrayBuffer", genFP<32, toArrayBuffer<32>>);
  NODE_SET_METHOD(exports, "fingerprint64asHex", genFP<64, toHex<64>>);
  NODE_SET_METHOD(exports, "fingerprint64asArrayBuffer", genFP<64, toArrayBuffer<64>>);
  NODE_SET_METHOD(exports, "fingerprint128asHex", genFP<128, toHex<128>>);
  NODE_SET_METHOD(exports, "fingerprint128asArrayBuffer", genFP<128, toArrayBuffer<128>>);
}

NODE_MODULE(farmhash, init)
