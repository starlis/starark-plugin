#pragma once
/*                             READ ME
*
*   This file contains some basic and useful includes.
*   You can change this how you like, but I suggest
*	only removing something If you know what you are doing.
*/
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <API/ARK/Ark.h>
#include <Logger/Logger.h>
#include <Tools.h>
#include <macros.h>
#include <BlueprintCache.h>
#include <fstream>
#include "json.hpp"
#include <unordered_set>
#include <Timer.h>

#pragma comment(lib, "ArkApi.lib")