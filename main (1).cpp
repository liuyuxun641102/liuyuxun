#include <cmath>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <iomanip>

// ============================================
// 内存管理系统
// ============================================

class MemoryManager {
private:
    struct MemoryBlock {
        void* ptr;
        size_t size;
        std::string file;
        int line;
        bool isArray;
    };

    std::map<void*, MemoryBlock> allocatedMemory;
    size_t totalAllocated;
    size_t peakUsage;
    int allocationCount;
    int deallocationCount;

    static MemoryManager* instance;

    MemoryManager() : totalAllocated(0), peakUsage(0), allocationCount(0), deallocationCount(0) {
        std::cout << "=========================================\n";
        std::cout << "           内存管理系统 v1.0\n";
        std::cout << "          作者:1225\n";
        std::cout << "         功能:内存分配跟踪与泄漏检测\n";
        std::cout << "=========================================\n\n";
    }

public:
    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator=(const MemoryManager&) = delete;

    static MemoryManager& getInstance() {
        if (!instance) {
            instance = new MemoryManager();
        }
        return *instance;
    }

    static void destroyInstance() {
        if (instance) {
            delete instance;
            instance = nullptr;
        }
    }

    void* allocate(size_t size, const char* file = "unknown", int line = 0, bool isArray = false) {
        void* ptr = std::malloc(size);
        if (!ptr) {
            std::cerr << "内存分配失败！大小: " << size << " 字节, 位置: " << file << ":" << line << "\n";
            return nullptr;
        }

        MemoryBlock block;
        block.ptr = ptr;
        block.size = size;
        block.file = file;
        block.line = line;
        block.isArray = isArray;

        allocatedMemory[ptr] = block;
        totalAllocated += size;
        allocationCount++;

        if (totalAllocated > peakUsage) {
            peakUsage = totalAllocated;
        }

        return ptr;
    }

    void deallocate(void* ptr, bool isArray = false) {
        if (!ptr) return;

        auto it = allocatedMemory.find(ptr);
        if (it != allocatedMemory.end()) {
            if (it->second.isArray != isArray) {
                std::cerr << "警告: 内存类型不匹配! 位置: " << it->second.file << ":" << it->second.line << "\n";
            }

            totalAllocated -= it->second.size;
            deallocationCount++;
            allocatedMemory.erase(it);
            std::free(ptr);
        } else {
            std::cerr << "警告: 尝试释放未分配的内存: " << ptr << "\n";
        }
    }

    void checkLeaks() {
        std::cout << "\n内存泄漏检查报告\n";
        std::cout << "=========================================\n";

        if (allocatedMemory.empty()) {
            std::cout << "恭喜！没有检测到内存泄漏！\n";
        } else {
            std::cout << "检测到内存泄漏！\n";
            std::cout << "泄漏块数: " << allocatedMemory.size() << "\n";
            std::cout << "泄漏字节: " << totalAllocated << " 字节\n\n";

            std::cout << "泄漏详情:\n";
            std::cout << std::left << std::setw(18) << "地址"
                      << std::setw(10) << "大小"
                      << std::setw(8) << "类型"
                      << std::setw(30) << "位置" << "\n";
            std::cout << "------------------------------------------------\n";

            for (const auto& pair : allocatedMemory) {
                const MemoryBlock& block = pair.second;
                std::cout << std::left << std::setw(18) << block.ptr
                          << std::setw(10) << block.size
                          << std::setw(8) << (block.isArray ? "数组" : "对象")
                          << std::setw(30) << (block.file + ":" + std::to_string(block.line))
                          << "\n";
            }
        }
        std::cout << "=========================================\n";
    }

