#pragma once

#include <string>
#include <typeinfo>

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#if defined(__GNUC__)
# include <cxxabi.h>
#endif /* __GNUC__ */


namespace Hypodermic
{

    struct TypeInfo
    {
        explicit TypeInfo(const std::type_info& typeInfo)
            : m_typeInfo(&typeInfo)
            , m_fullyQualifiedName(dotNetify(demangleTypeName(m_typeInfo->name())))
        {
        }

        const std::type_info& intrinsicTypeInfo() const
        {
            return *m_typeInfo;
        }

        const std::string& fullyQualifiedName() const
        {
            return m_fullyQualifiedName;
        }

        bool operator==(const TypeInfo& rhs) const
        {
            return intrinsicTypeInfo() == rhs.intrinsicTypeInfo();
        }

        static std::string dotNetify(const std::string& typeName)
        {
            return boost::algorithm::replace_all_copy(typeName, "::", ".");
        }

        static std::string demangleTypeName(const std::string& typeName)
        {
#if defined(__GNUC__)
            int status;

            auto demangledName = abi::__cxa_demangle(typeName.c_str(), 0, 0, &status);
            if (demangledName == nullptr)
                return typeName;

            std::string result = demangledName;
            free(demangledName);
            return result;
#else
            std::string demangled = typeName;
            demangled = boost::regex_replace(demangled, boost::regex("(const\\s+|\\s+const)"), std::string());
            demangled = boost::regex_replace(demangled, boost::regex("(volatile\\s+|\\s+volatile)"), std::string());
            demangled = boost::regex_replace(demangled, boost::regex("(static\\s+|\\s+static)"), std::string());
            demangled = boost::regex_replace(demangled, boost::regex("(class\\s+|\\s+class)"), std::string());
            demangled = boost::regex_replace(demangled, boost::regex("(struct\\s+|\\s+struct)"), std::string());
            return demangled;
#endif /* defined(__GNUC__) */
        }

    private:
        const std::type_info* m_typeInfo;
        std::string m_fullyQualifiedName;
    };

namespace Utils
{

    template <class T>
    inline const TypeInfo& getMetaTypeInfo()
    {
        static TypeInfo result(typeid(T));
        return result;
    }

} // namespace Utils
} // namespace Hypodermic


#include <functional>
#include <typeindex>


namespace std
{

    template <>
    class hash< Hypodermic::TypeInfo > : public unary_function< Hypodermic::TypeInfo, size_t >
    {
    public:
        size_t operator()(const Hypodermic::TypeInfo& value) const
        {
            return hash< type_index >()(type_index(value.intrinsicTypeInfo()));
        }
    };

    inline ostream& operator<<(ostream& stream, const Hypodermic::TypeInfo& info)
    {
        return stream
            << "FullyQualifiedName: '" << info.fullyQualifiedName() << "'"
            ;
    }

} // namespace std