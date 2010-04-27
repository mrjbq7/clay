#include "clay.hpp"



//
// analyze wrappers
//

bool analysisToPValue(ObjectPtr x, PValuePtr &pv)
{
    switch (x->objKind) {

    case VALUE_HOLDER : {
        ValueHolder *y = (ValueHolder *)x.ptr();
        pv = new PValue(y->type, true);
        return true;
    }

    case PVALUE :
        pv = (PValue *)x.ptr();
        return true;

    case TYPE :
    case VOID_TYPE :
    case PRIM_OP :
    case PROCEDURE :
    case OVERLOADABLE :
    case RECORD :
    case STATIC_PROCEDURE :
    case STATIC_OVERLOADABLE :
    case MODULE_HOLDER :
        pv = new PValue(staticType(x), true);
        return true;

    default :
        return false;
    }
}

ObjectPtr analyzeMaybeVoidValue(ExprPtr expr, EnvPtr env)
{
    ObjectPtr v = analyze(expr, env);
    if (!v)
        return NULL;
    switch (v->objKind) {
    case VOID_VALUE :
        return voidValue.ptr();
    default : {
        PValuePtr pv;
        if (!analysisToPValue(v, pv))
            error(expr, "expecting a value or void");
        return pv.ptr();
    }
    }
}

PValuePtr analyzeValue(ExprPtr expr, EnvPtr env)
{
    ObjectPtr v = analyze(expr, env);
    if (!v)
        return NULL;
    PValuePtr pv;
    if (!analysisToPValue(v, pv))
        error(expr, "expecting a value");
    return pv;
}

PValuePtr analyzePointerValue(ExprPtr expr, EnvPtr env)
{
    PValuePtr pv = analyzeValue(expr, env);
    if (!pv)
        return NULL;
    if (pv->type->typeKind != POINTER_TYPE)
        error(expr, "expecting a pointer value");
    return pv;
}

PValuePtr analyzeArrayValue(ExprPtr expr, EnvPtr env)
{
    PValuePtr pv = analyzeValue(expr, env);
    if (!pv)
        return NULL;
    if (pv->type->typeKind != ARRAY_TYPE)
        error(expr, "expecting an array value");
    return pv;
}

PValuePtr analyzeTupleValue(ExprPtr expr, EnvPtr env)
{
    PValuePtr pv = analyzeValue(expr, env);
    if (!pv)
        return NULL;
    if (pv->type->typeKind != TUPLE_TYPE)
        error(expr, "expecting a tuple value");
    return pv;
}

PValuePtr analyzeRecordValue(ExprPtr expr, EnvPtr env)
{
    PValuePtr pv = analyzeValue(expr, env);
    if (!pv)
        return NULL;
    if (pv->type->typeKind != RECORD_TYPE)
        error(expr, "expecting a record value");
    return pv;
}



//
// analyze
//

