#include <iostream>
#include <string>
#include <vector>

enum Slots
{
    MAX_SLOTS = 8,
    SLOT_MAX_STACK = 100
};

// shut up compiler...
class Player;

// item IDS
enum Items
{
    // I use this shit so its more easy to find empty spaces with only 1 method, excellent right? ;)
    ITEM_NONE  = 0,
    ITEM_APPLE = 1,
    ITEM_PEAR  = 2
};

// I should expand this...
class Item
{ 
    public:
        Item(std::string name, int id) : _name(name), _id(id) { }
        Item(const Item& it) 
        {
            _name = it._name;
            _id = it._id;
        }
        Item()
        {
            _id = ITEM_NONE;
            _name = "Empty";
        }
        
        int getId() { return _id; }
        std::string getName() { return _name; }
        
    protected:
        std::string _name;
        int _id;
};

// each inventory slot has an item (can be nullptr) and a quantity
class InventorySlot
{
    public:
        InventorySlot() 
        { 
            _item = new Item;
            _stack = 0; 
        }
        
        ~InventorySlot()
        {
            delete _item;
        }
        
        InventorySlot(Item* it, int quant) : _item(it), _stack(quant) { }
        
        Item* getItem() { return _item; }
        int getStack() { return _stack; }
        void setItem(Item* it) { _item = it; }
        void setStack(int newStack) { _stack = newStack; } 
        void reset() { _item = new Item; _stack = 0; }
        
    protected:
        Item* _item;
        int _stack;
};

// little redefinition, this motherfucker is lazy today
typedef std::pair<int, bool> searchResult;

// well, simply a vector of inventoryslots, I don't really like how the methods are implemented, but I'm feeling dumb today, so maybe myself of tomorrow will fix this shit code
class Inventory
{
    public:
        Inventory(Player* owner) 
        {
            _owner = owner;
            _emptySlots = 0;
            
            for(int i=0;i<MAX_SLOTS;i++)
            {
                InventorySlot* slot = new InventorySlot();
                slots.push_back(slot);
                _emptySlots++;
            }
        }
        
        ~Inventory() 
        {
            for(int i=0;i<(int)slots.size();i++)
            {
                // crash check, I'm not really sure if this can happen tho
                if (slots[i] != nullptr)
                    delete slots[i]->getItem();
                delete slots[i];
            }
            slots.clear();
        }
        
        bool hasFreeSlots() { return _emptySlots != 0; }
        
        bool addItem(Item* it, int quant) 
        {
            if (it == nullptr)
                return false;
            searchResult result = findInventorySlotWithItem(it);

            if (!result.second)
            {
                while (quant >= SLOT_MAX_STACK)
                {
                    // we must check the slots everytime
                    if (!hasFreeSlots())
                        return false;
                    
                    // make a new item per loop
                    Item* newItem = new Item(*it);
                    int emptySlot = findNextEmptySlot();
                    quant -= SLOT_MAX_STACK;
                    // no safe check as it shouldn't be null
                    slots[emptySlot]->setItem(newItem);
                    slots[emptySlot]->setStack(SLOT_MAX_STACK);
                    _emptySlots--;
                }
                
                if (quant > 0)
                {
                    // also here
                    if (!hasFreeSlots())
                        return false;
                    Item* newItem = new Item(*it);
                    int emptySlot = findNextEmptySlot();
                    slots[emptySlot]->setItem(newItem);
                    slots[emptySlot]->setStack(quant);
                    _emptySlots--;
                }
                
                return true;
            }
            else
            {
                int updatedStack = slots[result.first]->getStack() + quant;
                if (updatedStack <= SLOT_MAX_STACK)
                {
                    slots[result.first]->setStack(updatedStack);
                    return true;
                }
                else
                {
                    // first we fill the available space
                    int slotsLeft = SLOT_MAX_STACK - slots[result.first]->getStack();
                    slots[result.first]->setStack(SLOT_MAX_STACK);
                    quant -= slotsLeft;
                    
                    // then we need to create all the stacks needed if there are free slots
                    while (quant >= SLOT_MAX_STACK)
                    {
                        if (!hasFreeSlots())
                            return false;
                        Item* newItem = new Item(*it);
                        int emptySlot = findNextEmptySlot();
                        slots[emptySlot]->setItem(newItem);
                        slots[emptySlot]->setStack(SLOT_MAX_STACK);
                        quant -= SLOT_MAX_STACK;
                        _emptySlots--;
                    }
                    
                    // add the rest, and also check for free slots
                    if (quant > 0)
                    {
                        if (!hasFreeSlots())
                            return false;
                        Item* newItem = new Item(*it);
                        int emptySlot = findNextEmptySlot();
                        slots[emptySlot]->setItem(newItem);
                        slots[emptySlot]->setStack(quant);
                        _emptySlots--;
                    }
                    
                    return true;
                }
            }
            
            return false;
        }
        
