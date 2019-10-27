// Mahjong 样例程序
// 贪(wu)心(nao)策略
// 作者：zzh_from_ytyz

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <set>
#include<cmath>
#include "jsoncpp/json.h"

using namespace std;

///RS
#define INF 1000000000
#define max(a,b) (a > b ? a : b)
#define min(a,b) (a < b ? a : b)
#define F(t) for(int myCount = 0; myCount < t; myCount++)
#define M(t) memset(t, 0, sizeof(t))
#define W(t) while(t--)

bool check_Peng(pair<int,int> card);
string TingPai();
///RE

int myID, cardCount, nonFrozenCount;
string cards[108];
///RS
struct aCard  //一张牌的数字信息
{
	int type;
	int num;
	aCard() {}
	aCard(int type_, int num_) { type = type_; num = num_; }
	bool operator==(const aCard & p)const
	{
		return type == p.type && num == p.num;
	}
};

struct keepingCard   //手中某张牌的价值（不包括“钦定牌”和“我的明牌”）
{
	double val;
	int type;
	int num;
	keepingCard() { val = 0.0; }
	keepingCard(int type_, int num_) { type = type_; num = num_; val = 0.0; }
	bool operator < (const keepingCard &p)const
	{
		return val < p.val;
	}
};

typedef set<keepingCard> sset;
sset list_of_cards;
map<char, int> cTypeMap;
///RE

// 对牌排序，获得可打出牌的数量
void SortAndGetFrozenCount()
{
	sort(cards, cards + cardCount);
	for (int i = 0; i < cardCount; i++)
	{
		if (cards[i][0] >= 'a')
		{
			nonFrozenCount = i;
			return;
		}
	}
	nonFrozenCount = cardCount;
}

// 递归检查能否胡
bool CheckHu(int num[10])
{
	for (int i = 1; i <= 9; i++)
		if (num[i] < 0)
			return false;
	if (num[0] == 0)
		return true;
	if (num[0] == 1)
		return false;
	if (num[0] == 2)
	{
		// 剩下两张将牌
		for (int i = 1; i <= 9; i++)
			if (num[i] == 2)
				return true;
		return false;
	}

	for (int i = 1; i <= 9; i++)
	{
		if (num[i] > 0)
		{
			if (i <= 7)
			{
				// ABC型句子
				num[i]--, num[i + 1]--, num[i + 2]--;
				num[0] = num[0] - 3;
				if (CheckHu(num))
					return true;
				num[0] = num[0] + 3;
				num[i]++, num[i + 1]++, num[i + 2]++;
			}
			if (num[i] >= 3)
			{
				// AAA型句子
				num[i] = num[i] - 3;
				num[0] = num[0] - 3;
				if (CheckHu(num))
					return true;
				num[0] = num[0] + 3;
				num[i] = num[i] + 3;
			}
		}
	}

	return false;
}

// 判断能不能胡（未考虑碰过故意不杠的情况）
bool Hu()
{
	SortAndGetFrozenCount();
	if (cardCount < 14 || cardCount > 18)
		return false;
	if ((nonFrozenCount - 2) % 3 != 0)
		return false;
	int num[3][10] = { 0 }; // 顺序：万、筒、条，下标为0的项用于记录总数
	for (int i = 0; i < nonFrozenCount; i++)
	{
		// 没亮出来的手牌
		if (cards[i][0] == 'W')
		{
			num[0][cards[i][1] - '0']++;
			num[0][0]++;
		}
		if (cards[i][0] == 'B')
		{
			num[1][cards[i][1] - '0']++;
			num[1][0]++;
		}
		if (cards[i][0] == 'T')
		{
			num[2][cards[i][1] - '0']++;
			num[2][0]++;
		}
	}
	return CheckHu(num[0]) && CheckHu(num[1]) && CheckHu(num[2]);
}