ObjectPtr analyze(ExprPtr expr, EnvPtr env)
{
    LocationContext loc(expr->location);

    switch (expr->exprKind) {

    case BOOL_LITERAL : {
        BoolLiteral *x = (BoolLiteral *)expr.ptr();
        return boolToValueHolder(x->value).ptr();
    }

    case INT_LITERAL : {
        IntLiteral *x = (IntLiteral *)expr.ptr();
        char *ptr = (char *)x->value.c_str();
        char *end = ptr;
        ValueHolderPtr vh;
        if (x->suffix == "i8") {
            long y = strtol(ptr, &end, 0);
            if (*end != 0)
                error("invalid int8 literal");
            if ((errno == ERANGE) || (y < SCHAR_MIN) || (y > SCHAR_MAX))
                error("int8 literal out of range");
            vh = new ValueHolder(int8Type);
            *((char *)vh->buf) = (char)y;
        }
        else if (x->suffix == "i16") {
            long y = strtol(ptr, &end, 0);
            if (*end != 0)
                error("invalid int16 literal");
            if ((errno == ERANGE) || (y < SHRT_MIN) || (y > SHRT_MAX))
                error("int16 literal out of range");
            vh = new ValueHolder(int16Type);
            *((short *)vh->buf) = (short)y;
        }
        else if ((x->suffix == "i32") || x->suffix.empty()) {
            long y = strtol(ptr, &end, 0);
            if (*end != 0)
                error("invalid int32 literal");
            if (errno == ERANGE)
                error("int32 literal out of range");
            vh = new ValueHolder(int32Type);
            *((int *)vh->buf) = (int)y;
        }
        else if (x->suffix == "i64") {
            long long y = strtoll(ptr, &end, 0);
            if (*end != 0)
                error("invalid int64 literal");
            if (errno == ERANGE)
                error("int64 literal out of range");
            vh = new ValueHolder(int64Type);
            *((long long *)vh->buf) = y;
        }
        else if (x->suffix == "u8") {
            unsigned long y = strtoul(ptr, &end, 0);
            if (*end != 0)
                error("invalid uint8 literal");
            if ((errno == ERANGE) || (y > UCHAR_MAX))
                error("uint8 literal out of range");
            vh = new ValueHolder(uint8Type);
            *((unsigned char *)vh->buf) = (unsigned char)y;
        }
        else if (x->suffix == "u16") {
            unsigned long y = strtoul(ptr, &end, 0);
            if (*end != 0)
                error("invalid uint16 literal");
            if ((errno == ERANGE) || (y > USHRT_MAX))
                error("uint16 literal out of range");
            vh = new ValueHolder(uint16Type);
            *((unsigned short *)vh->buf) = (unsigned short)y;
        }
        else if (x->suffix == "u32") {
            unsigned long y = strtoul(ptr, &end, 0);
            if (*end != 0)
                error("invalid uint32 literal");
            if (errno == ERANGE)
                error("uint32 literal out of range");
            vh = new ValueHolder(uint32Type);
            *((unsigned int *)vh->buf) = (unsigned int)y;
        }
        else if (x->suffix == "u64") {
            unsigned long long y = strtoull(ptr, &end, 0);
            if (*end != 0)
                error("invalid uint64 literal");
            if (errno == ERANGE)
                error("uint64 literal out of range");
            vh = new ValueHolder(uint64Type);
            *((unsigned long long *)vh->buf) = y;
        }
        else if (x->suffix == "f32") {
            float y = (float)strtod(ptr, &end);
            if (*end != 0)
                error("invalid float32 literal");
            if (errno == ERANGE)
                error("float32 literal out of range");
            vh = new ValueHolder(float32Type);
            *((float *)vh->buf) = y;
        }
        else if (x->suffix == "f64") {
            double y = strtod(ptr, &end);
            if (*end != 0)
                error("invalid float64 literal");
            if (errno == ERANGE)
                error("float64 literal out of range");
            vh = new ValueHolder(float64Type);
            *((double *)vh->buf) = y;
        }
        else {
            error("invalid literal suffix: " + x->suffix);
        }
        return vh.ptr();
    }

    case FLOAT_LITERAL : {
        FloatLiteral *x = (FloatLiteral *)expr.ptr();
        char *ptr = (char *)x->value.c_str();
        char *end = ptr;
        ValueHolderPtr vh;
        if (x->suffix == "f32") {
            float y = (float)strtod(ptr, &end);
            if (*end != 0)
                error("invalid float32 literal");
            if (errno == ERANGE)
                error("float32 literal out of range");
            vh = new ValueHolder(float32Type);
            *((float *)vh->buf) = y;
        }
        else if ((x->suffix == "f64") || x->suffix.empty()) {
            double y = strtod(ptr, &end);
            if (*end != 0)
                error("invalid float64 literal");
            if (errno == ERANGE)
                error("float64 literal out of range");
            vh = new ValueHolder(float64Type);
            *((double *)vh->buf) = y;
        }
        else {
            error("invalid float literal suffix: " + x->suffix);
        }
        return vh.ptr();
    }

    case CHAR_LITERAL : {
        CharLiteral *x = (CharLiteral *)expr.ptr();
        if (!x->desugared)
            x->desugared = desugarCharLiteral(x->value);
        return analyze(x->desugared, env);
    }

    case STRING_LITERAL : {
        StringLiteral *x = (StringLiteral *)expr.ptr();
        PValuePtr pv = new PValue(arrayType(int8Type, x->value.size()), true);
        vector<ExprPtr> args;
        args.push_back(new ObjectExpr(pv.ptr()));
        return analyzeInvoke(kernelName("stringRef"), args, env);
    }

    case NAME_REF : {
        NameRef *x = (NameRef *)expr.ptr();
        ObjectPtr y = lookupEnv(env, x->name);
        if (y->objKind == EXPRESSION)
            return analyze((Expr *)y.ptr(), env);
        return analyzeStaticObject(y);
    }

    case RETURNED : {
        ObjectPtr y = lookupEnv(env, new Identifier("%returned"));
        if (!y)
            error("'returned' requires explicit return type declaration");
        ReturnedInfo *z = (ReturnedInfo *)y.ptr();
        if (!z->returnType)
            error("'returned' cannot be used when returning void");
        if (!z->returnIsTemp)
            error("'returned' cannot be used when returning by reference");
        return new PValue(z->returnType, false);
    }

    case TUPLE : {
        Tuple *x = (Tuple *)expr.ptr();
        vector<ExprPtr> args2;
        bool expanded = expandVarArgs(x->args, env, args2);
        if (!expanded && (args2.size() == 1))
            return analyze(args2[0], env);
        return analyzeInvoke(primName("tuple"), args2, env);
    }

    case ARRAY : {
        Array *x = (Array *)expr.ptr();
        vector<ExprPtr> args2;
        expandVarArgs(x->args, env, args2);
        return analyzeInvoke(primName("array"), args2, env);
    }

    case INDEXING : {
        Indexing *x = (Indexing *)expr.ptr();
        ObjectPtr indexable = analyze(x->expr, env);
        if (!indexable)
            return NULL;
        vector<ExprPtr> args2;
        expandVarArgs(x->args, env, args2);
        return analyzeIndexing(indexable, args2, env);
    }

    case CALL : {
        Call *x = (Call *)expr.ptr();
        ObjectPtr callable = analyze(x->expr, env);
        if (!callable)
            return NULL;
        vector<ExprPtr> args2;
        expandVarArgs(x->args, env, args2);
        return analyzeInvoke(callable, args2, env);
    }

    case FIELD_REF : {
        FieldRef *x = (FieldRef *)expr.ptr();
        ObjectPtr base = analyze(x->expr, env);
        if (!base)
            return NULL;
        return analyzeFieldRef(base, x->name);
    }

    case TUPLE_REF : {
        TupleRef *x = (TupleRef *)expr.ptr();
        vector<ExprPtr> args;
        args.push_back(x->expr);
        args.push_back(new ObjectExpr(sizeTToValueHolder(x->index).ptr()));
        ObjectPtr prim = primName("tupleRef");
        return analyzeInvoke(prim, args, env);
    }

    case UNARY_OP : {
        UnaryOp *x = (UnaryOp *)expr.ptr();
        if (!x->desugared)
            x->desugared = desugarUnaryOp(x);
        return analyze(x->desugared, env);
    }

    case BINARY_OP : {
        BinaryOp *x = (BinaryOp *)expr.ptr();
        if (!x->desugared)
            x->desugared = desugarBinaryOp(x);
        return analyze(x->desugared, env);
    }

    case AND : {
        And *x = (And *)expr.ptr();
        ObjectPtr first = analyze(x->expr1, env);
        if (!first)
            return NULL;
        if (first->objKind == VALUE_HOLDER) {
            ValueHolder *y = (ValueHolder *)first.ptr();
            if (y->type == boolType) {
                if (*((char *)y->buf) == 0)
                    return first;
            }
        }
        PValuePtr a;
        if (!analysisToPValue(first, a))
            error(x->expr1, "expecting a value");
        PValuePtr b = analyzeValue(x->expr2, env);
        if (!a || !b)
            return NULL;
        if (a->type != b->type)
            error("type mismatch in 'and' expression");
        return new PValue(a->type, a->isTemp || b->isTemp);
    }

    case OR : {
        Or *x = (Or *)expr.ptr();
        ObjectPtr first = analyze(x->expr1, env);
        if (!first)
            return NULL;
        if (first->objKind == VALUE_HOLDER) {
            ValueHolder *y = (ValueHolder *)first.ptr();
            if (y->type == boolType) {
                if (*((char *)y->buf) != 0)
                    return first;
            }
        }
        PValuePtr a;
        if (!analysisToPValue(first, a))
            error(x->expr1, "expecting a value");
        PValuePtr b = analyzeValue(x->expr2, env);
        if (!a || !b)
            return NULL;
        if (a->type != b->type)
            error("type mismatch in 'or' expression");
        return new PValue(a->type, a->isTemp || b->isTemp);
    }

    case LAMBDA : {
        Lambda *x = (Lambda *)expr.ptr();
        if (!x->initialized)
            initializeLambda(x, env);
        return analyze(x->converted, env);
    }

    case VAR_ARGS_REF :
        error("invalid use of '...'");

    case NEW : {
        New *x = (New *)expr.ptr();
        if (!x->desugared)
            x->desugared = desugarNew(x);
        return analyze(x->desugared, env);
    }

    case SC_EXPR : {
        SCExpr *x = (SCExpr *)expr.ptr();
        return analyze(x->expr, x->env);
    }

    case OBJECT_EXPR : {
        ObjectExpr *x = (ObjectExpr *)expr.ptr();
        return analyzeStaticObject(x->obj);
    }

    default :
        assert(false);
        return NULL;

    }
}



