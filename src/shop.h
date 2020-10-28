/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __SHOP_H_
#define __SHOP_H_

#include <list>
#include <string>

#include "mthread.h"


class WindowElementMemo;


struct ShopDetailItem {
	unsigned short itemID;
	unsigned char count;
	std::string name;
	unsigned int weight;
	int buyPrice;
	int sellPrice;
};

struct ShopItem {
	unsigned short itemID;
	unsigned char count;
	unsigned char amount;
};


typedef std::list<ShopDetailItem> ShopDetailItemsList;
typedef std::list<ShopItem> ShopItemsList;

class Shop {
private:
	WindowElementMemo*	memo;
	bool				purchase;

	std::string			name;

	ShopDetailItemsList		itemsList;
	ShopItemsList			purchaseList;
	ShopItemsList			saleList;

	unsigned int	money;

	bool ignoreCap;
	bool inBackpack;

	MUTEX	lockShop;

public:
	Shop();
	~Shop();

	void SetName(std::string name);
	std::string GetName();

	void SetMemo(WindowElementMemo* memo);
	WindowElementMemo* GetMemo();
	void SetPurchase(bool state);

	void SetMoney(unsigned int value);
	unsigned int GetMoney();
	unsigned int* GetMoneyPtr();

	void SetIgnoreCap(bool state);
	bool GetIgnoreCap();

	void SetInBackpack(bool state);
	bool GetInBackpack();

	void AddShopDetailItem(ShopDetailItem item);
	ShopDetailItem GetShopDetailItemByID(unsigned short itemID, unsigned char count);
	ShopDetailItemsList GetShopDetailItems();
	void ClearShopDetailItems();

	void AddPurchaseItem(ShopItem item);
	ShopItem GetPurchaseItem(int number);
	ShopItemsList GetPurchaseItems();
	void ClearPurchaseItems();

	void AddSaleItem(ShopItem item);
	void UpdateSaleItem(ShopItem item);
	void ClearSaleItemsAmount();
	ShopItem GetSaleItem(int number);
	ShopItemsList GetSaleItems();
	void ClearSaleItems();

	void UpdateMemo();
};

#endif //__SHOP_H_
