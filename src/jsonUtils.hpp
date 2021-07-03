#pragma once

#include "TileUtils.hpp"

namespace HyperTiler {
	namespace js {
		struct ErrorStack : public vector<pair<int, string>> {
			string what() const;
		};

		template<typename T>
		inline bool Load(json const& j, ErrorStack& er, int stackLevel, T& val) {
			try {
				val = T(j);
				return true;
			} catch (ErrorStack const& ex) {
				er.push_back(ex.front());
				return false;
			}
		}

		template bool Load(json const& j, ErrorStack& er, int stackLevel, string& val);
		template bool Load(json const& j, ErrorStack& er, int stackLevel, int& val);
		template bool Load(json const& j, ErrorStack& er, int stackLevel, unsigned int& val);
		template bool Load(json const& j, ErrorStack& er, int stackLevel, float& val);
		template bool Load(json const& j, ErrorStack& er, int stackLevel, double& val);
		template bool Load(json const& j, ErrorStack& er, int stackLevel, bool& val);

		//template<typename T>
		//inline json Save(T const& val) {
		//	return val.Save();
		//}
		
		template<typename T>
		inline json Save(T const& val) {
			return json(val);
		}

#define Make_Vec_Load(L, TT)\
		template<> inline bool Load(json const& j, ErrorStack& er, int stackLevel, glm::vec<L,TT>& val)			{			\
			static_assert(L >= 1 && L <= 4);																			\
			const size_t insertPos = er.size();																			\
			if (j.is_array() && j.size() == L) {																		\
				json::array_t ar = j.get<json::array_t>();																\
				for (int i = 0; i < L; ++i) {																			\
					if (!Load<TT>(ar[i], er, stackLevel + 1, val[i])) {													\
						er.emplace(er.begin() + insertPos, stackLevel, "Failed to Load vector, element incorrect");		\
						return false;																					\
					}																									\
				}																										\
				return true;																							\
			}																											\
			return false;																								\
		};

		Make_Vec_Load(1, int);
		Make_Vec_Load(2, int);
		Make_Vec_Load(3, int);
		Make_Vec_Load(4, int);
		Make_Vec_Load(1, uint32_t);
		Make_Vec_Load(2, uint32_t);
		Make_Vec_Load(3, uint32_t);
		Make_Vec_Load(4, uint32_t);
		Make_Vec_Load(1, float);
		Make_Vec_Load(2, float);
		Make_Vec_Load(3, float);
		Make_Vec_Load(4, float);
		Make_Vec_Load(1, double);
		Make_Vec_Load(2, double);
		Make_Vec_Load(3, double);
		Make_Vec_Load(4, double);

		template<uint32_t L, typename T>
		inline json Save(glm::vec<L, T> const& val) {
			static_assert(L >= 1 && L <= 4);
			json::array_t res;
			res.resize(L);
			for (int i = 0; i < L; ++i) {
				res[i] = Save(val[i]);
			}
			return res;
		}

		inline json Save(glm::ivec3 const& val) {
			return Save<3, int>(val);
		}

		inline json Save(glm::ivec2 const& val) {
			return Save<2, int>(val);
		}

		inline json Save(glm::dvec2 const& val) {
			return Save<2, double>(val);
		}

		template<typename T>
		inline bool LoadNamed(json const& j, ErrorStack& er, int stackLevel, string const& name, T& val) {
			if (j.find(name) == j.end()) {
				er.emplace_back(stackLevel, "Can't find parameter named \"" + name + "\"");
				return false;
			}
			return Load<T>(j[name], er, stackLevel, val);
		}
		
		template<typename T>
		inline bool Load(json const& j, ErrorStack& er, int stackLevel, DiscreteAABB2<int>& val) {
			ivec2 curVal;
			if (!LoadNamed<ivec2>(j, er, stackLevel + 1, "min", curVal)) {
				return false;
			}
			else {
				val.Begin = curVal;
			}
			if (!LoadNamed<ivec2>(j, er, stackLevel + 1, "max", curVal)) {
				return false;
			}
			else {
				val.End = curVal;
			}
			return true;
		}

		template<typename T>
		inline json Save(DiscreteAABB2<T> const& val) {
			return {
				{ "min", Save(val.Begin) },
				{ "max", Save(val.End) }
			};
		}

		struct ParseContext {
			json const& data;
			int stackLevel;
			ErrorStack er;

#define Destore(name) M_LoadNamed<decltype(name)>(#name, name);
			template<typename T>
			inline void M_LoadNamed(string const& name, T& val) {
				LoadNamed<T>(data, er, stackLevel, name, val);
			}

			ParseContext(json const& data);
		};

		struct SaveContex : public json {
#define Store(name) M_SaveNamed(#name, name);
			template<typename T>
			inline void M_SaveNamed(string const& name, T const& val) {
				(*this)[name] = Save(val);
			}
		};
	}
}