int remain_card[3][10];      //我所不知道的牌（牌堆里&其他人手上）
int thrown_card[3][10];
int otherFrozen_card[3][10];    //其他人的明牌
int myFrozen_card[3][10];       //“我的明牌”
int myPointed_card[3][10];     //“钦定牌”：我凑出来的句子（暗置），直到比赛结束，除非别人给我杠，我是不会动他们的
keepingCard cVal[3][10];   //每张牌的信息和价值

aCard changeExpression(string c)  //把字符串转换为含数字的结构体
{
	aCard p;
	p.type = cTypeMap[c[0]];
	p.num = c[1] - '0';
	return p;
}

double val_by_remain(int r) //根据剩余牌数，判定此牌能被抽到的概率
{
	switch (r)
	{
	case 0:return 0.0;
	case 1:return 1.0;
	case 2:return 2.0;
	case 3:return 3.0;
	case 4:return 4.0;
	default:
		return 0;
	}
}

void checkPointed()  //将句子从inHandCard中除去，表示这些牌（句子）是绝无可能被打出的（至少在本回合）
{
	aCard c;
	int i, j;
	M(myPointed_card);
	int inHandCard[3][10] = { 0 };      //不包括“钦定牌”和“我的明牌”
	for (i = 0; i < cardCount; i++)
		if (cards[i][0] < 'a')
		{
			c = changeExpression(cards[i]);
			inHandCard[c.type][c.num]++;
		}
	//for (i = 0; i < 3; i++)
	//	for (j = 1; j <= 9; j++)
	//		inHandCard[i][j] -= myPointed_card[i][j];
	for (i = 0; i < 3; i++)
		for (j = 1; j <= 9; j++)
			if (inHandCard[i][j] >= 3)
			{
				myPointed_card[i][j] += 3; ///need revise
				inHandCard[i][j] -= 3;
			}
	for (i = 0; i < 3; i++)
		for (j = 1; j <= 7; j++)
			while (inHandCard[i][j] > 0 && inHandCard[i][j + 1] > 0 && inHandCard[i][j + 2] > 0)
			{
				inHandCard[i][j]--;
				inHandCard[i][j + 1]--;
				inHandCard[i][j + 2]--;
				myPointed_card[i][j]++;
				myPointed_card[i][j + 1]++;
				myPointed_card[i][j + 2]++;
			}
}

void initCVal()       //初始化牌面信息
{
	int i, j;
	for (i = 0; i < 3; i++)
		for (j = 1; j <= 9; j++)
		{
			cVal[i][j].type = i;
			cVal[i][j].num = j;
		}
}

double valueOfTwo(aCard a, aCard b)   //a,b是否差一张牌凑成句子（如a=W1,b=W3），若否，返回-1.0，反之返回这两张牌价值的相反数
{
	double val;
	if (a.type != b.type)return -1.0; //花色都不一样
	int div = abs(a.num - b.num);
	if (div > 2)return -1.0;               //差太远（如W1,W4）
	if (div == 2)
	{
		val = val_by_remain(remain_card[a.type][(a.num + b.num) / 2]);
		if (fabs(val) < 0.0001)return -1.0;                                          //牌库里没有缺的牌了！（如a=W1,b=W3，但不可能摸到W2）
		return val;
	}
	if (div == 1)
	{
		int sml = (a.num + b.num) / 2;
		if (sml == 1)
			val = val_by_remain(remain_card[a.type][3]);
		else
			if (sml == 8)
				val = val_by_remain(remain_card[a.type][7]);
			else
				val = val_by_remain(remain_card[a.type][sml - 1]) + val_by_remain(remain_card[a.type][sml + 2]);
		if (fabs(val) < 0.0001)return -1.0;
		return val;
	}
	if (div == 0)
	{
		val = val_by_remain(remain_card[a.type][a.num]);
		if (fabs(val) < 0.0001)return -1.0;
		return val;
	}
	return -1.0;
}

