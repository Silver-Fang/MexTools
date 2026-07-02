//这两个顺序不能反，Mex工具会根据是否已定义__MEX_IO_ADAPTER_HPP__来决定要不要定义MexIORange
#include <mexAdapter.hpp>
#include "Mex工具.hpp"

#include <Windows.h>
import std;
#pragma comment(lib,"libMatlabDataArray.lib")
#pragma comment(lib,"libmex.lib")
using namespace matlab::data;
namespace Mex工具
{
	using namespace 内部;
	ArrayFactory 数组工厂;
	std::shared_ptr<matlab::engine::MATLABEngine> MATLAB引擎;
	std::map<void*, std::move_only_function<void(void*)const>>自动析构表;
	template<>
	CharArray 万能转码<CharArray>(Array&& 输入)
	{
		switch (输入.getType())
		{
		case ArrayType::CHAR:
			return std::move(输入);
		case ArrayType::MATLAB_STRING:
			return 数组工厂.createCharArray(reinterpret_cast<const Array&>(输入)[0].operator String());
		case ArrayType::CELL:
			return reinterpret_cast<const Array&>(输入)[0];
		default:
			return 数组工厂.createCharArray(MATLAB引擎->feval(MATLAB转换函数<MATLABString>::value, std::move(输入))[0].operator String());
		}
	}
	template<>
	MATLABString 万能转码<MATLABString>(Array&& 输入)
	{
		switch (输入.getType())
		{
		case ArrayType::CHAR:
			return CharArray(std::move(输入)).toUTF16();
		case ArrayType::MATLAB_STRING:
			return reinterpret_cast<const Array&>(输入)[0].operator String();
		case ArrayType::CELL:
			return reinterpret_cast<const Array&>(输入)[0].operator CharArray().toUTF16();
		default:
			return MATLAB引擎->feval(MATLAB转换函数<MATLABString>::value, std::move(输入))[0].operator String();
		}
	}
	template<>
	String 万能转码<String>(Array&& 输入)
	{
		switch (输入.getType())
		{
		case ArrayType::CHAR:
			return CharArray(std::move(输入)).toUTF16();
		case ArrayType::MATLAB_STRING:
			return reinterpret_cast<const Array&>(输入)[0];
		case ArrayType::CELL:
			return reinterpret_cast<const Array&>(输入)[0].operator CharArray().toUTF16();
		default:
			return MATLAB引擎->feval(MATLAB转换函数<MATLABString>::value, std::move(输入))[0];
		}
	}
	template<>
	std::string 万能转码<std::string>(Array&& 输入)
	{
		std::string 输出;
		switch (输入.getType())
		{
		case ArrayType::CHAR:
			if (const int 长度 = 输入.getNumberOfElements())
				输出.resize_and_overwrite((长度 + 1) * 3, [宽指针 = reinterpret_cast<wchar_t*>(CharArray(std::move(输入)).release().get()), 长度](char* 指针, size_t 尺寸)
					{
						return WideCharToMultiByte(CP_UTF8, 0, 宽指针, 长度, 指针, 尺寸, nullptr, nullptr);
					});
			break;
		case ArrayType::MATLAB_STRING:
		{
			const String 字符串 = reinterpret_cast<const Array&>(输入)[0];
			if (字符串.size())
				输出.resize_and_overwrite((字符串.size() + 1) * 3, [&字符串](char* 指针, size_t 尺寸)
					{
						return WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<const wchar_t*>(字符串.data()), 字符串.size(), 指针, 尺寸, nullptr, nullptr);
					});
		}
		break;
		case ArrayType::CELL:
		{
			CharArray 字符数组 = reinterpret_cast<const Array&>(输入)[0];
			if (const int 长度 = 字符数组.getNumberOfElements())
				输出.resize_and_overwrite((长度 + 1) * 3, [宽指针 = reinterpret_cast<wchar_t*>(字符数组.release().get()), 长度](char* 指针, size_t 尺寸)
					{
						return WideCharToMultiByte(CP_UTF8, 0, 宽指针, 长度, 指针, 尺寸, nullptr, nullptr);
					});
		}
		break;
		default:
			const String 字符串 = MATLAB引擎->feval(MATLAB转换函数<MATLABString>::value, std::move(输入))[0];
			if (字符串.size())
				输出.resize_and_overwrite((字符串.size() + 1) * 3, [&字符串](char* 指针, size_t 尺寸)
					{
						return WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<const wchar_t*>(字符串.data()), 字符串.size(), 指针, 尺寸, nullptr, nullptr);
					});
			break;
		}
		return 输出;
	}
	template<>
	StringArray 万能转码<StringArray>(Array&& 输入)
	{
		switch (输入.getType())
		{
		case ArrayType::CHAR:
		{
			StringArray 输出 = 数组工厂.createArray<MATLABString>({ 1 });
			输出[0] = CharArray(std::move(输入)).toUTF16();
			return 输出;
		}
		case ArrayType::MATLAB_STRING:
			return std::move(输入);
		case ArrayType::CELL:
		{
			StringArray 输出 = 数组工厂.createArray<MATLABString>(输入.getDimensions());
			const size_t 元素个数 = 输入.getNumberOfElements();
			for (size_t a = 0; a < 元素个数; ++a)
				输出[a] = reinterpret_cast<const Array&>(输入)[a].operator CharArray().toUTF16();
			return 输出;
		}
		default:
			return MATLAB引擎->feval(MATLAB转换函数<MATLABString>::value, std::move(输入));
		}
	}
	template<>
	std::wstring 万能转码<std::wstring>(Array&& 输入)
	{
		switch (输入.getType())
		{
		case ArrayType::CHAR:
		{
			const size_t 长度 = 输入.getNumberOfElements();
			return std::wstring(reinterpret_cast<wchar_t*>(CharArray(std::move(输入)).release().get()), 长度);
		}
		case ArrayType::MATLAB_STRING:
		{
			const String 字符串 = reinterpret_cast<const Array&>(输入)[0];
			return std::wstring(reinterpret_cast<const wchar_t*>(字符串.data()), 字符串.size());
		}
		case ArrayType::CELL:
		{
			CharArray 字符数组 = reinterpret_cast<const Array&>(输入)[0];
			const size_t 长度 = 字符数组.getNumberOfElements();
			return std::wstring(reinterpret_cast<wchar_t*>(字符数组.release().get()), 长度);
		}
		default:
		{
			const String 字符串 = MATLAB引擎->feval(MATLAB转换函数<MATLABString>::value, std::move(输入))[0];
			return std::wstring(reinterpret_cast<const wchar_t*>(字符串.data()), 字符串.size());
		}
		}
	}
	void 自动析构(void* 指针, std::move_only_function<void(void*)const>&& 删除器)noexcept
	{
		自动析构表[指针] = std::move(删除器);
	}
	bool 手动析构(void* 指针)noexcept
	{
		return 自动析构表.erase(指针);
	}
	bool 对象存在(void* 指针)noexcept
	{
		return 自动析构表.contains(指针);
	}
	namespace 内部
	{
		CharArray 标量转换<CharArray>::转换(const char* 输入, size_t 长度)
		{
			matlab::data::buffer_ptr_t<char16_t> 缓冲 = 数组工厂.createBuffer<char16_t>(长度 + 1);
			长度 = MultiByteToWideChar(CP_UTF8, 0, 输入, 长度, reinterpret_cast<wchar_t*>(缓冲.get()), 长度 + 1);
			return 数组工厂.createArrayFromBuffer({ 1,长度 }, std::move(缓冲));
		}
		String 标量转换<String>::转换(const char* 输入, size_t 长度)
		{
			String 返回;
			返回.resize_and_overwrite(长度 + 1, [输入, 长度](char16_t* 指针, size_t 尺寸)
				{
					return MultiByteToWideChar(CP_UTF8, 0, 输入, 长度, reinterpret_cast<wchar_t*>(指针), 尺寸);
				});
			return 返回;
		}
		int WCTMB(const wchar_t* 宽字符串, int 宽字符数, char* 字节缓冲, int 缓冲长度)
		{
			return WideCharToMultiByte(CP_UTF8, 0, 宽字符串, 宽字符数, 字节缓冲, 缓冲长度, nullptr, nullptr);
		}
		const std::u16string MATLAB转换函数<bool>::value = u"logical";
		const std::u16string MATLAB转换函数<char16_t>::value = u"char";
		const std::u16string MATLAB转换函数<MATLABString>::value = u"string";
		const std::u16string MATLAB转换函数<double>::value = u"double";
		const std::u16string MATLAB转换函数<float>::value = u"single";
		const std::u16string MATLAB转换函数<int8_t>::value = u"int8";
		const std::u16string MATLAB转换函数<int16_t>::value = u"int16";
		const std::u16string MATLAB转换函数<int32_t>::value = u"int32";
		const std::u16string MATLAB转换函数<int64_t>::value = u"int64";
		const std::u16string MATLAB转换函数<uint8_t>::value = u"uint8";
		const std::u16string MATLAB转换函数<uint16_t>::value = u"uint16";
		const std::u16string MATLAB转换函数<uint32_t>::value = u"uint32";
		const std::u16string MATLAB转换函数<uint64_t>::value = u"uint64";
	}
	std::unique_ptr<char16_t[], void* (*)(void*)> WindowsErrorMessage(uint32_t 错误码)noexcept
	{
		constexpr DWORD 语言ID = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
		LPWSTR 错误信息;
		if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, 错误码, 语言ID, reinterpret_cast<LPWSTR>(&错误信息), 0, nullptr))
			return {reinterpret_cast<char16_t*>(错误信息), LocalFree};
		if (HRESULT_FACILITY(static_cast<HRESULT>(错误码)) == FACILITY_WIN32)
			if (DWORD const Win32错误码 = HRESULT_CODE(static_cast<HRESULT>(错误码)))
				if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, Win32错误码, 语言ID, reinterpret_cast<LPWSTR>(&错误信息), 0, nullptr))
					return {reinterpret_cast<char16_t*>(错误信息), LocalFree};
		static HMODULE const Ntdll = GetModuleHandleW(L"ntdll.dll");
		if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE, Ntdll, 错误码, 语言ID, reinterpret_cast<LPWSTR>(&错误信息), 0, nullptr))
			return {reinterpret_cast<char16_t*>(错误信息), LocalFree};
		if (错误码 & FACILITY_NT_BIT && FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE, Ntdll, 错误码 & ~FACILITY_NT_BIT, 语言ID, reinterpret_cast<LPWSTR>(&错误信息), 0, nullptr))
			return {reinterpret_cast<char16_t*>(错误信息), LocalFree};
		错误信息 = static_cast<LPWSTR>(LocalAlloc(0, 11 * sizeof(wchar_t)));
		swprintf(错误信息, L"0x%08X", 错误码);
		return {reinterpret_cast<char16_t*>(错误信息), LocalFree};
	}
	std::unique_ptr<char16_t[], void* (*)(void*)> WindowsErrorMessage()noexcept
	{
		return WindowsErrorMessage(GetLastError());
	}
	template<typename T>
	struct 动态类型缓冲模板
	{
		[[noreturn]] static 动态类型缓冲* value(size_t 元素数)
		{
			EnumThrow(MexTools::Unsupported_type, typeid(T).name());
		}
	};
	template<typename T>
		requires requires{buffer_ptr_t<T>(数组工厂.createBuffer<T>(1)); }
	struct 动态类型缓冲模板<T> :动态类型缓冲
	{
		buffer_ptr_t<T>静态类型缓冲;
		void* get()const noexcept override
		{
			return 静态类型缓冲.get();
		}
		static 动态类型缓冲* value(size_t 元素数)
		{
			return new 动态类型缓冲模板(数组工厂.createBuffer<T>(元素数));
		}
		static 动态类型缓冲* value(Array&& 数组)
		{
			return new 动态类型缓冲模板(TypedArray<T>(std::move(数组)).release());
		}
		Array 创建数组(ArrayDimensions&& 各维尺寸)noexcept override
		{
			return 数组工厂.createArrayFromBuffer(std::move(各维尺寸), std::move(静态类型缓冲));
		}
		动态类型缓冲模板(buffer_ptr_t<T>&& 静态类型缓冲) :静态类型缓冲(std::move(静态类型缓冲)) {}
	};
	std::unique_ptr<动态类型缓冲>动态类型缓冲::创建(matlab::data::ArrayType 类型, size_t 元素数)
	{
		return std::unique_ptr<动态类型缓冲>(动态类型选择模板<动态类型缓冲模板>(类型, 元素数));
	}
	std::unique_ptr<动态类型缓冲>动态类型缓冲::创建(matlab::data::Array&&数组)
	{
		return std::unique_ptr<动态类型缓冲>(动态类型选择模板<动态类型缓冲模板>(数组.getType(), std::move(数组)));
	}
}
using namespace Mex工具;
using namespace matlab::mex;
[[noreturn]]static inline void SEH处理(DWORD 错误代码)
{
	throw matlab::engine::MATLABException("MexTools:Unexpected_SEH_exception", WindowsErrorMessage(错误代码).get());
}
static void SEH安全(const std::move_only_function<void()const>& 函数)
{
	__try
	{
		函数();
	}
	__except (GetExceptionCode() != 0xE06D7363)//魔数，表示标准C++异常，不在此处理
	{
		SEH处理(GetExceptionCode());
	}
}
static void 安全调用(const std::move_only_function<void()const>& 函数)
{
	try
	{
		SEH安全(函数);
	}
	catch (const std::exception&)
	{
		throw;
	}
	catch (...)
	{
		EnumThrow(MexTools::Unexpected_CPP_exception);
	}
}
MexFunction::MexFunction()
{
	MATLAB引擎 = getEngine();
	安全调用(初始化);
}
void MexFunction::operator()(ArgumentList outputs, ArgumentList inputs)
{
	安全调用([&outputs, &inputs]() {执行(outputs, inputs); });
}
MexFunction::~MexFunction()
{
	安全调用([]()
		{
			清理();
			for (auto& [指针, 删除器] : 自动析构表)
				删除器(指针);
		});
}
constexpr void* volatile 导出函数[] = { mexCreateMexFunction,mexDestroyMexFunction,mexFunctionAdapter };