#ifndef ES_TYPES_BUILTIN_STRING_OBJECT
#define ES_TYPES_BUILTIN_STRING_OBJECT

#include <math.h>

#include <es/types/object.h>
#include <es/parser/character.h>

namespace es {

std::u16string ToString(Error* e, JSValue* input);
double ToInteger(Error* e, JSValue* input);
double ToUint16(Error* e, JSValue* input);
std::u16string NumberToString(double m);

class StringProto : public JSObject {
 public:
  static  StringProto* Instance() {
    static  StringProto singleton;
    return &singleton;
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSValue* val = Runtime::TopValue();
    if (!val->IsObject()) {
      *e = *Error::TypeError(u"String.prototype.toString called with non-object");
      return nullptr;
    }
    JSObject* obj = static_cast<JSObject*>(val);
    if (obj->obj_type() != JSObject::OBJ_STRING) {
      *e = *Error::TypeError(u"String.prototype.toString called with non-string");
      return nullptr;
    }
    return obj->PrimitiveValue();
  }

  static JSValue* valueOf(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSValue* val = Runtime::TopValue();
    if (!val->IsObject()) {
      *e = *Error::TypeError(u"String.prototype.valueOf called with non-object");
      return nullptr;
    }
    JSObject* obj = static_cast<JSObject*>(val);
    if (obj->obj_type() != JSObject::OBJ_STRING) {
      *e = *Error::TypeError(u"String.prototype.valueOf called with non-string");
      return nullptr;
    }
    return obj->PrimitiveValue();
  }

  static JSValue* charAt(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    if (vals.size() == 0)
      return String::Empty();
    JSValue* val = Runtime::TopValue();
    val->CheckObjectCoercible(e);
    if (!e->IsOk()) return nullptr;
    std::u16string S = ::es::ToString(e, val);
    if (!e->IsOk()) return nullptr;
    int position = ToInteger(e, vals[0]);
    if (!e->IsOk()) return nullptr;
    if (position < 0 || position >= S.size())
      return String::Empty();
    return new String(S.substr(position, 1));
  }

  static JSValue* charCodeAt(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    if (vals.size() == 0)
      return Number::NaN();
    JSValue* val = Runtime::TopValue();
    val->CheckObjectCoercible(e);
    if (!e->IsOk()) return nullptr;
    std::u16string S = ::es::ToString(e, val);
    if (!e->IsOk()) return nullptr;
    int position = ToInteger(e, vals[0]);
    if (!e->IsOk()) return nullptr;
    if (position < 0 || position >= S.size())
      return Number::NaN();
    return new Number((double)S[position]);
  }

  static JSValue* concat(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSValue* val = Runtime::TopValue();
    val->CheckObjectCoercible(e);
    if (!e->IsOk()) return nullptr;
    std::u16string S = ::es::ToString(e, val);
    if (!e->IsOk()) return nullptr;
    std::u16string R = S;
    std::vector<JSValue*> args = vals;
    for (auto arg : args) {
      std::u16string next = ::es::ToString(e, arg);
      if (!e->IsOk()) return nullptr;
      R += next;
    }
    return new String(R);
  }

  static JSValue* indexOf(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSValue* val = Runtime::TopValue();
    val->CheckObjectCoercible(e);
    if (!e->IsOk()) return nullptr;
    std::u16string S = ::es::ToString(e, val);
    if (!e->IsOk()) return nullptr;
    JSValue* search_string;
    if (vals.size() == 0)
      search_string = Undefined::Instance();
    else
      search_string = vals[0];
    std::u16string search_str = ::es::ToString(e, search_string);
    if (!e->IsOk()) return nullptr;
    double pos;
    if (vals.size() < 2 || vals[1]->IsUndefined())
      pos = 0;
    else {
      pos = ToInteger(e, vals[1]);
      if (!e->IsOk()) return nullptr;
    }
    int start = fmin(fmax(pos, 0), S.size());
    size_t find_pos = S.find(search_str, start);
    if (find_pos != std::u16string::npos) {
      return new Number(find_pos);
    }
    return new Number(-1);
  }

  static JSValue* lastIndexOf(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSValue* val = Runtime::TopValue();
    val->CheckObjectCoercible(e);
    if (!e->IsOk()) return nullptr;
    std::u16string S = ::es::ToString(e, val);
    if (!e->IsOk()) return nullptr;
    JSValue* search_string;
    if (vals.size() == 0)
      search_string = Undefined::Instance();
    else
      search_string = vals[0];
    std::u16string search_str = ::es::ToString(e, search_string);
    if (!e->IsOk()) return nullptr;
    double pos;
    if (vals.size() < 2 || vals[1]->IsUndefined())
      pos = nan("");
    else {
      pos = ToNumber(e, vals[1]);
      if (!e->IsOk()) return nullptr;
    }
    int start;
    if (isnan(pos))
      start = S.size();
    else
      start = fmin(fmax(pos, 0), S.size());
    size_t find_pos = S.rfind(search_str, start);
    if (find_pos != std::u16string::npos) {
      return new Number(find_pos);
    }
    return new Number(-1);
  }