//
// expandVarArgs
//

bool expandVarArgs(const vector<ExprPtr> &args,
                   EnvPtr env,
                   vector<ExprPtr> &outArgs)
{
    bool expanded = false;
    for (unsigned i = 0; i < args.size(); ++i) {
        if (args[i]->exprKind == VAR_ARGS_REF) {
            ObjectPtr z = lookupEnv(env, new Identifier("%varArgs"));
            VarArgsInfo *vaInfo = (VarArgsInfo *)z.ptr();
            if (!vaInfo->hasVarArgs)
                error(args[i], "varargs unavailable");
            expanded = true;
            outArgs.insert(outArgs.end(),
                           vaInfo->varArgs.begin(),
                           vaInfo->varArgs.end());
        }
        else {
            outArgs.push_back(args[i]);
        }
    }
    return expanded;
}



//
// analyzeStaticObject
//

ObjectPtr analyzeStaticObject(ObjectPtr x)
{
    switch (x->objKind) {
    case ENUM_MEMBER : {
        EnumMember *y = (EnumMember *)x.ptr();
        return new PValue(y->type, true);
    }
    case GLOBAL_VARIABLE : {
        GlobalVariable *y = (GlobalVariable *)x.ptr();
        if (!y->type) {
            if (y->analyzing)
                return NULL;
            y->analyzing = true;
            PValuePtr pv = analyzeValue(y->expr, y->env);
            y->analyzing = false;
            if (!pv)
                return NULL;
            y->type = pv->type;
        }
        return new PValue(y->type, false);
    }
    case EXTERNAL_VARIABLE : {
        ExternalVariable *y = (ExternalVariable *)x.ptr();
        if (!y->type2)
            y->type2 = evaluateType(y->type, y->env);
        return new PValue(y->type2, false);
    }
    case EXTERNAL_PROCEDURE : {
        ExternalProcedure *y = (ExternalProcedure *)x.ptr();
        if (!y->analyzed)
            analyzeExternal(y);
        return new PValue(y->ptrType, true);
    }
    case STATIC_GLOBAL : {
        StaticGlobal *y = (StaticGlobal *)x.ptr();
        if (!y->result) {
            if (y->analyzing)
                return NULL;
            y->analyzing = true;
            y->result = analyze(y->expr, y->env);
            y->analyzing = false;
        }
        return analyzeStaticObject(y->result);
    }
    case CVALUE : {
        CValue *y = (CValue *)x.ptr();
        return new PValue(y->type, false);
    }
    }
    return x;
}



//
// analyzeExternal
//

void analyzeExternal(ExternalProcedurePtr x)
{
    assert(!x->analyzed);
    if (!x->attributesVerified)
        verifyAttributes(x);
    vector<TypePtr> argTypes;
    for (unsigned i = 0; i < x->args.size(); ++i) {
        ExternalArgPtr y = x->args[i];
        y->type2 = evaluateType(y->type, x->env);
        argTypes.push_back(y->type2);
    }
    x->returnType2 = evaluateMaybeVoidType(x->returnType, x->env);
    CallingConv callingConv = CC_DEFAULT;
    if (x->attrStdCall)
        callingConv = CC_STDCALL;
    else if (x->attrFastCall)
        callingConv = CC_FASTCALL;
    x->ptrType = cCodePointerType(callingConv, argTypes,
                                  x->hasVarArgs, x->returnType2);
    x->analyzed = true;
}



//
// verifyAttributes
//

void verifyAttributes(ExternalProcedurePtr x)
{
    assert(!x->attributesVerified);
    x->attributesVerified = true;
    x->attrDLLImport = false;
    x->attrDLLExport = false;
    x->attrStdCall = false;
    x->attrFastCall = false;
    x->attrAsmLabel = "";
    for (unsigned i = 0; i < x->attributes.size(); ++i) {
        ExprPtr expr = x->attributes[i];
        if (expr->exprKind == NAME_REF) {
            const string &s = ((NameRef *)expr.ptr())->name->str;
            if (s == "dllimport") {
                if (x->attrDLLExport)
                    error(expr, "dllimport specified after dllexport");
                x->attrDLLImport = true;
            }
            else if (s == "dllexport") {
                if (x->attrDLLImport)
                    error(expr, "dllexport specified after dllimport");
                x->attrDLLExport = true;
            }
            else if (s == "stdcall") {
                if (x->attrFastCall)
                    error(expr, "stdcall specified after fastcall");
                x->attrStdCall = true;
            }
            else if (s == "fastcall") {
                if (x->attrStdCall)
                    error(expr, "fastcall specified after stdcall");
                x->attrFastCall = true;
            }
            else {
                error(expr, "invalid attribute");
            }
        }
        else if (expr->exprKind == STRING_LITERAL) {
            StringLiteral *y = (StringLiteral *)expr.ptr();
            x->attrAsmLabel = y->value;
        }
        else {
            error(expr, "invalid attribute");
        }
    }
}

void verifyAttributes(ExternalVariablePtr x)
{
    assert(!x->attributesVerified);
    x->attributesVerified = true;
    x->attrDLLImport = false;
    x->attrDLLExport = false;
    for (unsigned i = 0; i < x->attributes.size(); ++i) {
        ExprPtr expr = x->attributes[i];
        if (expr->exprKind != NAME_REF)
            error(expr, "invalid attribute");
        const string &s = ((NameRef *)expr.ptr())->name->str;
        if (s == "dllimport") {
            if (x->attrDLLExport)
                error(expr, "dllimport specified after dllexport");
            x->attrDLLImport = true;
        }
        else if (s == "dllexport") {
            if (x->attrDLLImport)
                error(expr, "dllexport specified after dllimport");
            x->attrDLLExport = true;
        }
        else {
            error(expr, "invalid attribute");
        }
    }
}



//
// analyzeFieldRef
//

ObjectPtr analyzeFieldRef(ObjectPtr x, IdentifierPtr name)
{
    switch (x->objKind) {
    case PVALUE : {
        PValuePtr y = (PValue *)x.ptr();
        if (y->type->typeKind == STATIC_TYPE) {
            StaticType *t = (StaticType *)y->type.ptr();
            return analyzeFieldRef(t->obj, name);
        }
        vector<ExprPtr> args;
        args.push_back(new ObjectExpr(x));
        args.push_back(new ObjectExpr(name.ptr()));
        ObjectPtr prim = primName("recordFieldRefByName");
        return analyzeInvoke(prim, args, new Env());
    }
    case MODULE_HOLDER : {
        ModuleHolderPtr y = (ModuleHolder *)x.ptr();
        ObjectPtr z = lookupModuleMember(y, name);
        return analyzeStaticObject(z);
    }
    default :
        error("invalid field access operation");
        return NULL;
    }
}



