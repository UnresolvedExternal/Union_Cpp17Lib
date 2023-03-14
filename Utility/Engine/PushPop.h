namespace NAMESPACE
{
	template <class T>
	T PopArgument(zCParser* parser)
	{
		if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float>)
		{
			T arg;
			parser->GetParameter(arg);
			return arg;
		}
		else if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, unsigned>)
		{
			int arg;
			parser->GetParameter(arg);
			return static_cast<T>(arg);
		}
		else if constexpr (std::is_same_v<T, zSTRING> || std::is_same_v<T, string>)
		{
			zSTRING arg;
			parser->GetParameter(arg);
			return arg;
		}
		else if constexpr (std::is_same_v<T, zCPar_Symbol*>)
			return parser->GetSymbol(parser->datastack.Pop());
		else if constexpr (std::is_same_v<T, Symbol>)
			return Symbol(parser, parser->datastack.Pop());
		else if constexpr (std::is_convertible_v<T, zCObject*>)
		{
			Symbol symbol(parser, parser->datastack.Pop());

			if (!symbol)
				return nullptr;

			zCObject* instance = reinterpret_cast<zCObject*>(symbol.GetSymbol()->offset);
			return dynamic_cast<T>(instance);
		}
		else if constexpr (std::is_pointer_v<T>)
		{
			Symbol symbol(parser, parser->datastack.Pop());

			if (!symbol)
				return nullptr;

			return reinterpret_cast<T>(symbol.GetSymbol()->offset);
		}
		else
			static_assert(false, "Wrong type");
	}

	void PopArguments(zCParser* parser)
	{

	}

	template <class THead, class ...TTail>
	void PopArguments(zCParser* parser, THead& head, TTail&... tail)
	{
		PopArguments(parser, tail...);
		head = PopArgument<THead>(parser);
	}

#define ZARGS(...) PopArguments(zCParser::GetParser(), __VA_ARGS__)

	namespace Helpers
	{
		class StringManager
		{
		private:
			std::vector<zSTRING*> strings;
			Sub<void> cleaner;

			void OnExit()
			{
				for (zSTRING* string : strings)
					delete string;

				strings.clear();
				cleaner = {};
			}

			StringManager()
			{
				strings.reserve(16u);
				cleaner = { GameEvent::Loop | GameEvent::MenuLoop | GameEvent::Exit, std::bind(&StringManager::OnExit, this) };
			}

		public:
			static StringManager& GetInstance()
			{
				static StringManager instance;
				return instance;
			}

			zSTRING& AddString(const char* string)
			{
				strings += new zSTRING{ string };
				return *strings.back();
			}
		};
	}

	template <class T> 
	void PushArgument(zCParser* parser, const T& arg, int argSymbol = -1)
	{
		if constexpr (std::is_same_v<T, int> || std::is_same_v<T, bool> || std::is_same_v<T, unsigned>)
			parser->SetReturn(static_cast<int>(arg));
		else if constexpr (std::is_same_v<T, float>)
			parser->SetReturn(arg);
		else if constexpr (std::is_convertible_v<T, const char*>)
		{
			zSTRING& string = Helpers::StringManager::GetInstance().AddString(static_cast<const char*>(arg));
			parser->SetReturn(string);
		}
		else if constexpr (std::is_same_v<T, Symbol>)
			if (argSymbol == -1)
				parser->SetReturn(reinterpret_cast<void*>(arg.GetSymbol()->offset));
			else
			{
				parser->GetSymbol(argSymbol)->offset = arg.GetSymbol()->offset;
				parser->datastack.Push(argSymbol);
			}
		else if constexpr (std::is_pointer_v<T>)
			if (argSymbol == -1)
				parser->SetReturn((void*)arg);
			else
			{
				parser->GetSymbol(argSymbol)->offset = reinterpret_cast<int>(arg);
				parser->datastack.Push(argSymbol);
			}
		else
			static_assert(false, "Wrong type");
	}

	template <class... TArgs>
	void PushArguments(zCParser* parser, const TArgs&... args)
	{
		(PushArgument(parser, args), ...);
	}

