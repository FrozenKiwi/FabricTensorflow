/* 
 * This auto-generated file contains typemapping conversion fn
 * declarations for the data types found in Tensorflow codegen file
 *  - Do not modify this file, it will be overwritten
 */


#pragma once
inline bool String_to_char(const Fabric::EDK::KL::String & from, char* & to) { 
	to = const_cast<char*>(from.c_str());
  return true;
}

inline bool char_to_String(const char* const & from, Fabric::EDK::KL::String & to) {
	to = from;
  return true; 
}

