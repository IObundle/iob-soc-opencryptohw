#undef START
#undef END
#undef ENTRY
#undef LAST_ENTRY

#ifdef INSTANTIATE_CLASS
#define START(structName,arrayName) typedef struct structName{
#define END(typeName) } typeName;
#define ENTRY(name,bitsize) int name;
#define LAST_ENTRY(name,bitsize) int name;
#endif

#ifdef INSTANTIATE_ARRAY
#define START(structName,arrayName) static Wire arrayName[] = {
#define END(structName) };
#define ENTRY(name,bitsize) {#name,bitsize},
#define LAST_ENTRY(name,bitsize) {#name,bitsize}
#endif

START(UnitFConfig_t,unitFConfigWires)
LAST_ENTRY(configDelay,8)
END(UnitFConfig)

START(UnitMConfig_t,unitMConfigWires)
LAST_ENTRY(configDelay,8)
END(UnitMConfig)

