#include "CommonManager.h"
#include "ScanManager.h"
#include "DataManager.h"
#include "SqliteManager.h"
#include "pinyin.h"
#include "HighLight.h"
#include <stdio.h>
void TestDirctoryList()
{
	std::vector<std::string> subdirs;
	std::vector<std::string> subfiles;
	DirectoryList("E:", subfiles, subdirs);
	for (const auto &i : subdirs)
	{
		cout << i << endl;
	}
	for (const auto &i : subfiles)
	{
		cout << i << endl;
	}
}

void TestSqlite()//to do ���еĲ���������޸Ĳ���������ƴ��
{
	SqliteManager sq;
	sq.Open("docs.db");
	//std::string createtb_sql = "create table if not exists tb_doc (id INTEGER PRIMARY KEY, path text, doc_name text)";
	//doc_name_pinyin text, doc_name_initials��
	std::string createtb_sql = "create table if not exists tb_doc (id INTEGER PRIMARY KEY, doc_path text, doc_name text,doc_name_pinyin text, doc_name_initials text)";
	sq.ExecuteSql(createtb_sql);


	/*insert_sql = "insert into tb_doc(doc_path, doc_name) values('stl\', 'vector.h')";
	sq.ExecuteSql(insert_sql);
	insert_sql = "insert into tb_doc(doc_path, doc_name) values('stl\', 'list.h')";
	sq.ExecuteSql(insert_sql);
	insert_sql = "insert into tb_doc(doc_path, doc_name) values('stl\', 'deque.h')";
	sq.ExecuteSql(insert_sql);*/

	std::string query_sql = "select * from tb_doc where doc_path = 'stl\'";
	int row, col;
	char** ppRet;
	sq.GetTable(query_sql, row, col, ppRet);
	/*for (int i = 1; i < row; ++i)
	{
	for (int j = 0; j < col; ++j)
	{
	cout << ppRet[i*col + j] <<" ";
	}
	cout << endl;
	}
	*/

	//sqlite3_free_table(ppRet);

	AutoGetTable agt(&sq, query_sql, row, col, ppRet);
	for (int i = 1; i <= row; ++i)
	{
		for (int j = 0; j < col; ++j)
		{
			cout << ppRet[i*col + j] << " ";
		}
		cout << endl;
	}
}

void TestSearch()
{
	//����ɨ���߳�
	ScanManager *inst = ScanManager::CreatInstance();
	std::vector<std::string> localdirs;
	std::vector<std::string> localfiles;
	DirectoryList("E:\\VS2015", localdirs, localfiles);
	int len = localdirs.size();
	std::vector<std::thread> thd;
	thd.resize(len);
	for (auto& subdirs : localdirs)
	{
		std::string subpath = "E:\\VS2015";
		subpath += '\\';
		subpath += subdirs;
		subdirs = subpath;
	}
	DataManager::GetInstance()->BeginTransaction();
	for (int i = 0; i < len; ++i)
	{
		thd[i] = std::thread(&ScanManager::Scan, inst, localdirs[i]);
		thd[i].detach();
	}
	
	DataManager::GetInstance()->Init();
	std::vector<std::pair<std::string, std::string>> docinfos;
	std::string key;
	cout << "====================================" << endl;
	while (std::cin >> key)
	{
		DataManager::GetInstance()->Search(key, docinfos);
		printf("%-50s %-50s\n", "����", "·��");
		std::string prefix = "";
		std::string highlight = "";
		std::string suffix = "";
		for (const auto& e : docinfos)
		{
			//cout << e.first << "  " << e.second << endl;
			DataManager::GetInstance()->SplitHighlight(e.second, key, prefix, highlight, suffix);
			//printf("%-50s %-50s\n", e.second.c_str(), e.first.c_str());
			cout << prefix;
			ColourPrintf(highlight.c_str());
			cout << suffix;
			//cout << endl;
			for (int i = strlen(prefix.c_str()) + strlen(highlight.c_str()) + strlen(suffix.c_str()); i <= 50; ++i)
			{
				printf(" ");
			}
			printf("%-50s\n",e.first.c_str());
		}
		docinfos.clear();
		cout << "====================================" << endl;
	}
	//th.join();
}
void TestPinyin()
{
	std::string allspell = ChineseConvertPinYinAllSpell("ƴ�� ��Ŀ���� pinyin, test ����");
	std::string initials = ChineseConvertPinYinAllSpell("ƴ�� ��Ŀ���� pinyin, test ����");
	cout << allspell << endl;
	cout << initials << endl;
}
//void TestDatamaneger()
//{
//	DataManager dd;
//	std::string path = "hht";
//	std::set<std::string> docs;
//	dd.InsertDoc("hht", "set.h");
//	dd.DeleteDoc("stl", "vector.h");
//	dd.GetDocs(path, docs);
//
//	for (auto &i : docs)
//	{
//		cout << i << endl;
//	}
//}