    void printStats() {
        std::cout << "\n内存使用统计\n";
        std::cout << "=========================================\n";
        std::cout << "活跃分配块数: " << (allocationCount - deallocationCount) << "\n";
        std::cout << "当前使用内存: " << totalAllocated << " 字节\n";
        std::cout << "峰值使用内存: " << peakUsage << " 字节\n";
        std::cout << "总分配次数: " << allocationCount << "\n";
        std::cout << "总释放次数: " << deallocationCount << "\n";

        if (allocationCount > 0) {
            double leakPercent = (static_cast<double>(allocationCount - deallocationCount) / allocationCount) * 100;
            std::cout << "内存泄漏率: " << std::fixed << std::setprecision(2) << leakPercent << "%\n";
        }

        std::cout << "=========================================\n";
    }

    void printAllocations() {
        std::cout << "\n当前内存分配\n";
        std::cout << "=========================================\n";
        if (allocatedMemory.empty()) {
            std::cout << "当前没有活跃的内存分配\n";
            return;
        }

        int index = 1;
        for (const auto& pair : allocatedMemory) {
            const MemoryBlock& block = pair.second;
            std::cout << index++ << ". 地址: " << block.ptr
                      << ", 大小: " << block.size << " 字节"
                      << ", 位置: " << block.file << ":" << block.line
                      << (block.isArray ? " [数组]" : " [对象]") << "\n";
        }
        std::cout << "=========================================\n";
    }

    ~MemoryManager() {
        if (!allocatedMemory.empty()) {
            std::cout << "\n警告: 内存管理器析构时仍有 " << allocatedMemory.size()
                      << " 块内存未释放!\n";
        }
    }
};

MemoryManager* MemoryManager::instance = nullptr;

// 优化：改为安全的宏，避免干扰STL容器
#define SAFE_NEW(size) MemoryManager::getInstance().allocate(size, __FILE__, __LINE__, false)
#define SAFE_NEW_ARRAY(size) MemoryManager::getInstance().allocate(size, __FILE__, __LINE__, true)
#define SAFE_DELETE(ptr) MemoryManager::getInstance().deallocate(ptr, false)
#define SAFE_DELETE_ARRAY(ptr) MemoryManager::getInstance().deallocate(ptr, true)

// 移除全局new/delete重载，避免与STL容器冲突
// 如果需要全局监控，可以使用下面的替代方案
// 但会大幅降低性能

// ============================================
// 计算器核心算法
// ============================================

int check(const std::vector<int>& a, const std::vector<int>& b)
{
    if (a.size() > b.size()) return -1;
    if (a.size() < b.size()) return 1;
    for (int i = a.size() - 1; i >= 0; i--)
    {
        if (a[i] > b[i]) return -1;
        if (a[i] < b[i]) return 1;
    }
    return 0;
}

void print(const std::vector<int>& a, bool b, bool c)
{
    if (a.empty()) {
        std::cout << "0";
        if (c) std::cout << "\n\n";
        return;
    }

    if (a.back() == -1 && !a.empty()) {
        std::vector<int> temp = a;
        temp.pop_back();
        std::cout << '-';
        print(temp, b, false);
        if (c) std::cout << "\n\n";
        return;
    }

    if (b)
    {
        for (int i = a.size() - 1; i >= 0; i--)
        {
            std::cout << a[i];
        }
    }
    else
    {
        for (int i = 0; i < a.size(); i++)
        {
            std::cout << a[i];
        }
    }

    if (c) std::cout << "\n\n";
    std::cout.flush();
}

std::vector<int> jia(const std::vector<int>& a, const std::vector<int>& b)
{
    std::vector<int> c(std::max(a.size(), b.size()) + 1, 0);
    for (int i = 0; i < std::max(a.size(), b.size()); i++)
    {
        int r = 0;
        if (i < a.size()) r += a[i];
        if (i < b.size()) r += b[i];
        c[i] += r;
        if (c[i] > 9)
        {
            c[i + 1] += c[i] / 10;
            c[i] %= 10;
        }
    }
    while (c.back() == 0 && c.size() > 1) c.pop_back();
    return c;
}

