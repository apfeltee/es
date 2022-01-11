#ifndef ES_TYPES_BUILTIN_OBJECT_CONSTRUCTOR
#define ES_TYPES_BUILTIN_OBJECT_CONSTRUCTOR

#include <es/types/object.h>
#include <es/types/builtin/object_proto.h>
#include <es/types/builtin/function_proto.h>

#include <es/error.h>

namespace es {

class ObjectConstructor : public JSObject {
  ObjectConstructor() :
    JSObject(
      ObjType obj_type,
      "Object",
      bool extensible,
      JSValue* primitive_value,
      true,
      true,
    )

  JSValue* GetPrototypeOf(Error* e, JSValue* o) {

  }
}

}

#endif  // ES_TYPES_BUILTIN_OBJECT_CONSTRUCTOR