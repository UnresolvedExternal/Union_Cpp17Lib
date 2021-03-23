#pragma once

#include <functional>
#include <vector>
#include <unordered_set>

enum GameEvent : uint32_t
{
	NoEvent = 0u,

	Entry = 1u << 0u,
	Init = 1u << 1u,
	Exit = 1u << 2u,
	PreLoop = 1u << 3u,
	Loop = 1u << 4u,
	PostLoop = 1u << 5u,
	MenuLoop = 1u << 6u,
	SaveBegin = 1u << 7u,
	SaveEnd = 1u << 8u,
	LoadBegin_NewGame = 1u << 9u,
	LoadEnd_NewGame = 1u << 10u,
	LoadBegin_SaveGame = 1u << 11u,
	LoadEnd_SaveGame = 1u << 12u,
	LoadBegin_ChangeLevel = 1u << 13u,
	LoadEnd_ChangeLevel = 1u << 14u,
	LoadBegin_Trigger = 1u << 15u,
	LoadEnd_Trigger = 1u << 16u,
	Pause = 1u << 17u,
	Unpause = 1u << 18u,
	DefineExternals = 1u << 19u,
	Detach = 1u << 20u,

	Execute = 1u << 21u,
	All = (1u << 22u) - 1u,
	LoadBegin = LoadBegin_NewGame | LoadBegin_SaveGame | LoadBegin_ChangeLevel,
	LoadEnd = LoadEnd_NewGame | LoadEnd_SaveGame | LoadEnd_ChangeLevel
};

constexpr uint32_t maxGameEvents = 21u;

#define TNUM(x) static_cast<uint32_t>(x)
#define TEVE(x) static_cast<GameEvent>(x);

inline GameEvent operator&(const GameEvent& x, const GameEvent& y) { return TEVE(TNUM(x) & TNUM(y)); }
inline GameEvent operator~(const GameEvent& x) { return TEVE(~TNUM(x)); }
inline GameEvent operator|(const GameEvent& x, const GameEvent& y) { return TEVE(TNUM(x) | TNUM(y)); }

#undef TNUM
#undef TEVE

class Publisher
{
private:
	std::vector<const std::function<void()>*> subs[maxGameEvents];
	std::unordered_set<const std::function<void()>*> subsHashed[maxGameEvents];

	Publisher() = default;

public:
	static Publisher& GetInstance()
	{
		static Publisher instance;
		return instance;
	}
	
	void Subscribe(const GameEvent& events, const Delegate<void()>& delegate)
	{
		for (uint32_t i = 0u, flags = events; flags && i < maxGameEvents; i++, flags >>= 1)
			if (flags & 1u)
			{
				subs[i].push_back(&delegate.GetRaw());
				subsHashed[i].insert(&delegate.GetRaw());
			}
	}
	
	void Unsubscribe(const GameEvent& events, const Delegate<void()>& delegate)
	{
		for (uint32_t i = 0u, flags = events; flags && i < maxGameEvents; i++, flags >>= 1)
			if (flags & 1u)
			{
				subs[i].erase(std::find(subs[i].begin(), subs[i].end(), &delegate.GetRaw()));
				subsHashed[i].erase(subsHashed[i].find(&delegate.GetRaw()));
			}
	}
	
	void Raise(const GameEvent& events)
	{
		size_t index = 0;

		for (size_t flags = events; flags; flags >>= 1)
			index += 1;

		index -= 1;

		std::vector<const std::function<void()>*> copy = subs[index];

		for (const std::function<void()>* func : copy)
			if (subsHashed[index].find(func) != subsHashed[index].end())
				(*func)();
	}
};