std::vector<int> jian(const std::vector<int>& a, const std::vector<int>& b)
{
    bool flag = 0;
    std::vector<int> aa = a, bb = b;
    int cmp = check(aa, bb);
    if (cmp == 1) {
        std::swap(aa, bb);
        flag = 1;
    } else if (cmp == 0) {
        return {0};
    }

    std::vector<int> c(aa.size(), 0);
    for (int i = 0; i < aa.size(); i++)
    {
        int res = aa[i];
        if (i < bb.size()) res -= bb[i];
        c[i] += res;
        if (c[i] < 0)
        {
            c[i + 1]--;
            c[i] += 10;
        }
    }
    while (c.back() == 0 && c.size() > 1) c.pop_back();
    if (flag == 1 && !(c.size() == 1 && c[0] == 0)) c.push_back(-1);
    return c;
}

std::vector<int> cheng(const std::vector<int>& a, const std::vector<int>& b)
{
    std::vector<int> c(a.size() + b.size(), 0);
    for (int i = 0; i < a.size(); i++)
    {
        for (int j = 0; j < b.size(); j++)
        {
            c[i + j] += a[i] * b[j];
        }
    }
    for (int i = 0; i < c.size() - 1; i++)
    {
        c[i + 1] += c[i] / 10;
        c[i] %= 10;
    }
    while (c.size() > 1 && c.back() == 0) c.pop_back();
    return c;
}

// 快速幂算法
std::vector<int> quick_mi(const std::vector<int>& base, int exp) {
    if (exp == 0) return {1};
    if (exp == 1) return base;

    std::vector<int> half = quick_mi(base, exp / 2);
    std::vector<int> result = cheng(half, half);

    if (exp % 2 == 1) {
        result = cheng(result, base);
    }

    return result;
}

std::vector<int> mi_optimized(const std::vector<int>& a, const std::vector<int>& b)
{
    if (b.size() == 1 && b[0] == 0) {
        return {1};
    }

    if (a.size() == 1 && a[0] == 0) {
        return {0};
    }

    if (b.size() == 1 && b[0] == 1) {
        return a;
    }

    int exp = 0;
    for (int i = b.size() - 1; i >= 0; i--) {
        exp = exp * 10 + b[i];
        if (exp > 1000000) {
            std::cout << "错误：指数太大，无法计算！\n";
            return {0};
        }
    }

    if (exp > 1000) {
        std::cout << "警告：指数为 " << exp << "，计算可能需要一些时间...\n";
    }

    return quick_mi(a, exp);
}

std::pair<std::vector<int>, std::vector<int> > chu(const std::vector<int>& a, const std::vector<int>& b)
{
    if (b[0] == 0 && b.size() == 1) {
        std::cout << "错误：除数不能为0！\n";
        return {{0}, {0}};
    }
    if (a[0] == 0 && a.size() == 1) return {{0}, {0}};
    int cmp = check(a, b);
    if (cmp == 1) return {{0}, a};
    if (cmp == 0) return {{1}, {0}};

    std::vector<int> q(a.size(), 0);
    std::vector<int> res;
    res.push_back(0);

    for (int i = a.size() - 1; i >= 0; i--)
    {
        res.insert(res.begin(), a[i]);
        while (res.back() == 0 && res.size() > 1) res.pop_back();
        int l = 0, r = 9;
        while (l <= r)
        {
            int mid = (l + r) / 2;
            std::vector<int> m(1, mid);
            std::vector<int> t = cheng(m, b);
            int cmp = check(res, t);
            if (cmp == -1 || cmp == 0)
            {
                l = mid + 1;
            }
            else
            {
                r = mid - 1;
            }
        }
        int t = r;
        q[i] = t;
        if (t > 0)
        {
            std::vector<int> pp(1, t);
            std::vector<int> pro = cheng(b, pp);
            res = jian(res, pro);
        }
    }
    while (q.back() == 0 && q.size() > 1) q.pop_back();
    while (res.back() == 0 && res.size() > 1) res.pop_back();
    return {q, res};
}

// ============================================
// 界面函数
// ============================================

void clearScreen() {
    system("cls");
}