#define ZRETURN(...) PushArguments(zCParser::GetParser(), __VA_ARGS__)

	template <class T, class TName, class ...TArgs>
	T CallParser(zCParser* parser, const TName& nameIndex, TArgs&&... args)
	{
		int index = -1;

		if constexpr (std::is_convertible_v<TName, const char*>)
			index = parser->GetIndex(static_cast<const char*>(nameIndex));
		else
			index = nameIndex;

		Symbol func(parser, index);
		int argument = func.GetIndex() + 1;
		(PushArgument(parser, args, argument++), ...);

		if (func.GetType() == Symbol::Type::ExternalFunc)
			reinterpret_cast<int(__cdecl*)()>(func.GetValue<int>(0))();
		else
			parser->DoStack(func.GetValue<int>(0));

		if constexpr (std::is_same_v<T, void>)
			return;
		else
			return PopArgument<T>(parser);
	}

	template <class T>
	int GetParserType()
	{
		if constexpr (std::is_same_v<T, int> || std::is_same_v<T, bool>)
			return zPAR_TYPE_INT;
		else if constexpr (std::is_same_v<T, unsigned>)
			return zPAR_TYPE_FUNC;
		else if constexpr (std::is_same_v<T, float>)
			return zPAR_TYPE_FLOAT;
		else if constexpr (std::is_same_v<T, zSTRING> || std::is_same_v<T, string> || std::is_same_v<T, const char*>)
			return zPAR_TYPE_STRING;
		else if constexpr (std::is_same_v<T, void>)
			return zPAR_TYPE_VOID;
		else if constexpr (std::is_same_v<T, Symbol> || std::is_pointer_v<T>)
			return zPAR_TYPE_INSTANCE;
		else
			static_assert(false, "Wrong type");
	}

	class ExternalRegistrationBase
	{
	protected:
		zCParser*& parser;
		string name;
		int(__cdecl* func)();

		static std::vector<ExternalRegistrationBase*> registrations;

	public:
		static ActiveValue<bool> enableGameExternals;
		static ActiveValue<bool> enableMenuExternals;
		
		static void TriggerAll(zCParser* currentParser)
		{
			for (ExternalRegistrationBase* registration : registrations)
				registration->TryRegister(currentParser);
		}

		ExternalRegistrationBase(zCParser*& parser, const string& name, int(__cdecl* func)()) :
			parser{ parser },
			name{ name },
			func{ func }
		{
			if (!CHECK_THIS_ENGINE)
				return;

			if (&parser == &Gothic::Parsers::Game)
				enableGameExternals = true;

			if (&parser == &Gothic::Parsers::Menu)
				enableMenuExternals = true;

			registrations += this;
		}

		virtual void TryRegister(zCParser* currentParser) = 0;
	};

	ActiveValue<bool> ExternalRegistrationBase::enableGameExternals;
	ActiveValue<bool> ExternalRegistrationBase::enableMenuExternals;
	std::vector<ExternalRegistrationBase*> ExternalRegistrationBase::registrations;

	template <class TReturn, class... TArgs>
	class ExternalRegistration : public ExternalRegistrationBase
	{
	public:
		ExternalRegistration(zCParser*& parser, const string& name, int (__cdecl* func)()) :
			ExternalRegistrationBase{ parser, name, func }
		{

		}

		virtual void TryRegister(zCParser* currentParser) override
		{
			if (currentParser == parser)
				parser->DefineExternal(name.GetVector(), func, GetParserType<TReturn>(), GetParserType<TArgs>()..., zPAR_TYPE_VOID);
		}
	};

	Sub registerGameExternals(ZSUB(GameEvent::DefineExternals), ExternalRegistrationBase::enableGameExternals, []
		{
			ExternalRegistrationBase::TriggerAll(parser);
		});

	void __cdecl Hook_zCMenu_CreateParser();
	Hook<void(__cdecl*)(), ActiveValue<bool>> Ivk_zCMenu_CreateParser(ZENFOR(0x004CD320, 0x004DDA90, 0x004D7950, 0x004D9F10), &Hook_zCMenu_CreateParser, HookMode::Patch, ExternalRegistrationBase::enableMenuExternals);
	void __cdecl Hook_zCMenu_CreateParser()
	{
		zCParser* const oldParser = parserMenu;

		Ivk_zCMenu_CreateParser();

		if (parserMenu == oldParser)
			return;

		ExternalRegistrationBase::TriggerAll(parserMenu);
	}

#define ZEXTERNAL(ret, name, ...) ExternalRegistration<ret, __VA_ARGS__> name ## _reg(parser, #name, &name)
#define ZEXTERNAL_MENU(ret, name, ...) ExternalRegistration<ret, __VA_ARGS__> name ## _regMenu(parserMenu, #name, &name)
#define ZEXTERNAL_NS(ret, name, ...) ExternalRegistration<ret, __VA_ARGS__> name ## _reg(parser, string{ PROJECT_NAME } + ":" + #name, &name);
}