sset getList()  //计算手中每张牌的价值，罗列出来
{
	sset lst;
	aCard c;
	double val;
	int i, j;
	for (i = 0; i < 3; i++)
		for (j = 1; j <= 9; j++)
			cVal[i][j].val = 0.0;
	int inHandCard[3][10] = { 0 };      //不包括“钦定牌”和“我的明牌”
	for (i = 0; i < cardCount; i++)
		if (cards[i][0] < 'a')
		{
			c = changeExpression(cards[i]);
			inHandCard[c.type][c.num]++;
		}
	for (i = 0; i < 3; i++)
		for (j = 1; j <= 9; j++)
			inHandCard[i][j] -= myPointed_card[i][j];

	int cntCard = 0;
	for (i = 0; i < 3; i++)
		for (j = 1; j <= 9; j++)
			cntCard += inHandCard[i][j];
	if (cntCard == 2)                                //只差将牌
	{
		for (i = 0; i < 3; i++)
			for (j = 1; j <= 9; j++)
				if (inHandCard[i][j] > 0)
				{
					val = val_by_remain(remain_card[i][j]);
					cVal[i][j].val = 2 * val;
					lst.insert(cVal[i][j]);
				}
		return lst;
	}
	else
		if (cntCard == 5)                            //可能听牌：判断是否能听牌。若能，判断打出哪张
		{
			vector<aCard> lft, lft2;
			for (i = 0; i < 3; i++)
				for (j = 1; j <= 9; j++)
					if (inHandCard[i][j] > 0)
					{
						int tp = inHandCard[i][j];
						while (tp--)
							lft.push_back(aCard(i, j));
					}
			for (i = 0; i < 4; i++)                   //判断是否有一对将牌。若有，剔除将牌，算剩下三张牌哪张价值最低
				if (lft[i] == lft[i + 1])
				{
					lft2.clear();
					for (j = 0; j < 5; j++)
						if (j != i && j != i + 1)
							lft2.push_back(lft[j]);
					if (lft2.size() != 3)continue;
					double v1, v2, v3;
					v1 = valueOfTwo(lft2[1], lft2[2]);          //a,b两张牌组合起来（若能组合起来的话）的价值越大，c的价值越小
					v2 = valueOfTwo(lft2[0], lft2[2]);
					v3 = valueOfTwo(lft2[0], lft2[1]);
					if (v1 > 0.0)
					{
						cVal[lft2[0].type][lft2[0].num].val = -v1;
						lst.insert(cVal[lft2[0].type][lft2[0].num]);
					}
					if (v2 > 0.0)
					{
						cVal[lft2[1].type][lft2[1].num].val = -v2;
						lst.insert(cVal[lft2[1].type][lft2[1].num]);
					}
					if (v3 > 0.0)
					{
						cVal[lft2[2].type][lft2[2].num].val = -v3;
						lst.insert(cVal[lft2[2].type][lft2[2].num]);
					}
				}
			if (!lst.empty())                    //lst非空，说明能听牌，返回听牌情况下的lst。反之继续计算，直到计算出lst     
				return lst;
		}

	for (i = 0; i < 3; i++)
		for (j = 1; j <= 9; j++)
		{
			val = val_by_remain(remain_card[i][j]);   //[i][j]越容易被抽到，他附近的牌价值越大

													  //凑123
			if (j <= 7)
			{
				if (inHandCard[i][j + 1] && inHandCard[i][j + 2])
				{
					cVal[i][j + 1].val += 120 * val;
					cVal[i][j + 2].val += 80 * val;
				}
				else
					if (inHandCard[i][j + 1])
						cVal[i][j + 1].val += 10 * val;
					else
						if (inHandCard[i][j + 2])
							cVal[i][j + 2].val += 4 * val;
			}
			if (j >= 2 && j <= 8)
			{
				if (inHandCard[i][j + 1] && inHandCard[i][j - 1])
				{
					cVal[i][j + 1].val += 100 * val;
					cVal[i][j - 1].val += 100 * val;
				}
				else
					if (inHandCard[i][j + 1])
						cVal[i][j + 1].val += 9 * val;
					else
						if (inHandCard[i][j - 1])
							cVal[i][j - 1].val += 9 * val;
			}
			if (j >= 3)
			{
				if (inHandCard[i][j - 2] && inHandCard[i][j - 1])
				{
					cVal[i][j - 2].val += 80 * val;
					cVal[i][j - 1].val += 120 * val;
				}
				else
					if (inHandCard[i][j - 2])
						cVal[i][j - 2].val += 4 * val;
					else
						if (inHandCard[i][j - 1])
							cVal[i][j - 1].val += 10 * val;
			}

			//凑333
			if (inHandCard[i][j] == 2)
				cVal[i][j].val += 320 * val;
			else
				if (inHandCard[i][j] == 1)
					cVal[i][j].val += 2.5 * val;
		}
	for (i = 0; i < 3; i++)
		for (j = 1; j <= 9; j++)
			if (inHandCard[i][j])
				lst.insert(cVal[i][j]);
	return lst;
}

