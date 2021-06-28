#include "jsonUtils.hpp"

namespace HyperTiler::js {
	ParseContext::ParseContext(json const& data) : data(data), stackLevel(0), er() { }

	string ErrorStack::what() const {
		string res;
		for (auto const& kvp : *this) res += string(kvp.first, '\t') + ": " + kvp.second + "\n";
		return res;
	}

	bool Load(json const& j, ErrorStack& er, int stackLevel, string& val) {
		if (j.is_string()) {
			val = j.get<string>();
			return true;
		}
		er.emplace_back(stackLevel, "Failed to Load " + string(j.type_name()) + " as string");
		return false;
	}

	bool Load(json const& j, ErrorStack& er, int stackLevel, int& val) {
		if (j.is_number_integer() || j.is_number_unsigned()) {
			val = j.get<int>();
			return true;
		}
		er.emplace_back(stackLevel, "Failed to Load " + string(j.type_name()) + " as int");
		return false;
	}

	bool Load(json const& j, ErrorStack& er, int stackLevel, unsigned int& val) {
		if (j.is_number_unsigned()) {
			val = j.get<unsigned int>();
			return true;
		}
		er.emplace_back(stackLevel, "Failed to Load " + string(j.type_name()) + " as uint");
		return false;
	}

	bool Load(json const& j, ErrorStack& er, int stackLevel, float& val) {
		if (j.is_number_float() || j.is_number_integer() || j.is_number_unsigned()) {
			val = j.get<float>();
			return true;
		}
		er.emplace_back(stackLevel, "Failed to Load " + string(j.type_name()) + " as float");
		return false;
	}

	bool Load(json const& j, ErrorStack& er, int stackLevel, double& val) {
		if (j.is_number_float() || j.is_number_integer() || j.is_number_unsigned()) {
			val = j.get<double>();
			return true;
		}
		er.emplace_back(stackLevel, "Failed to Load " + string(j.type_name()) + " as double");
		return false;
	}

	bool Load(json const& j, ErrorStack& er, int stackLevel, bool& val) {
		if (j.is_boolean()) {
			val = j.get<bool>();
			return true;
		}
		er.emplace_back(stackLevel, "Failed to Load " + string(j.type_name()) + " as bool");
		return false;
	}
}