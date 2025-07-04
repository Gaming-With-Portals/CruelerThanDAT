#include "pch.hpp"
#include "LanguageManager.h"


LanguageManager& LanguageManager::Instance()
{

	static LanguageManager instance;
	return instance;

}

void LanguageManager::Init()
{

}


LanguageManager::LanguageManager() {
}

LanguageManager::~LanguageManager() = default;