//
// analyzeIndexing
//

ObjectPtr analyzeIndexing(ObjectPtr x, const vector<ExprPtr> &args, EnvPtr env)
{
    switch (x->objKind) {

    case PVALUE : {
        PValue *y = (PValue *)x.ptr();
        if (y->type->typeKind == STATIC_TYPE) {
            StaticType *t = (StaticType *)y->type.ptr();
            return analyzeIndexing(t->obj, args, env);
        }
        vector<ExprPtr> args2;
        args2.push_back(new ObjectExpr(x));
        args2.insert(args2.end(), args.begin(), args.end());
        ObjectPtr z = kernelName("index");
        return analyzeInvoke(z, args2, env);
    }

    case PRIM_OP : {
        PrimOpPtr y = (PrimOp *)x.ptr();

        switch (y->primOpCode) {

        case PRIM_Pointer : {
            ensureArity(args, 1);
            TypePtr t = evaluateType(args[0], env);
            return pointerType(t).ptr();
        }

        case PRIM_CodePointer : {
            if (args.size() < 1)
                error("atleast one argument required for"
                      " code pointer types.");
            vector<TypePtr> types;
            for (unsigned i = 0; i+1 < args.size(); ++i)
                types.push_back(evaluateType(args[i], env));
            TypePtr returnType = evaluateMaybeVoidType(args.back(), env);
            return codePointerType(types, returnType, true).ptr();
        }

        case PRIM_RefCodePointer : {
            if (args.size() < 1)
                error("atleast one argument required for"
                      " code pointer types.");
            vector<TypePtr> types;
            for (unsigned i = 0; i+1 < args.size(); ++i)
                types.push_back(evaluateType(args[i], env));
            TypePtr returnType = evaluateMaybeVoidType(args.back(), env);
            return codePointerType(types, returnType, false).ptr();
        }

        case PRIM_CCodePointer : {
            if (args.size() < 1)
                error("atleast one argument required for"
                      " code pointer types.");
            vector<TypePtr> types;
            for (unsigned i = 0; i+1 < args.size(); ++i)
                types.push_back(evaluateType(args[i], env));
            TypePtr returnType = evaluateMaybeVoidType(args.back(), env);
            return cCodePointerType(CC_DEFAULT, types,
                                    false, returnType).ptr();
        }

        case PRIM_StdCallCodePointer : {
            if (args.size() < 1)
                error("atleast one argument required for"
                      " code pointer types.");
            vector<TypePtr> types;
            for (unsigned i = 0; i+1 < args.size(); ++i)
                types.push_back(evaluateType(args[i], env));
            TypePtr returnType = evaluateMaybeVoidType(args.back(), env);
            return cCodePointerType(CC_STDCALL, types,
                                    false, returnType).ptr();
        }

        case PRIM_FastCallCodePointer : {
            if (args.size() < 1)
                error("atleast one argument required for"
                      " code pointer types.");
            vector<TypePtr> types;
            for (unsigned i = 0; i+1 < args.size(); ++i)
                types.push_back(evaluateType(args[i], env));
            TypePtr returnType = evaluateMaybeVoidType(args.back(), env);
            return cCodePointerType(CC_FASTCALL, types,
                                    false, returnType).ptr();
        }

        case PRIM_Array : {
            ensureArity(args, 2);
            TypePtr t = evaluateType(args[0], env);
            int n = evaluateInt(args[1], env);
            return arrayType(t, n).ptr();
        }

        case PRIM_Tuple : {
            vector<TypePtr> types;
            for (unsigned i = 0; i < args.size(); ++i)
                types.push_back(evaluateType(args[i], env));
            return tupleType(types).ptr();
        }

        case PRIM_Static : {
            ensureArity(args, 1);
            ObjectPtr obj = evaluateStatic(args[0], env);
            return staticType(obj).ptr();
        }

        }
    }

    case RECORD : {
        RecordPtr y = (Record *)x.ptr();
        ensureArity(args, y->patternVars.size());
        vector<ObjectPtr> params;
        for (unsigned i = 0; i < args.size(); ++i)
            params.push_back(evaluateStatic(args[i], env));
        return recordType(y, params).ptr();
    }

    }
    error("invalid indexing operation");
    return NULL;
}



//
// analyzeInvoke
//

ObjectPtr analyzeInvoke(ObjectPtr x, const vector<ExprPtr> &args, EnvPtr env)
{
    switch (x->objKind) {
    case TYPE :
    case RECORD :
    case PROCEDURE :
    case OVERLOADABLE :
        return analyzeInvokeCallable(x, args, env);
    case STATIC_PROCEDURE : {
        StaticProcedurePtr y = (StaticProcedure *)x.ptr();
        StaticInvokeEntryPtr entry = analyzeStaticProcedure(y, args, env);
        if (!entry->result)
            return NULL;
        return analyzeStaticObject(entry->result);
    }
    case STATIC_OVERLOADABLE : {
        StaticOverloadablePtr y = (StaticOverloadable *)x.ptr();
        StaticInvokeEntryPtr entry = analyzeStaticOverloadable(y, args, env);
        if (!entry->result)
            return NULL;
        return analyzeStaticObject(entry->result);
    }
    case PVALUE : {
        PValue *y = (PValue *)x.ptr();
        if (y->type->typeKind == STATIC_TYPE) {
            StaticType *t = (StaticType *)y->type.ptr();
            return analyzeInvoke(t->obj, args, env);
        }
        return analyzeInvokeValue(y, args, env);
    }
    case PRIM_OP :
        return analyzeInvokePrimOp((PrimOp *)x.ptr(), args, env);
    }
    error("invalid call operation");
    return NULL;
}



//
// analyzeInvokeCallable, analyzeInvokeSpecialCase, analyzeanalyzeCallable
//

ObjectPtr analyzeInvokeCallable(ObjectPtr x,
                                const vector<ExprPtr> &args,
                                EnvPtr env)
{
    const vector<bool> &isStaticFlags =
        lookupIsStaticFlags(x, args.size());
    vector<ObjectPtr> argsKey;
    vector<ValueTempness> argsTempness;
    vector<LocationPtr> argLocations;
    if (!computeArgsKey(isStaticFlags, args, env,
                        argsKey, argsTempness, argLocations))
        return NULL;
    ObjectPtr result = analyzeInvokeSpecialCase(x, isStaticFlags, argsKey);
    if (result.ptr())
        return result;
    InvokeStackContext invokeStackContext(x, argsKey);
    InvokeEntryPtr entry =
        analyzeCallable(x, isStaticFlags,
                        argsKey, argsTempness,
                        argLocations);
    if (entry->inlined)
        return analyzeInvokeInlined(entry, args, env);
    if (!entry->analyzed)
        return NULL;
    return entry->analysis;
}