int thisDecideOfMine()         //决策
{
	checkPointed();
	sset valueOfCards = getList();
	keepingCard toThrow = *(valueOfCards.begin());      //最没价值的牌，准备扔
	string t;
	int i;
	switch (toThrow.type)
	{
	case 0:
		t.push_back('W');
		break;
	case 1:
		t.push_back('B');
		break;
	case 2:
		t.push_back('T');
		break;
	default:
		break;
	}
	t.push_back('0' + toThrow.num);
	for (i = 0; i < cardCount; i++)
		if (t == cards[i])
			return i;
	return 0;
}

bool valid(int i)//牌面数字是否合法（没有越界）
{
	if (i<1 || i>9)
		return false;
	else return true;
}

string num2str(int i, int j)
{
	string ans;
	if (i == 0)
		ans.push_back('W');
	else if (i == 1)
		ans.push_back('B');
	else
		ans.push_back('T');
	ans.push_back(j + '0');
	return ans;
}

pair<int, int> str2num(string card)
{
	int j = 0, k = 1;
	if (card[0] == 'W')
		j = 0;
	else if (card[0] == 'B')
		j = 1;
	else if (card[0] == 'T')
		j = 2;
	k = card[1] - '0';
	return make_pair(j, k);
}

string Target()
{
	string tar;

	int cnt = 0;//tar中已经有多少张牌
	int targetcnt = cardCount + 1;//最终的目标牌数应为cardCount+1

	SortAndGetFrozenCount();
	int num[3][10] = { 0 };//数一下每一个花色有多少张牌,顺序：万、筒、条，下标为0的项用于记录总数
	for (int i = 0; i < nonFrozenCount; i++)
	{
		// 没亮出来的手牌
		if (cards[i][0] == 'W')
		{
			num[0][cards[i][1] - '0']++;
			num[0][0]++;
		}
		if (cards[i][0] == 'B')
		{
			num[1][cards[i][1] - '0']++;
			num[1][0]++;
		}
		if (cards[i][0] == 'T')
		{
			num[2][cards[i][1] - '0']++;
			num[2][0]++;
		}
	}

	for (int i = 0; i<cardCount; i++)
	{
		if (cards[i][0] >= 'a')//已经明示的牌，直接放进目标序列中
		{
			string ans = cards[i];
			ans[0] -= ('a' - 'A');//小写变成大写
			tar = tar + ans + " ";
			cnt++;
		}
	}
	for (int i = 0; i<3; i++)
		for (int j = 1; j <= 9; j++)
		{
			if (valid(j + 2) && num[i][j] >= 1 && num[i][j + 1] >= 1 && num[i][j + 2] >= 1)//已经凑成一套的吃牌，直接放进去
			{
				string ans = num2str(i, j) + " " + num2str(i, j + 1) + " " + num2str(i, j + 2) + " ";
				tar = tar + ans;
				num[i][j]--;
				num[i][j + 1]--;
				num[i][j + 2]--;//个数减一，因为还可能用于判断对子
				cnt += 3;
				j--;//一套顺子可能包含了不止一套吃牌，如334455
				continue;
			}
			if (num[i][j] >= 3)//已经凑成一套的碰牌，直接放进去
			{
				string ans = num2str(i, j);
				tar = tar + ans + " " + ans + " " + ans + " ";
				num[i][j] -= 3;
				cnt += 3;
			}
		}

	string Double[10];//记录剩下的所有的两张牌
	string Zhang;
	int ptr = 0;//数组指针
	for (int i = 0; i<3; i++)
		for (int j = 1; j <= 9; j++)
		{
			if (num[i][j] == 2)//找到了一个对子
			{
				//cntdouble++;
				Double[ptr++] = num2str(i, j);
			}
		}
	if (ptr >= 1)
	{
		Zhang = Double[0];//找一对掌
		cnt += 2;
		pair<int, int> card = str2num(Double[0]);
		num[card.first][card.second] -= 2;//牌数减二
	}
	else//缺掌
	{
		for (int i = 0; i<3; i++)
			for (int j = 1; j <= 9; j++)
				if (num[i][j] == 1)
				{
					Zhang = num2str(i, j);//找一个单张的作为掌
					cnt += 2;
					num[i][j]--;
				}
	}

	for (int i = 0; i<3 && cnt + 3 <= targetcnt; i++)//先考虑顺子再考虑对子(顺子机会值更大)
		for (int j = 1; j <= 9 && cnt + 3 <= targetcnt; j++)
		{
			if (valid(j + 1) && num[i][j] == 1 && num[i][j + 1] == 1)//已有两张连续的牌，离顺子还差一张牌
			{
				if (valid(j + 2))
				{
					string ans = num2str(i, j) + " " + num2str(i, j + 1) + " " + num2str(i, j + 2) + " ";
					tar = tar + ans;
				}
				else
				{
					string ans = num2str(i, j - 1) + " " + num2str(i, j) + " " + num2str(i, j + 1) + " ";
					tar = tar + ans;
				}
				num[i][j + 1]--;
				num[i][j]--;//牌数各减一
				cnt += 3;
			}
		}


	for (int i = 1; i<ptr && cnt + 3 <= targetcnt; i++)//剩余的对子全凑成碰牌，作为目标
	{
		string ans = Double[i];
		tar = tar + ans + " " + ans + " " + ans + " ";
		cnt += 3;
		pair<int, int> card = str2num(ans);
		num[card.first][card.second] -= 2;//牌数减二
	}


	for (int i = 0; i<3 && cnt + 3 <= targetcnt; i++)
		for (int j = 1; j <= 9 && cnt + 3 <= targetcnt; j++)
		{
			if (num[i][j]>0)
			{
				string ans = num2str(i, j);
				tar = tar + ans + " " + ans + " " + ans + " ";
				num[i][j] = 0;//牌数清零
			}
		}

	tar = tar + Zhang + " " + Zhang;//把掌加在最后

	return tar;
}