void TestScanManager()
{
	//ScanManager scanmgr;
	//scanmgr.Scan("D:\\open code");
	///scanmgr.Scan("D:\\dir");l
	//scanmgr.Scan("E:\\QQMusic");
}

void TestHighlight()
{

	// 1.key��ʲô ����key
	{
		std::string key = "��Ŀ����";
		std::string str = "����һ����Ŀ����233";
		size_t pos = str.find(key);
		std::string prefix, suffix;
		prefix = str.substr(0, pos);
		suffix = str.substr(pos + key.size(), std::string::npos);
		cout << prefix;
		ColourPrintf(key.c_str());
		cout << suffix << endl;
	}

	// 2.key��ƴ����������Ӧ�ĺ���
	{
		std::string key = "xiangmu";
		std::string str = "����һ����Ŀ����233";
		std::string prefix, suffix;
		std::string str_py = ChineseConvertPinYinAllSpell(str);
		std::string key_py = ChineseConvertPinYinAllSpell(key);
		size_t pos = str_py.find(key_py);

		if (pos == std::string::npos)
		{
			cout << "ƴ����ƥ��" << endl;
		}
		else
		{
			size_t key_start = pos;
			size_t key_end = pos + key_py.size();

			size_t str_i = 0, py_i = 0;
			while (py_i < key_start)
			{
				char chinese[3] = { '\0' };
				chinese[0] = str[str_i];
				chinese[1] = str[str_i + 1];
				str_i += 2;

				std::string py_str = ChineseConvertPinYinAllSpell(chinese);
				py_i += py_str.size();
			}

			prefix = str.substr(0, str_i);

			size_t str_j = str_i, py_j = py_i;
			while (py_j < key_end)
			{
				char chinese[3] = { '\0' };
				chinese[0] = str[str_j];
				chinese[1] = str[str_j + 1];
				str_j += 2;

				std::string py_str = ChineseConvertPinYinAllSpell(chinese);
				py_j += py_str.size();
			}

			key = str.substr(str_i, str_j - str_i);
			suffix = str.substr(str_j, std::string::npos);

			cout << prefix;
			ColourPrintf(key.c_str());
			cout << suffix << endl;
		}

	}

	// 2.key��ƴ������ĸ��������Ӧ�ĺ���
	{
		std::string key = "xmcs";
		std::string str = "����һ����Ŀ����233";
		std::string prefix, suffix;
	}
}

void TestChinese()
{
	char a[10] = "C����";
	int i;
	for (i = 0; a[i]; i++)
		if (a[i] < 0) {
			printf("Chinese :%c%c\n", a[i], a[i + 1]);
			i++;
		}
		else printf("Not Chinese:%c\n", a[i]);
}

void TestToUpper()
{
	std::string str = "ѧϰstl���ʹ��";
	std::string key = "STL";
	std::string up_str = ToUpper(str);
	std::string up_key = ToUpper(key);
	size_t pos = up_str.find(up_key);
	std::string prefix = str.substr(0, pos);
	std::string suffix = str.substr(pos + up_key.size(), std::string::npos);

	cout << prefix;
	ColourPrintf(key.c_str());
	cout << suffix << endl;
	
}
void TestThings()
{
	//int ret;
	//char *zErrorMsg;
	//ret = sqlite3_exec(db, "begin transaction", 0, 0, &zErrorMsg); // ��ʼһ������
	//ret = sqlite3_exec(db, "commit transaction", 0, 0, &zErrorMsg); // �ύ����
	//ret = sqlite3_exec(db, "begin transaction", 0, 0, &zErrorMsg);
	//for (��)
	//{
	//	//insert into operate
	//	// �����������
	//	ret = sqlite3_exec(db, ��rollback transaction��, 0, 0, &zErrorMsg)
	//}
	//ret = sqlite3_exec(db, ��commit transaction��, 0, 0, &zErrorMsg);
}

void Func(int &i)
{
	static int n = 0;
	cout << n << endl;
	if (n >= 5)
	{
		n = 0;
	}
	if (i > 100)
	{
		return;
	}
	++i;
	++n;
	Func(i);
}
void TestStatic()
{
	int i = 0;
	Func(i);
}
//void test()
//{
//	DataManager _data;
//	printf("%p\n", &_data);
//}
void func()
{
	cout << "this is a test" << endl;
}
void TestThreadPool()
{

	ScanManager::CreatInstance()->Scan("E:");
	//ThreadPool::GetInstance();
	//singleton::initance();
}
int main()
{
	//utf-8����ת����GBK����linux�����в�֧��ƴ������
	//sqlite���̰߳�ȫ�ģ�δ�����дռ�����⣬ʹ�ö�д��(sqlite�̰߳�ȫ���⣩
	//TestDirctoryList();
	//TestSqlite();
	//TestDatamaneger();
	//TestScanManager();
	TestSearch();
	//TestPinyin();
	//TestHighlight();
	//TestChinese();
	//TestToUpper();
	//TestStatic();
	//test();
	//TestThreadPool();

	system("pause");
	return 0;
}