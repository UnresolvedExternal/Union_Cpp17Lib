#include <variant>

enum class HookMode
{
	Disabled = 0,
	Hook,
	Patch
};

template <class T, class ActiveValue = void>
class Hook
{
private:
	ActiveValue* option;
	Delegate<void()> onChange;
	std::variant<std::monostate, CInvoke<T>, ModulePatchCallInvoker<T>> ivk;
	const TInstance origin;
	const TInstance hook;
	const HookMode mode;
	bool attached;

	void TryAttach()
	{
		if (attached)
			return;

		attached = true;

		if (ivk.index() == 0)
			if (mode == HookMode::Patch)
				ivk.emplace<2>(origin, hook);
			else
				ivk.emplace<1>(origin, hook);

		if (mode == HookMode::Patch)
			std::get<2>(ivk).Attach(origin, hook);
		else
			std::get<1>(ivk).Attach(origin, hook);
	}

	void TryDetach()
	{
		if (!attached)
			return;

		attached = false;

		if (mode == HookMode::Patch)
			std::get<2>(ivk).Detach();
		else
			std::get<1>(ivk).Detach();
	}

public:
	Hook(const TInstance& origin, const TInstance& hook, const HookMode& mode, ActiveValue& option) :
		option{ (origin && mode != HookMode::Disabled) ? &option : nullptr },
		ivk{ },
		origin{ origin },
		hook{ hook },
		mode{ mode },
		attached{ false }
	{
		if (!this->option)
			return;

		if (static_cast<bool>(*option))
			TryAttach();

		onChange = [this]()
		{
			if (static_cast<bool>(**this->option))
				TryAttach();
			else
				TryDetach();
		};

		option.onChange += onChange;
	}

	operator T()
	{
		if (mode == HookMode::Patch)
			return std::get<2>(ivk);
		else
			return std::get<1>(ivk);
	}

	~Hook()
	{
		if (!option)
			return;
			
		option->onChange -= onChange;
		TryDetach();
	}
};

template <class T>
class Hook<T, void>
{
private:
	std::variant<std::monostate, CInvoke<T>, ModulePatchCallInvoker<T>> ivk;
	const TInstance origin;
	const TInstance hook;
	const HookMode mode;
	bool attached;

	void TryAttach()
	{
		if (attached)
			return;

		attached = true;

		if (ivk.index() == 0)
			if (mode == HookMode::Patch)
				ivk.emplace<2>(origin, hook);
			else
				ivk.emplace<1>(origin, hook);

		if (mode == HookMode::Patch)
			std::get<2>(ivk).Attach(origin, hook);
		else
			std::get<1>(ivk).Attach(origin, hook);
	}

	void TryDetach()
	{
		if (!attached)
			return;

		attached = false;

		if (mode == HookMode::Patch)
			std::get<2>(ivk).Detach();
		else
			std::get<1>(ivk).Detach();
	}

public:
	Hook(const TInstance& origin, const TInstance& hook, const HookMode& mode) :
		ivk{ },
		origin{ origin },
		hook{ hook },
		mode{ mode },
		attached{ false }
	{
		if (!origin || mode == HookMode::Disabled)
			return;

		TryAttach();
	}

	operator T()
	{
		if (mode == HookMode::Patch)
			return std::get<2>(ivk);
		else
			return std::get<1>(ivk);
	}

	~Hook()
	{
		TryDetach();
	}
};