int main()
{
	srand(time(0));

	// 读入JSON
	string str;
	getline(cin, str);
	Json::Reader reader;
	Json::Value input;
	reader.parse(str, input);
	///RS
	initCVal();
	///RE
	// 分析自己收到的输入和自己过往的输出，并恢复状态
	int turnID = input["responses"].size();
	for (int i = 0; i < turnID; i++)
	{
		istringstream in(input["requests"][i].asString()),
			out(input["responses"][i].asString());
		string act;

		int type, who;
		string card, what;
		///RS
		aCard this_card;
		///RE
		// 获得信息类型
		in >> type;
		switch (type)
		{
		case 0:
			///RS
			cTypeMap['W'] = 0;
			cTypeMap['w'] = 0;
			cTypeMap['B'] = 1;
			cTypeMap['b'] = 1;
			cTypeMap['T'] = 2;
			cTypeMap['t'] = 2;
			M(otherFrozen_card);
			M(thrown_card);
			M(myFrozen_card);
			M(myPointed_card);
			for (int myi = 0; myi < 3; myi++)
				for (int myj = 1; myj <= 9; myj++)
					remain_card[myi][myj] = 4;
			///RE
			// 告知编号
			in >> type;
			// response一定是PASS，不用看了
			break;
		case 1:
			// 告知手牌
			for (int j = 0; j < 13; j++)
			{
				in >> cards[j];
				///RS
				this_card = changeExpression(cards[j]);
				remain_card[this_card.type][this_card.num]--;
				///RE
			}
			cardCount = 13;
			// response一定是PASS，不用看了
			break;
		case 2:
			// 摸牌，收入手牌
			in >> cards[cardCount++];
			///RS
			this_card = changeExpression(cards[cardCount - 1]);
			remain_card[this_card.type][this_card.num]--;
			///RE
			// 然后我做了act
			out >> act;
			if (act == "PLAY")
			{
				// 当时我打出了……
				out >> act;
				// ……一张act！
				for (int j = 0; j < cardCount; j++)
				{
					if (cards[j] == act)
					{
						// 去掉这张牌，拿最后一张牌填这个空位
						cards[j] = cards[--cardCount];
						break;
					}
				}
			}
			else if (act == "GANG")
			{
				// 当时我杠了……
				out >> act;
				// 一张act！（act是大写的）
				// 在手牌里把这个牌变为小写（明示）
				for (int j = 0; j < cardCount; j++)
				{
					if (cards[j] == act)
						cards[j][0] += 'a' - 'A'; // 变成小写
				}
			}
			// HU不可能出现
			break;
		case 3:
			// 别人的动作
			in >> who >> what >> card;
			///RS
			this_card = changeExpression(card);
			///RE
			// 不是打牌的话，response一定是PASS，不用看了
			if (what != "PLAY")
				///RE
			{
				if (what == "PENG")
				{
					remain_card[this_card.type][this_card.num] -= 2;
					otherFrozen_card[this_card.type][this_card.num] += 3;
				}
				else
					if (what == "GANG")
					{
						remain_card[this_card.type][this_card.num] -= 3;
						otherFrozen_card[this_card.type][this_card.num] += 4;
					}
				break;
			}
			else
			{
				remain_card[this_card.type][this_card.num]--;
			}
			///break;
			///RS

			// 然后我又做了act
			out >> act;

			if (act == "PENG")
			{
				// 当时我碰牌了
				// 先看看这一回合有没有人胡
				{
					int tmp;
					string act, card2;
					istringstream nextInput(input["requests"][i + 1].asString());
					if (nextInput >> tmp >> tmp >> act >> card2 && act == "HU" && card == card2)
						break;
				}

				// 在手牌里把两张card变为小写（明示）
				int count = 0;
				for (int j = 0; j < cardCount; j++)
				{
					if (cards[j] == card) {
						cards[j][0] += 'a' - 'A'; // 变成小写
						if (++count == 2)
							break;
					}
				}
				// 再把card收入手牌
				card[0] += 'a' - 'A';
				cards[cardCount++] = card;

				// 然后我出了……
				out >> act;
				// ……一张act！
				for (int j = 0; j < cardCount; j++)
				{
					if (cards[j] == act)
					{
						// 去掉这张牌，拿最后一张牌填这个空位
						cards[j] = cards[--cardCount];
						break;
					}
				}
			}
			else if (act == "GANG")
			{
				// 当时我杠牌了
				// 先看看这一回合有没有人胡
				{
					int tmp;
					string act, card;
					istringstream nextInput(input["requests"][i + 1].asString());
					if (nextInput >> tmp >> tmp >> act >> card && act == "HU" && card != "SELF")
						break;
				}

				// 在手牌里把card都变为小写（明示）
				for (int j = 0; j < cardCount; j++)
				{
					if (cards[j] == card)
						cards[j][0] += 'a' - 'A'; // 变成小写
				}
				// 再把card收入手牌
				card[0] += 'a' - 'A';
				cards[cardCount++] = card;
			}
			// HU不可能出现，PASS不用处理
			break;
		case 4:
			// 这种情况不可能出现
			;
		}
	}

	// 看看自己本回合输入
	istringstream in(input["requests"][turnID].asString());
	int type, who;
	string act, card, temp, myAction = "PASS";
	in >> type;
	if (type == 2)
	{
		// 轮到我摸牌
		in >> card;

		// 能不能胡？
		cards[cardCount++] = card;
		if (Hu()) // 注意此时牌已经排序
			myAction = "HU";
		else
		{
			// 能不能杠？
			int count = 0;
			temp = card;
			temp[0] += 'a' - 'A';
			for (int i = 0; i < cardCount; i++)
			{
				if (cards[i] == temp || cards[i] == card)
					count++;
			}
			if (count == 4)//已经有三张一样的牌，加上摸得这张一共四张
			{
				// 杠！
				myAction = "GANG " + card;
			}
			else
			{
				string ans=TingPai();
				if(ans=="hehe")
				{
					int result = thisDecideOfMine();
					myAction = "PLAY " + cards[result];
				}
				else
					myAction = "PLAY " + ans;
			}
		}
	}
	else if (type == 3)
	{
		// 其他玩家……
		in >> who >> act >> card;

		if (act == "PLAY") // 除非别人打牌，否则什么也干不了
		{
			// 先收进来

			// 提示：
			// 如果只能PASS，
			// 手牌也不用恢复，
			// 因为下次会重新计算
			cards[cardCount++] = card;
			// 能不能胡？
			if (Hu()) // 注意此时牌已经排序
				myAction = "HU";
			else
			{
				// 能不能杠/碰？
				int count = 0;
				for (int i = 0; i < cardCount; i++)
				{
					if (cards[i] == card)
						count++;
				}
			
				if (count == 3)
				{
					if(check_Peng(str2num(card)))
					{
						SortAndGetFrozenCount();
						// 碰！然后找一张牌出
						string ans=TingPai();
						if(ans=="hehe")
						{
							int result = thisDecideOfMine();
							myAction = "PENG " + cards[result];
						}
						else
							myAction="PENG "+ans;
					}
				}
			}
		}
	}
	else if (type == 4)
	{
		
		myAction = Target();
		///RE
	}
	// 别的情况我只能PASS

	// 输出决策JSON
	Json::Value ret;

	ret["response"] = myAction;
	ret["data"] = "";

	Json::FastWriter writer;
	cout << writer.write(ret) << endl;
	return 0;
}

