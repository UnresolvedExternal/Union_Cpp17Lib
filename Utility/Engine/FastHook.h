namespace NAMESPACE
{
	template <class T, class E>
	auto InvokeExAuto_BySignature(string from, T to, const HookMode& mode, E& opt)
	{
		return Hook<T, E>{ static_cast<TInstance>(FindEngineAddress(from, typeid(to).name())), static_cast<TInstance>(to), mode, opt };
	}

#define AS_OPT(from, to, mode, option) InvokeExAuto_BySignature(#from, to, mode, option)

#define FASTHOOK_OPT(className, methodName, option)	\
	auto Hook_##className##_##methodName { AS_OPT(&##className##::##methodName, &##className##::##methodName##_Union, HookMode::Hook, option) }

#define FASTHOOK_PATCH_OPT(className, methodName, option)	\
	auto Hook_##className##_##methodName { AS_OPT(&##className##::##methodName, &##className##::##methodName##_Union, HookMode::Patch, option) }
}