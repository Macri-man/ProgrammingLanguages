#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <limits>

// Base Character class
class Character {
protected:
    std::string name;
    int health;
    int maxHealth;
    int attackPower;
public:
    Character(std::string name, int health, int attackPower)
        : name(name), health(health), maxHealth(health), attackPower(attackPower) {}

    virtual ~Character() {}

    virtual void attack(Character& target) {
        int damage = rand() % attackPower + 1; // 1 to attackPower
        std::cout << name << " attacks " << target.getName()
                  << " for " << damage << " damage!\n";
        target.takeDamage(damage);
    }

    virtual void takeDamage(int amount) {
        health -= amount;
        if (health < 0) health = 0;
    }

    bool isAlive() const {
        return health > 0;
    }

    std::string getName() const {
        return name;
    }

    int getHealth() const {
        return health;
    }

    int getMaxHealth() const {
        return maxHealth;
    }

    virtual void showStats() const {
        std::cout << name << " HP: " << health << "/" << maxHealth << "\n";
    }
};

// Item system with Weapon, Potion, Armor
struct Item {
    enum Type { Potion, Weapon, Armor } type;
    std::string name;
    int power; // For weapon: attack boost; for potion: heal amount; for armor: defense boost

    Item(Type t, std::string n, int p) : type(t), name(n), power(p) {}
};

enum class PlayerClass { Warrior, Mage, Archer };

class Player : public Character {
    PlayerClass playerClass;
    int level;
    int experience;

    // Equipment slots
    Item* equippedWeapon;
    Item* equippedArmor;

    std::vector<Item> inventory;

public:
    Player(std::string name, PlayerClass pclass)
        : Character(name, 100, 15),
          playerClass(pclass),
          level(1),
          experience(0),
          equippedWeapon(nullptr),
          equippedArmor(nullptr) 
    {
        // Initialize stats & inventory based on class
        switch (playerClass) {
            case PlayerClass::Warrior:
                maxHealth = health = 120;
                attackPower = 20;
                inventory.push_back(Item(Item::Potion, "Health Potion", 50));
                inventory.push_back(Item(Item::Weapon, "Iron Sword", 8));
                inventory.push_back(Item(Item::Armor, "Steel Armor", 5));
                break;
            case PlayerClass::Mage:
                maxHealth = health = 90;
                attackPower = 15;
                inventory.push_back(Item(Item::Potion, "Mana Potion", 30)); // treat as healing for now
                inventory.push_back(Item(Item::Weapon, "Magic Staff", 10));
                inventory.push_back(Item(Item::Armor, "Robes", 2));
                break;
            case PlayerClass::Archer:
                maxHealth = health = 100;
                attackPower = 18;
                inventory.push_back(Item(Item::Potion, "Health Potion", 40));
                inventory.push_back(Item(Item::Weapon, "Longbow", 9));
                inventory.push_back(Item(Item::Armor, "Leather Armor", 3));
                break;
        }
    }

    // Override attack to include weapon bonus
    void attack(Character& target) override {
        int weaponBonus = (equippedWeapon ? equippedWeapon->power : 0);
        int damage = rand() % attackPower + 1 + weaponBonus;
        std::cout << name << " (" << getClassName() << ") attacks " << target.getName()
                  << " with ";
        if (equippedWeapon)
            std::cout << equippedWeapon->name;
        else
            std::cout << "fists";
        std::cout << " for " << damage << " damage!\n";
        target.takeDamage(damage);
    }

    // Override takeDamage to factor armor defense
    void takeDamage(int amount) override {
        int armorDefense = (equippedArmor ? equippedArmor->power : 0);
        int reducedDamage = amount - armorDefense;
        if (reducedDamage < 0) reducedDamage = 0;
        std::cout << name << "'s armor reduces damage by " << armorDefense
                  << ". Damage taken: " << reducedDamage << "\n";
        health -= reducedDamage;
        if (health < 0) health = 0;
    }