bool check_Chi(int i,int* num)//判断这张牌是不是在一套吃牌里边
{
	if(num[i]==0)
		return false;
	if(valid(i-2))
	{
		if(num[i-2]>0&&num[i-1]>0)
			return true;
	}
	if(valid(i+2))
	{
		if(num[i+2]>0&&num[i+1]>0)
			return true;
	}
	if(valid(i+1)&&valid(i-1))
	{
		if(num[i+1]>0&&num[i-1]>0)
			return true;
	}
	return false;
}

bool check_Peng(pair<int,int> card)//判断要不要碰某一张牌，先算一遍优先值(只求这个花色的即可)
{
	int cntdouble=0;//计算有多少套对子
	int num[3][10]={0};//数一下每一个花色有多少张牌,顺序：万、筒、条，下标为0的项用于记录总数
	for (int i = 0; i < nonFrozenCount; i++)
	{
		// 没亮出来的手牌
		if (cards[i][0] == 'W')
		{
			num[0][cards[i][1] - '0']++;
			num[0][0]++;
		}
		if (cards[i][0] == 'B')
		{
			num[1][cards[i][1] - '0']++;
			num[1][0]++;
		}
		if (cards[i][0] == 'T')
		{
			num[2][cards[i][1] - '0']++;
			num[2][0]++;
		}
	}
	
	
	for(int i=0;i<3;i++)
		for(int j=1;j<=9;j++)
		{
			if(i==card.first&&j==card.second)
				num[i][j]--;//他拿进来那一张不要算进来
			if(num[i][j]==2)
				cntdouble++;
		}

	if(cntdouble==1)//唯一的将牌
		return false;
	else
	{
		if(check_Chi(card.second,num[card.first])&& num[card.first][card.second]==3)//12333，碰一波，但是不杠
			return true;
		else if(check_Chi(card.second,num[card.first]) && num[card.first][card.second]<=2)//在一套顺子里并且只有两张，不碰了
			return false;
		else return true;//其余的都碰
	}
}

