#include <utility>
#include "test.h"
#include <string.h>
#include <algorithm>
#include <functional>
#include <map>
void invert_index(std::vector<MyPage>& page_vec, MYHASHMAP::hash_map<std::string, std::vector<std::pair<int,int> > ,MyHashFn>& index_map)
{
	int index ;
	for(index = 0 ; index != page_vec.size(); index ++)
	{
		MYHASHMAP::hash_map<std::string, int ,MyHashFn>::iterator iter ;
		for(iter = ( page_vec[index].m_wordmap).begin() ; iter != page_vec[index].m_wordmap.end(); iter ++ )
		{
			index_map[iter -> first].push_back(std::make_pair(atoi(page_vec[index].m_docid.c_str()),iter -> second  ) );
		}
	}
}

static void remove_dup2(std::vector<MyPage>& page_vec)
{
	int indexi , indexj ;
	for(indexi = 0 ; indexi < page_vec.size() - 1; indexi ++)
	{
		for(indexj = indexi + 1 ; indexj < page_vec.size(); indexj ++)
		{
			if(page_vec[indexi] == page_vec[indexj])//运算符重载
			{
				//删除页面
				page_vec[indexj] = page_vec[page_vec.size() - 1] ;
				page_vec.pop_back() ;
				//该位置要重新比较
				indexj -- ;
			}
		}
	}
}
int  main()
{
	MyConf conf("./conf/my.conf");
	std::cout << "open /conf/my.conf " << std::endl ;
	MySplit asplit(conf) ;
	std::vector<MyPage> page_vec ;
	//打开网页库
	FILE * fp_ripepage = fopen(conf.m_conf["myripepage"].c_str(), "r");
	if(fp_ripepage == NULL)
	{
		std::cout << "open ripepage fail " << std::endl ;
		exit(0);
	}
	std::cout << "begin read ripepage" << std::endl ;
	int i = 1 ;
	while( i  <= 810)
	{
		std::cout << conf.m_offset[i].second + 1 << std::endl;//页面大小
		char * txt = new char[conf.m_offset[i].second + 1]() ;
		fread(txt, 1, conf.m_offset[i].second  , fp_ripepage);
		std::cout << i << std::endl ;
		std::cout << txt << std::endl ;
		MyPage apage(txt, conf, asplit);
		page_vec.push_back(apage);
		i ++  ;
		delete [] txt ;
	}
	fclose(fp_ripepage);
	std::cout <<" before page size : " <<page_vec.size() << std::endl ;
	//网页去重
	remove_dup2(page_vec);
	std::cout <<" after page size: " << page_vec.size() << std::endl ;
	

    int page_index ;
	//          词语                         网页ID   词频     
	MYHASHMAP::hash_map<std::string, std::vector<std::pair<int, int> > ,MyHashFn> index_map ;
	invert_index(page_vec, index_map);	
	
	//建立新偏移文件
	std::ofstream fout(conf.m_conf["mynewoffset"].c_str());
	if(!fout)
	{
		std::cout << "open mynewoffset fail " << std::endl ;
		exit(0);
	}
	for(page_index = 0 ; page_index != page_vec.size(); page_index ++ )
	{
	  fout << atoi(page_vec[page_index].m_docid.c_str()) <<"   "<<conf.m_offset[atoi(page_vec[page_index].m_docid.c_str()) ].first <<"   " <<conf.m_offset[atoi(page_vec[page_index].m_docid.c_str()) ].second << std::endl ;	
	}
	fout.close();
	fout.clear();
	
	
	//建立倒排索引
	fout.open(conf.m_conf["myinvertindex"].c_str());
	if( !fout )
	{
		std::cout <<"open myinvertindex fail" << std::endl ;
		exit(0);
	}
        MYHASHMAP::hash_map<std::string, std::vector<std::pair<int, int> > ,MyHashFn>::iterator iter_invert ;
	MYHASHMAP::hash_map<int, double> sum_map ;
	for(iter_invert = index_map.begin() ; iter_invert != index_map.end(); iter_invert ++)
	{
		int df = (iter_invert -> second).size();//文档频数
		int page_num = page_vec.size();  //总文档数
		//IDF_k  = log2(⁡N/DF_k   +0.05 )
		double idf = log((double)page_num/df + 0.05 )/log(2) ; //逆文档频率
		std::vector<std::pair<int, int> >::iterator iter_vec ;
		//计算 √(∑权值𝑖^2 )
		for(iter_vec = (iter_invert -> second ).begin() ; iter_vec != (iter_invert ->second).end(); iter_vec ++)
		{
		  sum_map[iter_vec -> first] += pow( (iter_vec -> second)*idf , 2 );
		}

	}
	
//write invert index
	for(iter_invert = index_map.begin() ; iter_invert != index_map.end(); iter_invert ++)
	{
		fout << iter_invert -> first <<"   " ;

		int df = (iter_invert -> second).size();
		int page_num = page_vec.size();
		double idf = log( (double)page_num/df + 0.05 )/log(2) ;
		std::vector<std::pair<int, int> >::iterator iter_vec ;
		for(iter_vec = (iter_invert -> second ).begin() ; iter_vec != (iter_invert ->second).end(); iter_vec ++)
		{
			//权值_k  = TF_k * IDF_k
			//重新定义权值_k = 权值_k /  √(∑权值𝑖^2 )
			double word_w = ( (iter_vec -> second)*idf / sqrt( ( sum_map[iter_vec -> first] ) ) ) ;
		   fout << 	iter_vec -> first <<"  " << iter_vec -> second <<"  " << word_w<<"   ";
		}
		fout << std::endl ;
	}
	fout.close();
	std::cout << "index_invert over !" << std::endl ;
	return 0 ;

}