  static JSValue* localeCompare(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* match(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* replace(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* search(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* slice(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* split(Error* e, JSValue* this_arg, std::vector<JSValue*> vals);

  static JSValue* substring(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    if (vals.size() == 0)
      return Number::NaN();
    JSValue* val = Runtime::TopValue();
    val->CheckObjectCoercible(e);
    if (!e->IsOk()) return nullptr;
    std::u16string S = ::es::ToString(e, val);
    int len = S.size();
    if (!e->IsOk()) return nullptr;
    int int_start = ToInteger(e, vals[0]);
    if (!e->IsOk()) return nullptr;
    int int_end;
    if (vals.size() < 2 || vals[0]->IsUndefined()) {
      int_end = S.size();
    } else {
      int_end = ToInteger(e, vals[1]);
      if (!e->IsOk()) return nullptr;
    }
    int final_start = fmin(fmax(int_start, 0), len);
    int final_end = fmin(fmax(int_end, 0), len);
    int from = fmin(final_start, final_end);
    int to = fmax(final_start, final_end);
    return new String(S.substr(from, to - from));
  }

  static JSValue* toLowerCase(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSValue* val = Runtime::TopValue();
    val->CheckObjectCoercible(e);
    if (!e->IsOk()) return nullptr;
    std::u16string S = ::es::ToString(e, val);
    if (!e->IsOk()) return nullptr;
    std::u16string L = S;
    std::transform(L.begin(), L.end(), L.begin(), character::ToLowerCase);
    return new String(L);
  }

  static JSValue* toLocaleLowerCase(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    // TODO(zhuzilin) may need to fix this.
    return toLowerCase(e, this_arg, vals);
  }

  static JSValue* toUpperCase(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSValue* val = Runtime::TopValue();
    val->CheckObjectCoercible(e);
    if (!e->IsOk()) return nullptr;
    std::u16string S = ::es::ToString(e, val);
    if (!e->IsOk()) return nullptr;
    std::u16string U = S;
    std::transform(U.begin(), U.end(), U.begin(), character::ToUpperCase);
    return new String(U);
  }

  static JSValue* toLocaleUpperCase(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    // TODO(zhuzilin) may need to fix this.
    return toUpperCase(e, this_arg, vals);
  }

  static JSValue* trim(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

 private:
   StringProto() :
    JSObject(
      OBJ_OTHER, u"String", true, String::Empty(), false, false
    ) {}
};

class StringObject : public JSObject {
 public:
  StringObject(JSValue* primitive_value) :
    JSObject(
      OBJ_STRING,
      u"String",
      true,  // extensible
      primitive_value,
      false,
      false
    ) {
    SetPrototype(StringProto::Instance());
    assert(primitive_value->IsString());
    double length = static_cast<String*>(primitive_value)->data().size();
    AddValueProperty(u"length", new Number(length), false, false, false);
  }

  JSValue* GetOwnProperty(std::u16string P) override {
    JSValue* val = JSObject::GetOwnProperty(P);
    if (!val->IsUndefined())
      return val;
    Error* e = Error::Ok();
    int index = ToInteger(e, new String(P));  // this will never has error.
    if (NumberToString(fabs(index)) != P)
      return Undefined::Instance();
    std::u16string str = static_cast<String*>(PrimitiveValue())->data();
    int len = str.size();
    if (len <= index)
      return Undefined::Instance();
    PropertyDescriptor* desc = new PropertyDescriptor();
    desc->SetDataDescriptor(new String(str.substr(index, 1)), true, false, false);
    return desc;
  }

};

class StringConstructor : public JSObject {
 public:
  static  StringConstructor* Instance() {
    static  StringConstructor singleton;
    return &singleton;
  }

  // 15.5.1.1 String ( [ value ] )
  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) override {
    if (arguments.size() == 0)
      return String::Empty();
    return new String(::es::ToString(e, arguments[0]));
  }

  // 15.5.2.1 new String ( [ value ] )
  JSObject* Construct(Error* e, std::vector<JSValue*> arguments) override {
    if (arguments.size() == 0)
      return new StringObject(String::Empty());
    std::u16string str = ::es::ToString(e, arguments[0]);
    if (!e->IsOk()) return nullptr;
    return new StringObject(new String(str));
  }

  static JSValue* fromCharCode(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    std::u16string result = u"";
    for (JSValue* val : vals) {
      char16_t c = ToUint16(e, val);
      if (!e->IsOk()) return nullptr;
      result += c;
    }
    return new String(result);
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    return new String(u"function String() { [native code] }");
  }

 private:
   StringConstructor() :
    JSObject(OBJ_OTHER, u"String", true, nullptr, true, true) {}
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_STRING_OBJECT