void start()
{
    std::cout << "              -=-=-=-=-=-=-=               \n";
    std::cout << "                简易计算器                 \n";
    std::cout << "              -=-=-=-=-=-=-=               \n";
    std::cout << "                                  版本号5.0\n";
    std::cout << "                         作者:liuyuxun,1225\n";
    std::cout << "       集成内存管理系统，更安全稳定        \n";
	std::cout << "             暂只支持整数运算        \n";
    std::cout << "          输入 usage查看使用方法           \n";
    std::cout << "-------------------------------------------\n\n";
}

void usage()
{
    std::cout << "                                           \n";
    std::cout << "使用方法                                   \n";
    std::cout << "###########################################\n";
    std::cout << "# 格式：                                  #\n";
    std::cout << "# 数字1符号数字2                          #\n";
    std::cout << "# 例：1234+5678                           #\n";
    std::cout << "#      /  |  \\                            #\n";
    std::cout << "#  数字1 符号 数字2                       #\n";
    std::cout << "###########################################\n";
    std::cout << "# 支持的运算:                             #\n";
    std::cout << "# +(加法)                         -(减法) #\n";
    std::cout << "# *(乘法)                         /(除法) #\n";
    std::cout << "# ^(幂运算)                               #\n";
    std::cout << "###########################################\n";
    std::cout << "# 指令：                                  #\n";
    std::cout << "# exit                               退出 #\n";
    std::cout << "# log                            更新日志 #\n";
    std::cout << "# clear                          清空屏幕 #\n";
    std::cout << "# usage                          使用方法 #\n";
    std::cout << "# qq                             联系作者 #\n";
    std::cout << "# memory                     查看内存状态 #\n";
    std::cout << "# allocations                查看内存分配 #\n";
	std::cout << "# information                查看作者信息 #\n";
    std::cout << "# test                      内存测试示例  #\n";
    std::cout << "###########################################\n\n";
}