    void usePotion() {
        for (size_t i = 0; i < inventory.size(); ++i) {
            if (inventory[i].type == Item::Potion) {
                int healAmount = inventory[i].power;
                health += healAmount;
                if (health > maxHealth) health = maxHealth;
                std::cout << name << " uses " << inventory[i].name << " and heals " << healAmount << " HP!\n";
                inventory.erase(inventory.begin() + i);
                return;
            }
        }
        std::cout << "No potions left in inventory!\n";
    }

    void showInventory() const {
        std::cout << "\n-- Inventory --\n";
        if (inventory.empty()) {
            std::cout << "Empty\n";
            return;
        }
        for (size_t i = 0; i < inventory.size(); ++i) {
            std::cout << i+1 << ". " << inventory[i].name << " (";
            switch (inventory[i].type) {
                case Item::Potion: std::cout << "Potion, Heal: "; break;
                case Item::Weapon: std::cout << "Weapon, Attack +"; break;
                case Item::Armor: std::cout << "Armor, Defense +"; break;
            }
            std::cout << inventory[i].power << ")\n";
        }
        std::cout << "\nEquipped Weapon: ";
        if (equippedWeapon)
            std::cout << equippedWeapon->name << " (Attack +" << equippedWeapon->power << ")\n";
        else
            std::cout << "None\n";

        std::cout << "Equipped Armor: ";
        if (equippedArmor)
            std::cout << equippedArmor->name << " (Defense +" << equippedArmor->power << ")\n";
        else
            std::cout << "None\n";
    }

    // Equip weapon or armor from inventory
    void equipItem() {
        if (inventory.empty()) {
            std::cout << "Inventory is empty. Nothing to equip.\n";
            return;
        }
        showInventory();
        std::cout << "Enter the number of the item to equip (weapon or armor), or 0 to cancel: ";
        int choice;
        std::cin >> choice;
        if (choice <= 0 || choice > (int)inventory.size()) {
            std::cout << "Equip cancelled.\n";
            return;
        }
        Item& item = inventory[choice - 1];
        if (item.type == Item::Weapon) {
            if (equippedWeapon) {
                std::cout << "Unequipped " << equippedWeapon->name << " and returned to inventory.\n";
                inventory.push_back(*equippedWeapon);
            }
            equippedWeapon = new Item(item);
            std::cout << "Equipped weapon: " << equippedWeapon->name << "\n";
            inventory.erase(inventory.begin() + (choice - 1));
        }
        else if (item.type == Item::Armor) {
            if (equippedArmor) {
                std::cout << "Unequipped " << equippedArmor->name << " and returned to inventory.\n";
                inventory.push_back(*equippedArmor);
            }
            equippedArmor = new Item(item);
            std::cout << "Equipped armor: " << equippedArmor->name << "\n";
            inventory.erase(inventory.begin() + (choice - 1));
        }
        else {
            std::cout << "Cannot equip that item.\n";
        }
    }

    void unequipItem() {
        std::cout << "Unequip:\n1. Weapon\n2. Armor\nChoose 0 to cancel: ";
        int choice;
        std::cin >> choice;
        if (choice == 1 && equippedWeapon) {
            inventory.push_back(*equippedWeapon);
            std::cout << "Unequipped " << equippedWeapon->name << " and returned to inventory.\n";
            delete equippedWeapon;
            equippedWeapon = nullptr;
        }
        else if (choice == 2 && equippedArmor) {
            inventory.push_back(*equippedArmor);
            std::cout << "Unequipped " << equippedArmor->name << " and returned to inventory.\n";
            delete equippedArmor;
            equippedArmor = nullptr;
        }
        else if (choice == 0) {
            std::cout << "Unequip cancelled.\n";
        }
        else {
            std::cout << "Nothing equipped in that slot.\n";
        }
    }