string TingPai()//rest代表剩余总牌数,返回1代表单叫掌，2代表对倒，3代表胡两边,0代表不能听
{
	string ans="hehe";
	int temp=0;//出这张牌能得到的听牌类型

	int num[3][10]={0};//数一下每一个花色有多少张牌,顺序：万、筒、条，下标为0的项用于记录总数
	for (int i = 0; i < nonFrozenCount; i++)
	{
		// 没亮出来的手牌
		if (cards[i][0] == 'W')
		{
			num[0][cards[i][1] - '0']++;
			num[0][0]++;
		}
		if (cards[i][0] == 'B')
		{
			num[1][cards[i][1] - '0']++;
			num[1][0]++;
		}
		if (cards[i][0] == 'T')
		{
			num[2][cards[i][1] - '0']++;
			num[2][0]++;
		}
	}

	int Tingtype=0;
	for(int x=0;x<3;x++)
		for(int y=1;y<=9 ;y++)
		{
			if(num[x][y]==0)
				continue;
			num[x][y]--;

			//检查听牌部分
			int tempnum[3][10];
			for(int i=0;i<3;i++)
				for(int j=1;j<=9;j++)
					tempnum[i][j]=num[i][j];//临时数组

			int cnt=0;//已经成套的牌数
			int rest=nonFrozenCount-1;
			for(int i=0;i<3;i++)
				for(int j=1;j<=9;j++)
				{
					if(valid(j+2) && tempnum[i][j]>0 &&tempnum[i][j+1]>0 && tempnum[i][j+2]>0)//先去掉顺子
					{
						tempnum[i][j]--;
						tempnum[i][j+1]--;
						tempnum[i][j+2]--;
						cnt+=3;
						j--;//334455需要算两遍
						continue;
					}
					if(tempnum[i][j]==3)//再去掉碰牌
					{
						tempnum[i][j]-=3;
						cnt+=3;
					}
				}
			rest-=cnt;
			if(rest==1||rest==4)
			{
				if(rest==1)
					temp = 1;
				if(rest==4)
				{
					int cnt=0;//记录对子个数
					for(int i=0;i<3;i++)
						for(int j=1;j<=9;j++)
						{
							if(tempnum[i][j]==2)
								cnt++;
						}
					if(cnt==2)//最后听的是4455之类的
						temp = 2;
					else if(cnt==1)//一套掌
					{
						for(int i=0;i<3;i++)
							for(int j=1;j<=9;j++)
							{
								if(valid(j+1) && tempnum[i][j]==1&&tempnum[i][j+1]==1)//最后听的是2388之类的
									temp = 3;
								if(valid(j+2) && tempnum[i][j]==1 && tempnum[i][j+2]==1)//最后听的是2488之类的
									temp=2;
							}
					}
					else
						temp = 0;
				}
			}
			else
				temp = 0;
			//检查听牌部分

			if(temp>Tingtype)//如果出这张牌可以听牌(或者听更容易胡的牌)，那就出这张
			{
				Tingtype=temp;
				ans=num2str(x,y);
			}
			num[x][y]++;
		}

	return ans;
}
