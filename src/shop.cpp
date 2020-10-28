/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "shop.h"

#include <stdio.h>

#include "allocator.h"
#include "window.h"


// ---- Shop ---- //

Shop::Shop() {
	memo = NULL;
	purchase = true;
	money = 0;

	ignoreCap = false;
	inBackpack = false;
}

Shop::~Shop() { }


void Shop::SetName(std::string name) {
	this->name = name;
}

std::string Shop::GetName() {
	return name;
}


void Shop::SetMemo(WindowElementMemo* memo) {
	LOCKCLASS lockClass(lockShop);

	this->memo = memo;
}

WindowElementMemo* Shop::GetMemo() {
	LOCKCLASS lockClass(lockShop);

	return memo;
}

void Shop::SetPurchase(bool state) {
	LOCKCLASS lockClass(lockShop);

	purchase = state;
}


void Shop::SetMoney(unsigned int value) {
	LOCKCLASS lockClass(lockShop);

	money = value;
}

unsigned int Shop::GetMoney() {
	LOCKCLASS lockClass(lockShop);

	return money;
}

unsigned int* Shop::GetMoneyPtr() {
	LOCKCLASS lockClass(lockShop);

	return &money;
}


void Shop::SetIgnoreCap(bool state) {
	LOCKCLASS lockClass(lockShop);

	ignoreCap = state;
}

bool Shop::GetIgnoreCap() {
	LOCKCLASS lockClass(lockShop);

	return ignoreCap;
}


void Shop::SetInBackpack(bool state) {
	LOCKCLASS lockClass(lockShop);

	inBackpack = state;
}

bool Shop::GetInBackpack() {
	LOCKCLASS lockClass(lockShop);

	return inBackpack;
}


void Shop::AddShopDetailItem(ShopDetailItem item) {
	LOCKCLASS lockClass(lockShop);

	itemsList.push_back(item);
}

ShopDetailItem Shop::GetShopDetailItemByID(unsigned short itemID, unsigned char count) {
	LOCKCLASS lockClass(lockShop);

	ShopDetailItemsList::iterator it = itemsList.begin();
	for (it; it != itemsList.end(); it++)
		if (itemID == it->itemID && count == it->count)
			return *it;

	ShopDetailItem item;
	memset(&item, 0, sizeof(item));

	return item;
}

ShopDetailItemsList Shop::GetShopDetailItems() {
	LOCKCLASS lockClass(lockShop);

	return itemsList;
}

void Shop::ClearShopDetailItems() {
	LOCKCLASS lockClass(lockShop);

	itemsList.clear();
}


void Shop::AddPurchaseItem(ShopItem item) {
	LOCKCLASS lockClass(lockShop);

	purchaseList.push_back(item);
}

ShopItem Shop::GetPurchaseItem(int number) {
	LOCKCLASS lockClass(lockShop);

	ShopItemsList::iterator it = purchaseList.begin();
	for (it, number; it != purchaseList.end() && number >= 0; it++, number--)
		if (number == 0)
			return *it;

	ShopItem item;
	memset(&item, 0, sizeof(item));

	return item;
}

ShopItemsList Shop::GetPurchaseItems() {
	LOCKCLASS lockClass(lockShop);

	return purchaseList;
}

void Shop::ClearPurchaseItems() {
	LOCKCLASS lockClass(lockShop);

	purchaseList.clear();
}


void Shop::AddSaleItem(ShopItem item) {
	LOCKCLASS lockClass(lockShop);

	saleList.push_back(item);
}

void Shop::UpdateSaleItem(ShopItem item) {
	LOCKCLASS lockClass(lockShop);

	ShopItemsList::iterator it = saleList.begin();
	for (it; it != saleList.end(); it++) {
		ShopItem sitem = *it;
		if (sitem.itemID == item.itemID)
			it->amount = item.amount;
	}
}

void Shop::ClearSaleItemsAmount() {
	LOCKCLASS lockClass(lockShop);

	ShopItemsList::iterator it = saleList.begin();
	for (it; it != saleList.end(); it++)
		it->amount = 0;
}

ShopItem Shop::GetSaleItem(int number) {
	LOCKCLASS lockClass(lockShop);

	ShopItemsList::iterator it = saleList.begin();
	for (it, number; it != saleList.end() && number >= 0; it++, number--)
		if (number == 0)
			return *it;

	ShopItem item;
	memset(&item, 0, sizeof(item));

	return item;
}

ShopItemsList Shop::GetSaleItems() {
	LOCKCLASS lockClass(lockShop);

	return saleList;
}

void Shop::ClearSaleItems() {
	LOCKCLASS lockClass(lockShop);

	saleList.clear();
}


void Shop::UpdateMemo() {
	LOCKCLASS lockClass1(Windows::lockWindows);
	LOCKCLASS lockClass2(lockShop);

	if (!memo)
		return;

	int option = memo->GetOption();

	memo->Clear();
	if (purchase) {
		ShopItemsList purchaseItems = GetPurchaseItems();
		ShopItemsList::iterator it = purchaseItems.begin();
		for (it; it != purchaseItems.end(); it++) {
			ShopItem purchaseItem = *it;

			ShopDetailItem item = GetShopDetailItemByID(purchaseItem.itemID, purchaseItem.count);
			std::string text = item.name + " / " + value2str(item.buyPrice) + " gp / " + float2str((float)item.weight / 100) + "oz.";

			memo->AddElement(text);
		}
	}
	else {
		ShopItemsList saleItems = GetSaleItems();
		ShopItemsList::iterator it = saleItems.begin();
		for (it; it != saleItems.end(); it++) {
			ShopItem saleItem = *it;

			ShopDetailItem item = GetShopDetailItemByID(saleItem.itemID, saleItem.count);
			std::string text = item.name + " / " + value2str(item.sellPrice) + " gp / " + float2str((float)item.weight / 100) + "oz.";

			memo->AddElement(text, saleItem.amount > 0);
		}
	}

	memo->SetOption(option);
}
