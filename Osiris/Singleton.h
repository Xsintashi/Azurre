#pragma once

// Implement Singletons
#define Singleton(TYPE) \
	public: \
	static std::shared_ptr<TYPE> Get() { return Single; } \
	private: \
	static std::shared_ptr<TYPE> Single;

#define Singleton_CPP(TYPE) \
	std::shared_ptr<TYPE> TYPE::Single = std::make_shared<TYPE>();