        bool removeItem(Item* it, int quant) 
        {
            if (it == nullptr)
                return false;
            
            searchResult result = findInventorySlotWithItem(it);
            
            if (!result.second)
                return false;
            
            
            if (slots[result.first]->getStack() >= quant)
            {
                slots[result.first]->setStack(slots[result.first]->getStack()-quant);
                
                if (slots[result.first]->getStack() == 0)
                {
                    slots[result.first]->reset();
                    _emptySlots++;
                }
            }
            else
            {
                quant -= slots[result.first]->getStack();
                slots[result.first]->reset();
                _emptySlots++;
                
                // now we should iterate until quant == 0, each time we call findInventorySlotWithItem
                while (quant != 0)
                {
                    result = findInventorySlotWithItem(it);
                    if (result.second)
                    {
                        if (slots[result.first]->getStack() - quant >= 0)
                        {
                            slots[result.first]->setStack(slots[result.first]->getStack()-quant);
                            quant = 0;
                        }
                        else
                        {
                            quant -= slots[result.first]->getStack();
                            slots[result.first]->setStack(0);
                        }
                        
                        if (slots[result.first]->getStack() == 0)
                        {
                            slots[result.first]->reset();
                            _emptySlots++;
                        }
                    }
                }
                
                return true;
            }
            
            
            return false;
        }
        
        bool findItem(Item* it) 
        {
            searchResult result= findInventorySlotWithItem(it);
            return result.second;
        }
        
        void showInventory()
        {
            for(int i=0;i<MAX_SLOTS;i++)
            {
                if (slots[i]->getItem()->getId() == ITEM_NONE)
                    continue;
                
                std::cout << "Slot: " << i << ", item: " << slots[i]->getItem()->getName() << ", quant: " << slots[i]->getStack() << std::endl;
            }
            std::cout << "End of player inventory" << std::endl;
        }
        
    protected:
        Player* _owner;
        std::vector<InventorySlot*> slots;
        // optimization purposes
        int _emptySlots;
        
        searchResult findInventorySlotWithItem(Item* it) 
        {
            searchResult result(-1, false);
            if (it == nullptr)
                return result;
            
            for(int i=0;i<MAX_SLOTS;i++) 
            { 
                auto item = slots[i]->getItem();
                if (item->getId() == it->getId())
                {
                    result.first = i;
                    result.second = true;
                }
            }
            
            return result;
        }
        
        // free slots
        int findNextEmptySlot()
        {
            int result = -1;            
            for(int i=0;i<MAX_SLOTS;i++) 
            { 
                auto item = slots[i]->getItem();
                if (item->getId() == ITEM_NONE)
                {
                    result = i;
                    break;
                }
            }
            
            return result;
        }
};

class Player
{
    public:
        Player(std::string name = "Andy") : _name(name) 
        {
            _inv = new Inventory(this);
        }
        
        ~Player() { delete _inv; }
        
        std::string sayHi() { std::string returnValue = "Hi, my name is " + _name; return returnValue; }
        
        void showInventory() { _inv->showInventory(); }
        bool addItemToInventory(Item* it, int quant) { return _inv->addItem(it, quant); } 
        bool removeItemFromInventory(Item* it, int quant) { return _inv->removeItem(it, quant); }
    
    protected:
        std::string _name;
        Inventory* _inv;
};


int main(int argc, char *argv[]) {
    std::string name;
    std::cout << "Welcome to the inventory system..." << std::endl;
    
    std::cout << "Please give me your name." << std::endl;
    std::cin >> name;
    
    Player* pl = new Player(name);
    
    std::cout << pl->sayHi() << std::endl;
    Item* apple = new Item("Apple", ITEM_APPLE);
    Item* pear = new Item("Pear", ITEM_PEAR);
    pl->addItemToInventory(apple, 154);
    pl->addItemToInventory(pear, 157);
    
    pl->showInventory();
    pl->removeItemFromInventory(pear, 113);
    pl->removeItemFromInventory(apple, 94);
    
    pl->showInventory();
    
    return 0;
}