void log()
{
    std::cout << "               =-=-=-=-=-=-                \n";
    std::cout << "                 更新日志                  \n";
    std::cout << "               =-=-=-=-=-=-                \n";
    std::cout << "                                           \n";
    std::cout << "###########################################\n";
    std::cout << "# 版本 5.0 (内存管理增强版)               #\n";
    std::cout << "# 更新时间：2026年2月8日                  #\n";
    std::cout << "# 更新内容：                              #\n";
    std::cout << "# 1.集成完整的内存管理系统                #\n";
    std::cout << "# 2.实时监控内存分配和释放                #\n";
    std::cout << "# 3.自动检测内存泄漏                      #\n";
    std::cout << "# 4.添加内存状态查询指令                  #\n";
    std::cout << "# 5.优化幂运算算法                        #\n";
    std::cout << "# 6.增强错误处理机制                      #\n";
    std::cout << "###########################################\n";
    std::cout << "                                           \n";
    std::cout << "###########################################\n";
    std::cout << "# 版本 4.1 (代码规范版)                   #\n";
    std::cout << "# 更新时间：2026年2月8日                  #\n";
    std::cout << "# 更新内容：                              #\n";
    std::cout << "# 1.完全重写代码结构                      #\n";
    std::cout << "# 2.符合C++社区编码规范                   #\n";
    std::cout << "# 3.优化函数参数传递方式                  #\n";
    std::cout << "# 4.修复已知的代码缺陷                    #\n";
    std::cout << "# 5.统一代码风格和格式                    #\n";
    std::cout << "###########################################\n";
	std::cout << "                                           \n";
	std::cout << "###########################################\n";
	std::cout << "# 版本 4.0 (功能增强版)                   #\n";
	std::cout << "# 更新时间：2026年2月6日                  #\n";
	std::cout << "# 更新内容：                              #\n";
	std::cout << "# 1.添加幂运算功能(^)                     #\n";
	std::cout << "# 2.支持大整数幂计算                      #\n";
	std::cout << "# 3.优化乘法运算效率                      #\n";
	std::cout << "# 4.改进除法算法逻辑                      #\n";
	std::cout << "###########################################\n";
	std::cout << "                                           \n";
	std::cout << "###########################################\n";
	std::cout << "# 版本 3.0 (稳定版)                       #\n";
	std::cout << "# 更新时间：2026年2月5日                  #\n";
	std::cout << "# 更新内容：                              #\n";
	std::cout << "# 1.添加完整的除法运算(/)                 #\n";
	std::cout << "# 2.支持商和余数同时显示                  #\n";
	std::cout << "# 3.实现第二代用户界面                    #\n";
	std::cout << "# 4.增加除零错误处理                      #\n";
	std::cout << "###########################################\n";
	std::cout << "                                           \n";
	std::cout << "###########################################\n";
	std::cout << "# 版本 2.0 (功能扩展版)                   #\n";
	std::cout << "# 更新时间：2026年2月5日                  #\n";
	std::cout << "# 更新内容：                              #\n";
	std::cout << "# 1.添加乘法运算功能(*)                   #\n";
	std::cout << "# 2.实现大整数乘法算法                    #\n";
	std::cout << "# 3.优化向量存储结构                      #\n";
	std::cout << "# 4.改进数字输入方式                      #\n";
	std::cout << "###########################################\n";
	std::cout << "                                           \n";
	std::cout << "###########################################\n";
	std::cout << "# 版本 1.1 (界面改进版)                   #\n";
	std::cout << "# 更新时间：2026年2月4日                  #\n";
	std::cout << "# 更新内容：                              #\n";
	std::cout << "# 1.更改表达式输入方式                    #\n";
	std::cout << "# 2.支持连续数字输入                      #\n";
	std::cout << "# 3.改进结果输出格式                      #\n";
	std::cout << "# 4.优化用户操作流程                      #\n";
	std::cout << "###########################################\n";
	std::cout << "                                           \n";
	std::cout << "###########################################\n";
	std::cout << "# 版本 1.0 (初始发布版)                   #\n";
	std::cout << "# 更新时间：2026年2月4日                  #\n";
	std::cout << "# 更新内容：                              #\n";
	std::cout << "# 1.实现基本加法运算(+)                   #\n";
	std::cout << "# 2.实现基本减法运算(-)                   #\n";
	std::cout << "# 3.支持大整数运算                        #\n";
	std::cout << "# 4.设计第一代用户界面                    #\n";
	std::cout << "###########################################\n";

    std::cout << "\n按Enter键继续...";
    std::cin.ignore();
    std::cin.get();
}

void qq()
{
	printf("作者联系方式:                              \n");
	printf("############################################\n");
	printf("# liuyuxun:                                #\n");
	printf("# qq:2160663365           微信:13263772875 #\n");
	printf("# 邮箱:lyxlele@outlook.com                 #\n");
	printf("# 邮箱（备用）:2160663365@qq.com           #\n");
	printf("############################################\n");
	printf("# 1225:                                    #\n");
	printf("# qq:3788387389                            #\n");
	printf("# 邮箱:eFrisk_Dreemurr@outlook.com         #\n");
	printf("# 邮箱（备用）:3788387389@qq.com           #\n");
	printf("############################################\n");
}

void information()
{
	printf("作者信息:                              \n");
	printf("############################################\n");
	printf("# liuyuxun:                                #\n");
	printf("# 学校:钦州师范学校附属小学                #\n");
	printf("# 简介:                                    #\n");
	printf("# 一个学编程的小学生                       #\n");
	printf("############################################\n");
	printf("# 1225:                                    #\n");
	printf("# 学校:钦州市第九小学                      #\n");
	printf("# 简介:                                    #\n");
	printf("# (暂无简介)                               #\n");
	printf("############################################\n");
} 