ObjectPtr analyzeInvokeSpecialCase(ObjectPtr x,
                                   const vector<bool> &isStaticFlags,
                                   const vector<ObjectPtr> &argsKey)
{
    switch (x->objKind) {
    case TYPE : {
        Type *y = (Type *)x.ptr();
        if (isStaticFlags.empty())
            return new PValue(y, true);
        break;
    }
    case OVERLOADABLE : {
        if ((x == kernelName("destroy")) &&
            (isStaticFlags.size() == 1) &&
            (!isStaticFlags[0]))
        {
            ObjectPtr y = argsKey[0];
            assert(y->objKind == TYPE);
            if (isPrimitiveType((Type *)y.ptr()))
                return voidValue.ptr();
        }
        break;
    }
    }
    return NULL;
}

static InvokeEntryPtr findNextMatchingEntry(InvokeSetPtr invokeSet,
                                            unsigned entryIndex,
                                            const vector<OverloadPtr> &overloads)
{
    if (entryIndex < invokeSet->entries.size())
        return invokeSet->entries[entryIndex];

    assert(entryIndex == invokeSet->entries.size());

    unsigned overloadIndex;
    if (invokeSet->overloadIndices.empty())
        overloadIndex = 0;
    else
        overloadIndex = invokeSet->overloadIndices.back() + 1;

    for (; overloadIndex < overloads.size(); ++overloadIndex) {
        OverloadPtr y = overloads[overloadIndex];
        MatchResultPtr result = matchInvoke(y->code, y->env,
                                            invokeSet->isStaticFlags,
                                            invokeSet->argsKey,
                                            y->target, invokeSet->callable);
        if (result->matchCode == MATCH_SUCCESS) {
            InvokeEntryPtr entry =
                new InvokeEntry(invokeSet->callable,
                                invokeSet->isStaticFlags,
                                invokeSet->argsKey);
            invokeSet->entries.push_back(entry);
            invokeSet->overloadIndices.push_back(overloadIndex);
            entry->code = clone(y->code);
            MatchSuccess *z = (MatchSuccess *)result.ptr();
            entry->staticArgs = z->staticArgs;
            entry->env = z->env;
            entry->fixedArgTypes = z->fixedArgTypes;
            entry->fixedArgNames = z->fixedArgNames;
            entry->hasVarArgs = z->hasVarArgs;
            entry->varArgTypes = z->varArgTypes;
            entry->inlined = y->inlined;
            return entry;
        }
    }
    return NULL;
}

static bool tempnessMatches(CodePtr code,
                            const vector<ValueTempness> &tempness)
{
    unsigned j = 0;
    for (unsigned i = 0; i < code->formalArgs.size(); ++i) {
        FormalArgPtr farg = code->formalArgs[i];
        if (farg->objKind != VALUE_ARG)
            continue;
        ValueArg *arg = (ValueArg *)farg.ptr();
        assert(j < tempness.size());
        ValueTempness vt = tempness[j++];
        if ((vt & arg->tempness) == 0)
            return false;
    }
    if (code->hasVarArgs)
        assert(j <= tempness.size());
    else
        assert(j == tempness.size());
    return true;
}

InvokeEntryPtr analyzeCallable(ObjectPtr x,
                               const vector<bool> &isStaticFlags,
                               const vector<ObjectPtr> &argsKey,
                               const vector<ValueTempness> &argsTempness,
                               const vector<LocationPtr> &argLocations)
{
    InvokeSetPtr invokeSet = lookupInvokeSet(x, isStaticFlags, argsKey);
    const vector<OverloadPtr> &overloads = callableOverloads(x);

    unsigned i = 0;
    InvokeEntryPtr entry;
    while ((entry = findNextMatchingEntry(invokeSet, i, overloads)).ptr()) {
        if (tempnessMatches(entry->code, argsTempness))
            break;
        ++i;
    }
    if (!entry)
        error("no matching operation");
    if (entry->analyzed || entry->analyzing)
        return entry;
    if (entry->inlined)
        return entry;

    entry->analyzing = true;
    analyzeCodeBody(entry);
    entry->analyzing = false;

    return entry;
}



//
// analyzeInvokeInlined
//

ObjectPtr analyzeInvokeInlined(InvokeEntryPtr entry,
                               const vector<ExprPtr> &args,
                               EnvPtr env)
{
    assert(entry->inlined);

    CodePtr code = entry->code;

    if (code->returnType.ptr()) {
        TypePtr retType = evaluateMaybeVoidType(code->returnType, entry->env);
        if (!retType)
            return voidValue.ptr();
        return new PValue(retType, !code->returnRef);
    }

    if (entry->hasVarArgs)
        assert(args.size() >= entry->fixedArgNames.size());
    else
        assert(args.size() == entry->fixedArgNames.size());

    EnvPtr bodyEnv = new Env(entry->env);

    for (unsigned i = 0; i < entry->fixedArgNames.size(); ++i) {
        ExprPtr expr = new SCExpr(env, args[i]);
        addLocal(bodyEnv, entry->fixedArgNames[i], expr.ptr());
    }

    VarArgsInfoPtr vaInfo = new VarArgsInfo(entry->hasVarArgs);
    if (entry->hasVarArgs) {
        for (unsigned i = entry->fixedArgNames.size(); i < args.size(); ++i) {
            ExprPtr expr = new SCExpr(env, args[i]);
            vaInfo->varArgs.push_back(expr);
        }
    }
    addLocal(bodyEnv, new Identifier("%varArgs"), vaInfo.ptr());

    addLocal(bodyEnv, new Identifier("%returned"), NULL);
    ObjectPtr result;
    bool ok = analyzeStatement(code->body, bodyEnv, result);
    if (!result && !ok)
        return NULL;
    if (!result)
        return voidValue.ptr();
    return result;
}



//
// analyzeCodeBody
//