    void gainExperience(int xp) {
        experience += xp;
        std::cout << name << " gains " << xp << " XP!\n";
        while (experience >= level * 100) {
            experience -= level * 100;
            levelUp();
        }
    }

    void levelUp() {
        level++;
        maxHealth += 20;
        health = maxHealth;
        attackPower += 5;
        std::cout << "\n*** " << name << " leveled up to level " << level << "! ***\n";
        std::cout << "Health and attack power increased!\n";
    }

    int getLevel() const {
        return level;
    }

    PlayerClass getClass() const {
        return playerClass;
    }

    std::string getClassName() const {
        switch (playerClass) {
            case PlayerClass::Warrior: return "Warrior";
            case PlayerClass::Mage: return "Mage";
            case PlayerClass::Archer: return "Archer";
        }
        return "Unknown";
    }

    void showStats() const override {
        std::cout << name << " (" << getClassName() << ") Level " << level
                  << "\nHP: " << health << "/" << maxHealth
                  << "\nAttack Power: " << attackPower << "\n";
        if (equippedWeapon)
            std::cout << "Weapon Bonus: +" << equippedWeapon->power << "\n";
        if (equippedArmor)
            std::cout << "Armor Bonus: +" << equippedArmor->power << "\n";
        std::cout << "Experience: " << experience << "/" << level * 100 << "\n";
    }

    // Special ability per class
    void specialAbility(Character& target) {
        switch (playerClass) {
            case PlayerClass::Warrior:
                warriorBerserk(target);
                break;
            case PlayerClass::Mage:
                mageFireball(target);
                break;
            case PlayerClass::Archer:
                archerRapidShot(target);
                break;
        }
    }

private:
    void warriorBerserk(Character& target) {
        int weaponBonus = (equippedWeapon ? equippedWeapon->power : 0);
        int damage = (rand() % (attackPower + 10)) + weaponBonus + 10;
        std::cout << name << " uses Berserk and deals " << damage << " massive damage to " << target.getName() << "!\n";
        target.takeDamage(damage);
        int recoil = 5;
        health -= recoil;
        if (health < 0) health = 0;
        std::cout << name << " suffers " << recoil << " recoil damage.\n";
    }

    void mageFireball(Character& target) {
        int weaponBonus = (equippedWeapon ? equippedWeapon->power : 0);
        int damage = (rand() % (attackPower + 15)) + weaponBonus + 5;
        std::cout << name << " casts Fireball and burns " << target.getName()
                  << " for " << damage << " damage!\n";
        target.takeDamage(damage);
    }

    void archerRapidShot(Character& target) {
        int weaponBonus = (equippedWeapon ? equippedWeapon->power : 0);
        int shots = 3;
        int totalDamage = 0;
        std::cout << name << " fires Rapid Shots!\n";
        for (int i = 0; i < shots; ++i) {
            int damage = (rand() % attackPower) + weaponBonus / 2 + 3;
            std::cout << "Shot " << (i + 1) << " deals " << damage << " damage.\n";
            target.takeDamage(damage);
            totalDamage += damage;
            if (!target.isAlive()) break;
        }
        std::cout << "Total Rapid Shot damage: " << totalDamage << "\n";
    }
};

// Multiple enemy types
class Enemy : public Character {
    int experienceReward;
public:
    Enemy(std::string name, int health, int attackPower, int xpReward)
        : Character(name, health, attackPower), experienceReward(xpReward) {}

    int getExperienceReward() const { return experienceReward; }

    void showStats() const override {
        std::cout << "[Enemy] " << name << " HP: " << health << "/" << maxHealth << "\n";
    }
};

Enemy generateRandomEnemy() {
    int choice = rand() % 3;
    switch (choice) {
        case 0: return Enemy("Goblin", 60, 15, 50);
        case 1: return Enemy("Orc", 90, 20, 80);
        case 2: return Enemy("Dragon", 150, 30, 200);
    }
    return Enemy("Goblin", 60, 15, 50); // fallback
}