// 内存测试示例
void testMemory() {
    std::cout << "\n内存管理系统测试\n";
    std::cout << "=========================================\n";

    MemoryManager& mm = MemoryManager::getInstance();

    // 正常分配释放
    int* p1 = static_cast<int*>(SAFE_NEW(sizeof(int) * 10));
    std::cout << "1. 分配了10个int数组: " << p1 << "\n";

    int* p2 = static_cast<int*>(SAFE_NEW(sizeof(int)));
    std::cout << "2. 分配了1个int: " << p2 << "\n";

    SAFE_DELETE(p2);
    std::cout << "3. 释放了1个int\n";

    mm.printStats();

    // 故意制造泄漏
    int* p3 = static_cast<int*>(SAFE_NEW(sizeof(int) * 5));
    std::cout << "4. 分配了5个int数组(故意不释放): " << p3 << "\n";

    std::cout << "\n测试完成，查看内存状态:\n";
    mm.printStats();

    SAFE_DELETE_ARRAY(p1);
    std::cout << "5. 释放了10个int数组\n";

    std::cout << "\n最终内存状态:\n";
    mm.printStats();

    std::cout << "\n注意: p3指针没有被释放，会被内存管理器检测到泄漏\n";
}

// ============================================
// 主函数
// ============================================

int main()
{
    system("title 简易计算器 v4.2(内存管理版)");

    // 启动内存管理器
    MemoryManager& mm = MemoryManager::getInstance();

    start();

    while (1)
    {
        char op;
        std::string s;
        std::string s1, s2;
        std::cout << "输入表达式或指令: ";
        std::cin >> s;

        if (s == "exit") {
            std::cout << "\n正在退出程序...\n";
            break;
        }
        if (s == "log")
        {
            log();
            clearScreen();
            start();
            continue;
        }
        if (s == "clear")
        {
            clearScreen();
            start();
            continue;
        }
        if (s == "usage")
        {
            usage();
            continue;
        }
        if (s == "qq")
        {
            qq();
            continue;
        }
        if (s == "memory")
        {
            mm.printStats();
            continue;
        }
        if (s == "allocations")
        {
            mm.printAllocations();
            continue;
        }
        if (s == "test")
        {
            testMemory();
            continue;
        }
		if (s == "information")
		{
			information();
			continue;
		}

        int n = -1, m = -1;
        for (size_t i = 0; i < s.size(); i++)
        {
            if (!isdigit(s[i]))
            {
                op = s[i];
                n = i;
                m = i + 1;
                break;
            }
        }

        if (n == -1) {
            std::cout << "错误：无效的表达式！\n";
            continue;
        }

        s1 = s.substr(0, n);
        s2 = s.substr(m);

        if (s1.empty() || s2.empty()) {
            std::cout << "错误：数字不能为空！\n";
            continue;
        }

        std::vector<int> a, b;

        for (int i = s1.size() - 1; i >= 0; i--) a.push_back(s1[i] - '0');
        for (int i = s2.size() - 1; i >= 0; i--) b.push_back(s2[i] - '0');

        std::cout << '=';

        if (op == '+')
        {
            std::vector<int> c = jia(a, b);
            print(c, 1, 1);
        }
        else if (op == '-')
        {
            std::vector<int> c = jian(a, b);
            print(c, 1, 1);
        }
        else if (op == '*')
        {
            std::vector<int> c = cheng(a, b);
            print(c, 1, 1);
        }
        else if (op == '/')
        {
            auto res = chu(a, b);
            std::vector<int> c = res.first;
            std::vector<int> d = res.second;
            print(c, 1, 0);
            std::cout << "......";
            print(d, 1, 1);
        }
        else if (op == '^')
        {
            std::vector<int> c = mi_optimized(a, b);
            print(c, 1, 1);
        }
        else
        {
            std::cout << "错误：不支持的操作符 '" << op << "'\n";
        }
    }

    std::cout << "\n=========================================\n";
    std::cout << "程序执行完成，开始内存泄漏检查...\n";
    mm.checkLeaks();

    MemoryManager::destroyInstance();

    std::cout << "\n感谢使用简易计算器 v5.0！\n";
    std::cout << "按Enter键退出...";
    std::cin.ignore();
    std::cin.get();

    return 0;
}