void analyzeCodeBody(InvokeEntryPtr entry) {
    assert(!entry->analyzed);
    ObjectPtr result;
    CodePtr code = entry->code;
    if (code->returnType.ptr()) {
        TypePtr retType = evaluateMaybeVoidType(code->returnType, entry->env);
        if (!retType)
            result = voidValue.ptr();
        else
            result = new PValue(retType, !code->returnRef);
    }
    else {
        EnvPtr bodyEnv = new Env(entry->env);

        for (unsigned i = 0; i < entry->fixedArgNames.size(); ++i) {
            PValuePtr parg = new PValue(entry->fixedArgTypes[i], false);
            addLocal(bodyEnv, entry->fixedArgNames[i], parg.ptr());
        }

        VarArgsInfoPtr vaInfo = new VarArgsInfo(entry->hasVarArgs);
        if (entry->hasVarArgs) {
            for (unsigned i = 0; i < entry->varArgTypes.size(); ++i) {
                PValuePtr parg = new PValue(entry->varArgTypes[i], false);
                ExprPtr expr = new ObjectExpr(parg.ptr());
                vaInfo->varArgs.push_back(expr);
            }
        }
        addLocal(bodyEnv, new Identifier("%varArgs"), vaInfo.ptr());

        addLocal(bodyEnv, new Identifier("%returned"), NULL);

        bool ok = analyzeStatement(code->body, bodyEnv, result);
        if (!result && !ok)
            return;
        if (!result)
            result = voidValue.ptr();
    }
    entry->analysis = result;
    entry->analyzed = true;
    if (result->objKind == PVALUE) {
        PValuePtr pret = (PValue *)result.ptr();
        entry->returnType = pret->type;
        entry->returnIsTemp = pret->isTemp;
    }
    else {
        assert(result->objKind == VOID_VALUE);
    }
}



//
// analyzeStatement
//

bool analyzeStatement(StatementPtr stmt, EnvPtr env, ObjectPtr &result)
{
    LocationContext loc(stmt->location);

    switch (stmt->stmtKind) {

    case BLOCK : {
        Block *x = (Block *)stmt.ptr();
        for (unsigned i = 0; i < x->statements.size(); ++i) {
            StatementPtr y = x->statements[i];
            if (y->stmtKind == BINDING) {
                env = analyzeBinding((Binding *)y.ptr(), env);
                if (!env)
                    return false;
            }
            else if (!analyzeStatement(y, env, result)) {
                return false;
            }
        }
        return true;
    }

    case LABEL :
    case BINDING :
    case ASSIGNMENT :
    case INIT_ASSIGNMENT :
    case UPDATE_ASSIGNMENT :
    case GOTO :
        return true;

    case RETURN : {
        Return *x = (Return *)stmt.ptr();
        if (!x->expr) {
            if (!result)
                result = voidValue.ptr();
            else if (result->objKind != VOID_VALUE)
                error("mismatching return kind");
        }
        else {
            PValuePtr pv = analyzeValue(x->expr, env);
            if (!pv)
                return false;
            PValuePtr y = new PValue(pv->type, true);
            if (!result)
                result = y.ptr();
            else if (result->objKind != PVALUE)
                error("mismatching return kind");
            PValue *prev = (PValue *)result.ptr();
            if (y->type != prev->type)
                error("mismatching return type");
            if (!prev->isTemp)
                error("mismatching return by ref & by value");
        }
        return true;
    }

    case RETURN_REF : {
        ReturnRef *x = (ReturnRef *)stmt.ptr();
        PValuePtr y = analyzeValue(x->expr, env);
        if (!y)
            return false;
        if (y->isTemp)
            error("cannot return a temporary by reference");
        if (!result)
            result = y.ptr();
        else if (result->objKind != PVALUE)
            error("mismatching return kind");
        PValue *prev = (PValue *)result.ptr();
        if (y->type != prev->type)
            error("mismatching return type");
        if (prev->isTemp)
            error("mismatching return by ref & by value");
        return true;
    }

    case IF : {
        If *x = (If *)stmt.ptr();
        bool thenResult = analyzeStatement(x->thenPart, env, result);
        bool elseResult = true;
        if (x->elsePart.ptr())
            elseResult = analyzeStatement(x->elsePart, env, result);
        return thenResult || elseResult;
    }

    case EXPR_STATEMENT :
        return true;

    case WHILE : {
        While *x = (While *)stmt.ptr();
        analyzeStatement(x->body, env, result);
        return true;
    }

    case BREAK :
    case CONTINUE :
        return true;

    case FOR : {
        For *x = (For *)stmt.ptr();
        if (!x->desugared)
            x->desugared = desugarForStatement(x);
        return analyzeStatement(x->desugared, env, result);
    }

    case SC_STATEMENT : {
        SCStatement *x = (SCStatement *)stmt.ptr();
        return analyzeStatement(x->statement, x->env, result);
    }

    case TRY : {
        Try *x = (Try *)stmt.ptr();
        bool tryResult = analyzeStatement(x->tryBlock, env, result);
        bool catchResult = analyzeStatement(x->catchBlock, env, result);
        return tryResult || catchResult; // FIXME
    }

    default :
        assert(false);
        return false;
    }
}

EnvPtr analyzeBinding(BindingPtr x, EnvPtr env)
{
    switch (x->bindingKind) {

    case VAR :
    case REF : {
        PValuePtr right = analyzeValue(x->expr, env);
        if (!right)
            return NULL;
        EnvPtr env2 = new Env(env);
        addLocal(env2, x->name, new PValue(right->type, false));
        return env2;
    }

    case STATIC : {
        ObjectPtr right = evaluateStatic(x->expr, env);
        if (!right)
            return NULL;
        EnvPtr env2 = new Env(env);
        addLocal(env2, x->name, right.ptr());
        return env2;
    }

    default :
        assert(false);
        return NULL;

    }
}



//
// analyzeStaticProcedure, analyzeStaticOverloadable
//

StaticInvokeEntryPtr
analyzeStaticProcedure(StaticProcedurePtr x,
                       const vector<ExprPtr> &argExprs,
                       EnvPtr env)
{
    vector<ObjectPtr> args;
    vector<LocationPtr> argLocations;
    for (unsigned i = 0; i < argExprs.size(); ++i) {
        args.push_back(evaluateStatic(argExprs[i], env));
        argLocations.push_back(argExprs[i]->location);
    }

    InvokeStackContext invokeStackContext(x.ptr(), args);

    StaticInvokeEntryPtr entry = lookupStaticInvoke(x.ptr(), args);
    if (!entry->result && !entry->analyzing) {
        entry->analyzing = true;

        MatchResultPtr result = matchStaticInvoke(x->code, x->env, args);
        if (result->matchCode != MATCH_SUCCESS)
            signalMatchError(result, argLocations);
        MatchSuccess *y = (MatchSuccess *)result.ptr();
        entry->result = evaluateStatic(x->code->body, y->env);

        entry->analyzing = false;
    }
    return entry;
}

StaticInvokeEntryPtr
analyzeStaticOverloadable(StaticOverloadablePtr x,
                          const vector<ExprPtr> &argExprs,
                          EnvPtr env)
{
    vector<ObjectPtr> args;
    vector<LocationPtr> argLocations;
    for (unsigned i = 0; i < argExprs.size(); ++i) {
        args.push_back(evaluateStatic(argExprs[i], env));
        argLocations.push_back(argExprs[i]->location);
    }

    InvokeStackContext invokeStackContext(x.ptr(), args);

    StaticInvokeEntryPtr entry = lookupStaticInvoke(x.ptr(), args);
    if (!entry->result && !entry->analyzing) {
        entry->analyzing = true;

        unsigned i = 0;
        for (; i < x->overloads.size(); ++i) {
            StaticOverloadPtr y = x->overloads[i];
            MatchResultPtr result = matchStaticInvoke(y->code, y->env, args);
            if (result->matchCode == MATCH_SUCCESS) {
                MatchSuccess *z = (MatchSuccess *)result.ptr();
                entry->result = evaluateStatic(y->code->body, z->env);
                break;
            }
        }
        if (i == x->overloads.size())
            error("no matching static overload");

        entry->analyzing = false;
    }
    return entry;
}