// ASCII art for UI enhancement
void printTitle() {
    std::cout << R"(
  ____                       _                
 |  _ \ ___ _ __   ___  _ __| |_ ___  _ __    
 | |_) / _ \ '_ \ / _ \| '__| __/ _ \| '__|   
 |  __/  __/ |_) | (_) | |  | || (_) | |      
 |_|   \___| .__/ \___/|_|   \__\___/|_|      
           |_|                                
    )" << "\n";
    std::cout << "Welcome to the ASCII RPG Battle!\n\n";
}

void printBattleBanner(const std::string& enemyName) {
    std::cout << "\n=== A wild " << enemyName << " appears! ===\n";
}

void battle(Player& player, Enemy& enemy) {
    printBattleBanner(enemy.getName());
    while (player.isAlive() && enemy.isAlive()) {
        std::cout << "\n--- Battle Menu ---\n";
        std::cout << "1. Attack\n";
        std::cout << "2. Defend (Heal)\n";
        std::cout << "3. Use Potion\n";
        std::cout << "4. Special Ability\n";
        std::cout << "5. Show Inventory\n";
        std::cout << "6. Equip Item\n";
        std::cout << "7. Unequip Item\n";
        std::cout << "Choose your action: ";

        int choice;
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Try again.\n";
            continue;
        }

        switch (choice) {
            case 1:
                player.attack(enemy);
                break;
            case 2: {
                int heal = rand() % 10 + 5;
                player.takeDamage(-heal); // heal
                if (player.getHealth() > player.getMaxHealth())
                    player.takeDamage(player.getHealth() - player.getMaxHealth());
                std::cout << player.getName() << " defends and heals " << heal << " HP!\n";
                break;
            }
            case 3:
                player.usePotion();
                break;
            case 4:
                player.specialAbility(enemy);
                break;
            case 5:
                player.showInventory();
                continue; // skip enemy attack after showing inventory
            case 6:
                player.equipItem();
                continue;
            case 7:
                player.unequipItem();
                continue;
            default:
                std::cout << "Invalid choice.\n";
                continue;
        }

        if (enemy.isAlive()) {
            enemy.attack(player);
        }

        std::cout << "\nStatus:\n";
        player.showStats();
        enemy.showStats();
    }

    if (player.isAlive()) {
        std::cout << "\nYou defeated " << enemy.getName() << "!\n";
        player.gainExperience(enemy.getExperienceReward());
    } else {
        std::cout << "\nYou were defeated by " << enemy.getName() << "...\n";
    }
}

PlayerClass chooseClass() {
    while (true) {
        std::cout << "\nChoose your class:\n";
        std::cout << "1. Warrior\n2. Mage\n3. Archer\n";
        std::cout << "Enter choice: ";
        int choice;
        std::cin >> choice;
        if (choice == 1) return PlayerClass::Warrior;
        if (choice == 2) return PlayerClass::Mage;
        if (choice == 3) return PlayerClass::Archer;
        std::cout << "Invalid choice. Try again.\n";
    }
}

int main() {
    srand(static_cast<unsigned int>(time(0)));

    printTitle();

    std::cout << "Enter your character's name: ";
    std::string playerName;
    std::cin.ignore(); // clear newline before getline
    std::getline(std::cin, playerName);

    PlayerClass pclass = chooseClass();

    Player player(playerName, pclass);

    std::cout << "\nGreetings, " << playerName << " the " << player.getClassName() << "!\n";

    bool keepPlaying = true;
    while (keepPlaying && player.isAlive()) {
        Enemy enemy = generateRandomEnemy();

        battle(player, enemy);

        if (!player.isAlive()) break;

        std::cout << "\nDo you want to fight another enemy? (y/n): ";
        char cont;
        std::cin >> cont;
        if (cont != 'y' && cont != 'Y') {
            keepPlaying = false;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // clear newline
    }

    std::cout << "\nThanks for playing! Final stats:\n";
    player.showStats();

    return 0;
}