ObjectPtr analyzeInvokeValue(PValuePtr x, 
                             const vector<ExprPtr> &args,
                             EnvPtr env)
{
    switch (x->type->typeKind) {

    case CODE_POINTER_TYPE : {
        CodePointerType *y = (CodePointerType *)x->type.ptr();
        if (!y->returnType)
            return voidValue.ptr();
        return new PValue(y->returnType, y->returnIsTemp);
    }

    case CCODE_POINTER_TYPE : {
        CCodePointerType *y = (CCodePointerType *)x->type.ptr();
        if (!y->returnType)
            return voidValue.ptr();
        return new PValue(y->returnType, true);
    }

    default : {
        vector<ExprPtr> args2;
        args2.push_back(new ObjectExpr(x.ptr()));
        args2.insert(args2.end(), args.begin(), args.end());
        return analyzeInvoke(kernelName("call"), args2, env);
    }

    }
}

ObjectPtr analyzeInvokePrimOp(PrimOpPtr x,
                              const vector<ExprPtr> &args,
                              EnvPtr env)
{
    switch (x->primOpCode) {

    case PRIM_Type : {
        ensureArity(args, 1);
        ObjectPtr y = analyzeMaybeVoidValue(args[0], env);
        if (!y)
            return NULL;
        switch (y->objKind) {
        case PVALUE : {
            PValue *z = (PValue *)y.ptr();
            return z->type.ptr();
        }
        case VOID_VALUE :
            return voidType.ptr();
        default :
            assert(false);
        }
        return NULL;
    }

    case PRIM_TypeP : {
        return new PValue(boolType, true);
    }

    case PRIM_TypeSize : {
        return new PValue(cSizeTType, true);
    }

    case PRIM_primitiveCopy :
        return voidValue.ptr();

    case PRIM_boolNot :
        return new PValue(boolType, true);

    case PRIM_numericEqualsP :
        return new PValue(boolType, true);

    case PRIM_numericLesserP :
        return new PValue(boolType, true);

    case PRIM_numericAdd :
    case PRIM_numericSubtract :
    case PRIM_numericMultiply :
    case PRIM_numericDivide : {
        ensureArity(args, 2);
        PValuePtr y = analyzeValue(args[0], env);
        if (!y)
            return NULL;
        return new PValue(y->type, true);
    }

    case PRIM_numericNegate : {
        ensureArity(args, 1);
        PValuePtr y = analyzeValue(args[0], env);
        if (!y)
            return NULL;
        return new PValue(y->type, true);
    }

    case PRIM_integerRemainder :
    case PRIM_integerShiftLeft :
    case PRIM_integerShiftRight :
    case PRIM_integerBitwiseAnd :
    case PRIM_integerBitwiseOr :
    case PRIM_integerBitwiseXor : {
        ensureArity(args, 2);
        PValuePtr y = analyzeValue(args[0], env);
        if (!y)
            return NULL;
        return new PValue(y->type, true);
    }

    case PRIM_integerBitwiseNot : {
        ensureArity(args, 1);
        PValuePtr y = analyzeValue(args[0], env);
        if (!y)
            return NULL;
        return new PValue(y->type, true);
    }

    case PRIM_numericConvert : {
        ensureArity(args, 2);
        TypePtr t = evaluateNumericType(args[0], env);
        return new PValue(t, true);
    }

    case PRIM_Pointer :
        error("Pointer type constructor cannot be called");

    case PRIM_addressOf : {
        ensureArity(args, 1);
        PValuePtr y = analyzeValue(args[0], env);
        if (!y)
            return NULL;
        return new PValue(pointerType(y->type), true);
    }

    case PRIM_pointerDereference : {
        ensureArity(args, 1);
        PValuePtr y = analyzePointerValue(args[0], env);
        if (!y)
            return NULL;
        PointerType *t = (PointerType *)y->type.ptr();
        return new PValue(t->pointeeType, false);
    }

    case PRIM_pointerEqualsP : {
        return new PValue(boolType, true);
    }

    case PRIM_pointerLesserP : {
        return new PValue(boolType, true);
    }

    case PRIM_pointerOffset : {
        ensureArity(args, 2);
        PValuePtr y = analyzePointerValue(args[0], env);
        return new PValue(y->type, true);
    }

    case PRIM_pointerToInt : {
        ensureArity(args, 2);
        TypePtr t = evaluateIntegerType(args[0], env);
        return new PValue(t, true);
    }

    case PRIM_intToPointer : {
        ensureArity(args, 2);
        TypePtr t = evaluateType(args[0], env);
        return new PValue(pointerType(t), true);
    }

    case PRIM_CodePointerP : {
        return new PValue(boolType, true);
    }

    case PRIM_CodePointer :
        error("CodePointer type constructor cannot be called");

    case PRIM_RefCodePointer :
        error("RefCodePointer type constructor cannot be called");

    case PRIM_makeCodePointer : {
        if (args.size() < 1)
            error("incorrect no. of parameters");
        ObjectPtr callable = evaluateStatic(args[0], env);
        switch (callable->objKind) {
        case TYPE :
        case RECORD :
        case PROCEDURE :
        case OVERLOADABLE :
            break;
        default :
            error(args[0], "invalid callable");
        }
        const vector<bool> &isStaticFlags =
            lookupIsStaticFlags(callable, args.size()-1);
        vector<ObjectPtr> argsKey;
        vector<ValueTempness> argsTempness;
        vector<LocationPtr> argLocations;
        for (unsigned i = 1; i < args.size(); ++i) {
            if (!isStaticFlags[i-1]) {
                TypePtr t = evaluateType(args[i], env);
                argsKey.push_back(t.ptr());
                argsTempness.push_back(LVALUE);
            }
            else {
                ObjectPtr param = evaluateStatic(args[i], env);
                argsKey.push_back(param);
            }
            argLocations.push_back(args[i]->location);
        }

        InvokeStackContext invokeStackContext(callable, argsKey);

        InvokeEntryPtr entry = analyzeCallable(callable, isStaticFlags,
                                               argsKey, argsTempness,
                                               argLocations);
        if (entry->inlined)
            error(args[0], "cannot create pointer to inlined code");
        if (!entry->analyzed)
            return NULL;
        vector<TypePtr> argTypes = entry->fixedArgTypes;
        if (entry->hasVarArgs) {
            argTypes.insert(argTypes.end(),
                            entry->varArgTypes.begin(),
                            entry->varArgTypes.end());
        }
        TypePtr cpType = codePointerType(argTypes,
                                         entry->returnType,
                                         entry->returnIsTemp);
        return new PValue(cpType, true);
    }

    case PRIM_CCodePointerP : {
        return new PValue(boolType, true);
    }

    case PRIM_CCodePointer :
        error("CCodePointer type constructor cannot be called");

    case PRIM_StdCallCodePointer :
        error("StdCallCodePointer type constructor cannot be called");

    case PRIM_FastCallCodePointer :
        error("FastCallCodePointer type constructor cannot be called");

    case PRIM_makeCCodePointer : {
        if (args.size() < 1)
            error("incorrect no. of parameters");
        ObjectPtr callable = evaluateStatic(args[0], env);
        switch (callable->objKind) {
        case TYPE :
        case RECORD :
        case PROCEDURE :
        case OVERLOADABLE :
            break;
        default :
            error(args[0], "invalid callable");
        }
        const vector<bool> &isStaticFlags =
            lookupIsStaticFlags(callable, args.size()-1);
        vector<ObjectPtr> argsKey;
        vector<ValueTempness> argsTempness;
        vector<LocationPtr> argLocations;
        for (unsigned i = 1; i < args.size(); ++i) {
            if (!isStaticFlags[i-1]) {
                TypePtr t = evaluateType(args[i], env);
                argsKey.push_back(t.ptr());
                argsTempness.push_back(LVALUE);
            }
            else {
                ObjectPtr param = evaluateStatic(args[i], env);
                argsKey.push_back(param);
            }
            argLocations.push_back(args[i]->location);
        }

        InvokeStackContext invokeStackContext(callable, argsKey);

        InvokeEntryPtr entry = analyzeCallable(callable, isStaticFlags,
                                               argsKey, argsTempness,
                                               argLocations);
        if (entry->inlined)
            error(args[0], "cannot create pointer to inlined code");
        if (!entry->analyzed)
            return NULL;
        vector<TypePtr> argTypes = entry->fixedArgTypes;
        if (entry->hasVarArgs) {
            argTypes.insert(argTypes.end(),
                            entry->varArgTypes.begin(),
                            entry->varArgTypes.end());
        }
        TypePtr ccpType = cCodePointerType(CC_DEFAULT,
                                           argTypes,
                                           false,
                                           entry->returnType);
        return new PValue(ccpType, true);
    }

    case PRIM_pointerCast : {
        ensureArity(args, 2);
        TypePtr t = evaluatePointerLikeType(args[0], env);
        return new PValue(t, true);
    }

    case PRIM_Array :
        error("Array type constructor cannot be called");

    case PRIM_array : {
        if (args.size() < 1)
            error("atleast one element required for creating an array");
        PValuePtr y = analyzeValue(args[0], env);
        if (!y)
            return NULL;
        TypePtr z = arrayType(y->type, args.size());
        return new PValue(z, true);
    }

    case PRIM_arrayRef : {
        ensureArity(args, 2);
        PValuePtr y = analyzeArrayValue(args[0], env);
        if (!y)
            return NULL;
        ArrayType *z = (ArrayType *)y->type.ptr();
        return new PValue(z->elementType, false);
    }

    case PRIM_TupleP : {
        return new PValue(boolType, true);
    }

    case PRIM_Tuple :
        error("Tuple type constructor cannot be called");

    case PRIM_TupleElementCount : {
        return new PValue(cSizeTType, true);
    }

    case PRIM_TupleElementOffset : {
        return new PValue(cSizeTType, true);
    }

    case PRIM_tuple : {
        vector<TypePtr> elementTypes;
        for (unsigned i = 0; i < args.size(); ++i) {
            PValuePtr y = analyzeValue(args[i], env);
            if (!y)
                return NULL;
            elementTypes.push_back(y->type);
        }
        return new PValue(tupleType(elementTypes), true);
    }

    case PRIM_tupleRef : {
        ensureArity(args, 2);
        PValuePtr y = analyzeTupleValue(args[0], env);
        if (!y)
            return NULL;
        size_t i = evaluateSizeT(args[1], env);
        TupleType *z = (TupleType *)y->type.ptr();
        if (i >= z->elementTypes.size())
            error(args[1], "tuple index out of range");
        return new PValue(z->elementTypes[i], false);
    }

    case PRIM_RecordP : {
        return new PValue(boolType, true);
    }

    case PRIM_RecordFieldCount : {
        return new PValue(cSizeTType, true);
    }

    case PRIM_RecordFieldOffset : {
        return new PValue(cSizeTType, true);
    }

    case PRIM_RecordFieldIndex : {
        return new PValue(cSizeTType, true);
    }

    case PRIM_recordFieldRef : {
        ensureArity(args, 2);
        PValuePtr y = analyzeRecordValue(args[0], env);
        if (!y)
            return NULL;
        RecordType *z = (RecordType *)y->type.ptr();
        size_t i = evaluateSizeT(args[1], env);
        const vector<TypePtr> &fieldTypes = recordFieldTypes(z);
        if (i >= fieldTypes.size())
            error("field index out of range");
        return new PValue(fieldTypes[i], false);
    }

    case PRIM_recordFieldRefByName : {
        ensureArity(args, 2);
        PValuePtr y = analyzeRecordValue(args[0], env);
        if (!y)
            return NULL;
        RecordType *z = (RecordType *)y->type.ptr();
        IdentifierPtr fname = evaluateIdentifier(args[1], env);
        const map<string, size_t> &fieldIndexMap = recordFieldIndexMap(z);
        map<string, size_t>::const_iterator fi =
            fieldIndexMap.find(fname->str);
        if (fi == fieldIndexMap.end())
            error(args[1], "field not in record");
        const vector<TypePtr> &fieldTypes = recordFieldTypes(z);
        return new PValue(fieldTypes[fi->second], false);
    }

    case PRIM_Static :
        error("Static type constructor cannot be called");

    case PRIM_StaticName : {
        ensureArity(args, 1);
        ObjectPtr y = evaluateStatic(args[0], env);
        ostringstream sout;
        printName(sout, y);
        ExprPtr z = new StringLiteral(sout.str());
        return analyze(z, env);
    }

    case PRIM_EnumP : {
        return new PValue(boolType, true);
    }

    case PRIM_enumToInt : {
        return new PValue(cIntType, true);
    }

    case PRIM_intToEnum : {
        ensureArity(args, 2);
        TypePtr t = evaluateEnumerationType(args[0], env);
        return new PValue(t, true);
    }

    default :
        assert(false);
        return NULL;
    }
}
