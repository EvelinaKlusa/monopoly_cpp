#include <random>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <mutex>
#include <future>
#include <unordered_map>
#include <cstdlib>
#include <numeric>
#include <ctime>

using namespace std;

// dabū īpašumus
struct Property {
    int nr;
    int price = 0;
    int rent = 0;
    int h1 = 0, h2 = 0, h3 = 0, h4 = 0;
    int hotel = 0;
    int h_cost = 0;
    int mortgage = 0;
    std::string color;
    int same = 0;
    int houses = 0;
};


// izveido spēlētāju kā paraugstruktūru, no kuras attiecīgi dabūs visus spēlētājus
struct Player {
    int laucins;
    int nauda;
    std::vector<int> ipasumi;
    std::vector<int> kilas;
    int jf;
    std::vector<int> merkis;
    std::string vards;

    bool operator==(const Player& other) const {
        return vards == other.vards; // katram savs
    }

    bool navvalid() const {
        return vards.empty(); // ja vārda nav, tad neeksistē, nav valid
    }
};


std::ostream& operator<<(std::ostream& os, const Player& p) {
    os << "Name: " << p.vards << "\n";
    os << "Nauda: " << p.nauda << "\n";
    os << "Pozicija: " << p.laucins << "\n";
    os << "Ipasumi: ";
    for (int ip : p.ipasumi) os << ip << " ";
    os << "\n";
    os << "Kilas: ";
    for (int k : p.kilas) os << k << " ";
    os << "\n";
    os << "Merkis: ";
    for (int m : p.merkis) os << m << " ";
    os << "\n";
    return os;
};

std::ostream& operator<<(std::ostream& os, const Property& prop) {
    os << "Property ID: " << prop.nr << "\n";
    os << "Color: " << prop.color << "\n";
    os << "Houses: " << prop.houses << "\n";
    os << "Same: " << prop.same << "\n";
    return os;
};


// funkcijas ==============================================================================================================================================================================

// numuri ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// no sākuma parāda, ko atgriezīs
// static = atcerēsis rezultātu, to nevar mainīt
// {} ir kā :
static Property* atrod_pec_nr(int nr, std::vector<Property>& ipasumi) {
    for (auto& property : ipasumi) {
        if (property.nr == nr) {
            return &property;
        }
    }
    return nullptr;
}

// ieliek pareizi spēlētāju sarakstā -------------------------------------------------------------------------------------------------------------------------------------------
void replace_sp(std::vector<Player>& speletaji, const Player& speletajs) {
    if (speletajs.navvalid()) {
        speletaji.erase(std::remove_if(speletaji.begin(), speletaji.end(), [](const Player& p) {return &p == nullptr; }), speletaji.end());
        return;
    }
    for (size_t i = 0; i < speletaji.size(); ++i) {
        if (speletaji[i].vards == speletajs.vards) {
            speletaji[i] = speletajs;
            return;
        }
    }
}

// chance un community --------------------------------------------------------------------------------------------------------------------------------------------------------------------
void chance_community(std::vector<Player>& speletaji, Player& speletajs, const std::string& krasa, std::vector<std::string>& chance, std::vector<std::string>& community_chest, std::vector<Property>& ipasumi) {
    if (speletajs.navvalid()) {
        return;
    }
    
    std::string k;
    if (krasa == "chance") {
        k = chance.front();
        std::string nonem = chance.front();
        chance.erase(chance.begin());
        if (k != "jf") {
            chance.push_back(nonem);
        }
    }
    else if (krasa == "chest") {
        k = community_chest.front();
        std::string nonem = community_chest.front();
        community_chest.erase(community_chest.begin());
        if (k != "jf") {
            community_chest.push_back(nonem);
        }
    }

    if (k == "jf") {
        speletajs.jf += 1;
    }
    else if (k == "-3s") {
        speletajs.laucins -= 3;
        Property* laucins_info = atrod_pec_nr(speletajs.laucins, ipasumi);
        if (laucins_info != nullptr) {
            std::string kr = laucins_info->color;
            if (kr == "chest" || kr == "chance") {
                chance_community(speletaji, speletajs, kr, chance, community_chest, ipasumi);
            }
        }
    }
    else if (k == "l_25") {
        if (speletajs.laucins > 25) {
            speletajs.nauda += 2000;
        }
        speletajs.laucins = 25;
    }
    else if (k == "l_12") {
        if (speletajs.laucins > 12) {
            speletajs.nauda += 2000;
        }
        speletajs.laucins = 12;
    }
    else if (k == "l_1") {
        speletajs.laucins = 1;
    }
    else if (k == "l_0") {
        speletajs.laucins = 0;
    }
    else if (k == "l_6") {
        if (speletajs.laucins > 6) {
            speletajs.nauda += 2000;
        }
        speletajs.laucins = 6;
    }
    else if (k == "l_40") {
        speletajs.laucins = 40;
    }
    else if (k == "l_utility") {
        if (speletajs.laucins > 30) {
            speletajs.laucins = 13;
            speletajs.nauda += 2000;
        }
        else if (speletajs.laucins > 13) {
            speletajs.laucins = 29;
        }
        else {
            speletajs.laucins = 13;
        }
    }
    else if (k == "l_station") {
        if (speletajs.laucins > 36 || speletajs.laucins < 6) {
            speletajs.laucins = 6;
            speletajs.nauda += 2000; // šo kārti var dabūt tikai chance, ir tikai pirms strata, tāpēc visiem dod naudu
        }
        else if (speletajs.laucins > 26) {
            speletajs.laucins = 36;
        }
        else if (speletajs.laucins > 16) {
            speletajs.laucins = 26;
        }
        else {
            speletajs.laucins = 16;
        }
    }
    else if (k == "tax_ch") {
        int houses = 0;
        int hotels = 0;
        for (int nr : speletajs.ipasumi) {
            Property* property = atrod_pec_nr(nr, ipasumi);
            if (property) {
                if (property->houses < 5) {
                    houses += property->houses;
                }
                else if (property->houses == 5) {
                    hotels += 1;
                }
            }
        } speletajs.nauda -= 400 * houses + 1500 * hotels;
    }
    else if (k == "tax_com") {
        int houses = 0;
        int hotels = 0;
        for (int nr : speletajs.ipasumi) {
            Property* property = atrod_pec_nr(nr, ipasumi);
            if (property) {
                if (property->houses < 5) {
                    houses += property->houses;
                }
                else if (property->houses == 5) {
                    hotels += 1;
                }
            }
        } speletajs.nauda -= 250 * houses + 1000 * hotels;
    }
    else if (k == "pl+500") {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<>dis(0, speletaji.size() - 1); // dabū random int strap speletaju indeksiem
        int index = dis(gen);
        while (speletaji[index].vards == speletajs.vards) { // nedrīkst maksāt pats sev
            index = dis(gen);
        } speletaji[index].nauda += 500;
        speletajs.nauda -= 500;
    }
    else if (k == "pl-100") {
        for (auto& pl : speletaji) {
            pl.nauda -= 100;
            speletajs.nauda += 100;
        }
    }
    replace_sp(speletaji, speletajs);
}


// pardosana ----------------------------------------------------------------------------------------------------------------------------------------------------------------------
void pardosana(Player& seller, int nr, Player& buyer, int cena, std::vector<Property>& ipasumi) {
    // pārbauda, vai eksistē 
    if (seller.navvalid() || buyer.navvalid()) {
        return;
    }
    
    // pārbauda, vai ir, ko pārdot, vai pircējam ir nauda
    if ((std::find(seller.ipasumi.begin(), seller.ipasumi.end(), nr) != seller.ipasumi.end() ||
        std::find(seller.kilas.begin(), seller.ipasumi.end(), nr) != seller.kilas.end()) &&
        buyer.nauda >= cena) {

        Property* property = atrod_pec_nr(nr, ipasumi);
        // noņem mājas no vienkrāsainajiem, tās pārdod
        std::unordered_map<int, Property*> seller_properties;
        for (int nr_visi : seller.ipasumi) {
            seller_properties[nr_visi] = atrod_pec_nr(nr_visi, ipasumi);
        }

        for (auto& pair : seller_properties) {
            int nr_visi = pair.first;
            Property* prop = pair.second;
            if (prop->color == property->color && prop->houses > 0) {
                while (prop->houses > 0) {
                    prop->houses -= 1;
                    seller.nauda += 0.5 * prop->h_cost;
                }
            }
        }

        // nogādā īpašumu jaunajam īpašniekam, atņem pārdevējam
        auto it = std::find(seller.ipasumi.begin(), seller.ipasumi.end(), nr); // man pašai nav jānorāda type
        if (it != seller.ipasumi.end()) {
            seller.ipasumi.erase(it);
            buyer.ipasumi.push_back(nr);
        }
        else {
            it = find(seller.kilas.begin(), seller.ipasumi.end(), nr);
            if (it != seller.kilas.end()) {
                seller.kilas.erase(it);
                buyer.kilas.push_back(nr);
            }
        }
        // samaksā
        buyer.nauda -= cena;
        seller.nauda += cena;

        // salabo same
        for (auto& speletajs : { std::ref(seller), std::ref(buyer) }) { // ref izveido kopiju, neietekmē īsto objektu; "reference"
            int same = 0;
            std::vector<Property*> vienkrasaini;
            for (int nr : speletajs.get().ipasumi) {
                Property* prop = atrod_pec_nr(nr, ipasumi);
                if (prop->color == property->color) {
                    vienkrasaini.push_back(prop);
                }
            }
            for (int nr : speletajs.get().kilas) {
                Property* prop = atrod_pec_nr(nr, ipasumi);
                if (prop->color == property->color) {
                    vienkrasaini.push_back(prop);
                }
            }
            same = vienkrasaini.size();
            for (Property* prop : vienkrasaini) {
                prop->same = same;
            }
        }
    }
}


// izsole ---------------------------------------------------------------------

std::pair<Player*, int> izsole(std::vector<Player> speletaji, int property_nr) {
    std::vector<std::pair<Player*, int>> buyers;
    for (auto& sp : speletaji) {
        // nabagi un cietumnieki izsolē nepiedalās
        if (!sp.navvalid() && sp.laucins > 0 && sp.nauda > 100) {
            buyers.push_back({ &sp, 100 }); // pointer un sākumcena
        }
    }
    if (buyers.empty()) {
        return { nullptr, 0 };
    }

    std::vector<std::pair<Player*, int>> buyers_var;

    while (buyers.size() > 1) {
        buyers_var = buyers;
        buyers.erase(std::remove_if(buyers.begin(), buyers.end(), [](const std::pair<Player*, int>& bu) {
            return 1 - static_cast<double>(bu.second) / bu.first->nauda > 0.7 &&
                bu.first->nauda >= bu.second;
            }), buyers.end());

        if (buyers.empty()) {
            break;
        }
        // palielina cenu
        for (auto& buyer : buyers) {
            buyer.second += 100;
        }

    }

    if (buyers.size() == 1) {
        return buyers[0];
    }
    else {
        int index = std::rand() % buyers_var.size();
        return { buyers_var[index] }; // ja līdz beigām neizturēja neviens, tad izvelas random kādu no pirmspēdējā raunda
    }
}


// naudas ieguve ------------------------------------------------------------------------------------------------------------------------------------------------------------------

std::tuple<vector<Player>, Player, vector<Property>> naudas_ieguve(Player& speletajs, std::vector<Player>& speletaji, std::vector<Property>& ipasumi) {
    if (speletajs.navvalid()) {
        return { speletaji, speletajs, ipasumi };
    }
    
    auto& privatipasums = speletajs.ipasumi;
    Property* dargaka_v = nullptr;
    Property* dargaka_m = nullptr;
    Property* dargaka_k = nullptr;

    // viesnīcas pārdošana
    for (auto& property : ipasumi) {
        if (std::find(privatipasums.begin(), privatipasums.end(), property.nr) != privatipasums.end() && property.houses == 5) {
            if (dargaka_v == nullptr || property.hotel > dargaka_v->hotel) {
                dargaka_v = &property;
            }
        }
    } if (dargaka_v) {
        dargaka_v->houses -= 1;
        speletajs.nauda += 0.5 * dargaka_v->hotel;
    }
    else { // mājas pārdošana
        for (int houses = 4; houses > 0; --houses) {
            for (auto& property : ipasumi) {
                if (std::find(privatipasums.begin(), privatipasums.end(), property.nr) != privatipasums.end() && property.houses == houses) {
                    if (dargaka_m == nullptr || property.h_cost > dargaka_m->h_cost) {
                        dargaka_m = &property;
                    }
                }
            }
        } if (dargaka_m) {
            dargaka_m->houses -= 1;
            speletajs.nauda += 0.5 * dargaka_m->h_cost;
        } // ieķīlāšana
    } if (!dargaka_m && !dargaka_v) {
        for (auto& property : ipasumi) {
            if (std::find(privatipasums.begin(), privatipasums.end(), property.nr) != privatipasums.end() &&
                (!dargaka_k || property.mortgage > dargaka_k->mortgage)) {
                dargaka_k = &property;
            }
        }
    }if (dargaka_k) {
        auto it = std::find(speletajs.ipasumi.begin(), speletajs.ipasumi.end(), dargaka_k->nr);
        if (it != speletajs.ipasumi.end()) {
            speletajs.ipasumi.erase(it);
            speletajs.kilas.push_back(dargaka_k->nr);
            speletajs.nauda += dargaka_k->mortgage;
        }

    } // pardosana citam
    else {
        for (auto& numurs : privatipasums) {
            auto [buyer, cena] = izsole(speletaji, numurs);
            if (buyer) {
                for (auto& s : speletaji) {
                    if (buyer == &s) {
                        pardosana(speletajs, numurs, *buyer, cena, ipasumi);
                        replace_sp(speletaji, speletajs);
                        replace_sp(speletaji, *buyer);
                        break;
                    }
                }
            }
        } for (auto& numurs : speletajs.kilas) {
            auto [buyer, cena] = izsole(speletaji, numurs);
            if (buyer) {
                for (auto& s : speletaji) {
                    if (buyer == &s) {
                        pardosana(speletajs, numurs, *buyer, cena, ipasumi);
                        replace_sp(speletaji, speletajs);
                        replace_sp(speletaji, *buyer);
                        break;
                    }
                }
            }
        }
    }
    replace_sp(speletaji, speletajs);
    return { speletaji, speletajs, ipasumi };
}


// nebankrotesana -----------------------------------------------------------------------------------------------------------

std::tuple<Player*, std::vector<Player>, std::vector<Property>, std::vector<std::string>, std::vector<std::string>> nebankrotesana(
    Player& speletajs, std::vector<Player>& speletaji, std::vector<Property> ipasumi, std::vector<std::string>& chance, std::vector<std::string>& community_chest){
    if (speletajs.navvalid()) {
        return { &speletajs, speletaji, ipasumi, chance, community_chest };
    }

    while (speletajs.nauda < 0 && !speletajs.ipasumi.empty()) {
        std::tie(speletaji, speletajs, ipasumi) = naudas_ieguve(speletajs, speletaji, ipasumi);

    }
    if (speletajs.jf > 0 && speletajs.nauda < 0) {
        std::vector<Player> pirceji = speletaji;
        pirceji.erase(std::remove(pirceji.begin(), pirceji.end(), speletajs), pirceji.end());
        for (auto& buyer : pirceji) {
            double velme_pirkt = static_cast<double>(std::rand()) / RAND_MAX;
            if (velme_pirkt >= 0.5 && buyer.nauda >= 500) {
                buyer.nauda -= 500;
                speletajs.nauda += 500;
                buyer.jf += 1;
                speletajs.jf -= 1;
                replace_sp(speletaji, speletajs);
                replace_sp(speletaji, buyer);
            }
        }
    } // bankrots klāt
    if (speletajs.nauda < 0) {
        auto it = std::find(speletaji.begin(), speletaji.end(), speletajs);
        if (it != speletaji.end()) {
            Player seller = speletajs;
            speletaji.erase(it);

            // atliek jf kartītes
            while (seller.jf > 0) {
                if (chance.size() < 16) {
                    chance.push_back("jf");
                }
                else if (community_chest.size() < 16) {
                    community_chest.push_back("jf");
                }
                seller.jf -= 1;
            }

            // izsole
            for (auto property_nr : seller.ipasumi) {
                auto [buyer, prasita_cena] = izsole(speletaji, property_nr);
                if (buyer) {
                    auto it_buyer = std::find(speletaji.begin(), speletaji.end(), *buyer);
                    if (it_buyer != speletaji.end()) {
                        pardosana(seller, property_nr, *buyer, prasita_cena, ipasumi);
                        replace_sp(speletaji, seller);
                        replace_sp(speletaji, *buyer);
                    }
                }
            }
            for (auto property_nr : seller.kilas) {
                auto [buyer, prasita_cena] = izsole(speletaji, property_nr);
                if (buyer) {
                    auto it_buyer = std::find(speletaji.begin(), speletaji.end(), *buyer);
                    if (it_buyer != speletaji.end()) {
                        pardosana(seller, property_nr, *buyer, prasita_cena, ipasumi);
                        replace_sp(speletaji, seller);
                        replace_sp(speletaji, *buyer);
                    }
                }
            }

            // izņem no spēles
            speletajs = {};
        }

    } 
    replace_sp(speletaji, speletajs);
    return { &speletajs, speletaji, ipasumi, chance, community_chest };
}


// metiens ------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int metiens() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 6);
    return dis(gen);
}


// gajiens ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
std::tuple<std::vector<Player>, Player, int, std::vector<Property>, std::vector<std::string>, std::vector<std::string>> gajiens(
    std::vector<Player>& speletaji, Player& speletajs, int double_count, std::vector<Property>& ipasumi, std::vector<std::string>& chance, std::vector<std::string>& community_chest) {
    // ja jau neeksistē, tad neiet
    if (speletajs.navvalid()) {
        return std::make_tuple(speletaji, speletajs, double_count, ipasumi, chance, community_chest);
    }

    // metieni un saliek dubultos
    int d = 0;
    int pirmais = metiens();
    int otrais = metiens();
    d = (pirmais == otrais) ? 1 : 0;

    if (d == 0) {
        double_count = 0;
    }
    else {
        double_count += 1;
    }

    int rez = pirmais + otrais;

    // ja ir 3 dubultie, tad iet cietumā
    if (double_count == 3) {
        speletajs.laucins = 0;
        return std::make_tuple(speletaji, speletajs, 0, ipasumi, chance, community_chest);
    }

    // ja jau ir cietumā -> izkļūst un paiet vai turpina sēdēt
    if (speletajs.laucins <= 0) {
        speletajs.laucins -= 1;

        // ja uzmeta dubulto, tad tiek ārā
        if (d == 1) {
            speletajs.laucins = 11 + rez;
        } // ja vienkārši sēž cietumā, izmanto jail free card
        else if (speletajs.jf > 0) {
            speletajs.jf -= 1;
            speletajs.laucins = 11 + rez;
        } // ja ir trešā reize, tad maksā un tiek ārā
        else if (speletajs.laucins <= -3) {
            speletajs.nauda -= 500;
            speletajs.laucins = 11 + rez;
            //pārbauda, vai nebankrotē
            if (speletajs.nauda < 0) {
                replace_sp(speletaji, speletajs);
                auto result = nebankrotesana(speletajs, speletaji, ipasumi, chance, community_chest);
                // ja bankrotē, spēle beidzas
                if (std::get<0>(result) == nullptr) {
                    return std::make_tuple(speletaji, speletajs, 0, ipasumi, chance, community_chest);
                }
                else {
                    speletajs = *std::get<0>(result);
                    speletaji = std::get<1>(result);
                    ipasumi = std::get<2>(result);
                    chance = std::get<3>(result);
                    community_chest = std::get<4>(result);
                }
            }
        }
        replace_sp(speletaji, speletajs);
    }
    // ja nav iesaistīti cietuma dzīvē -> paiet
    else if (!speletajs.navvalid()) {
        int jaunais_lauks = speletajs.laucins + rez;
        // starta nauda
        if (jaunais_lauks > 40) {
            jaunais_lauks -= 40;
            speletajs.nauda += 2000;
        }

        speletajs.laucins = jaunais_lauks;
        replace_sp(speletaji, speletajs);
        // ja vēl ir cietumā -> gājiens beidzas
        if (speletajs.laucins <= 0) {
            return std::make_tuple(speletaji, speletajs, 0, ipasumi, chance, community_chest);
        }
    }

    // pluss pārbaude, vai nav nullptr
    if (speletajs.navvalid()) {
        return std::make_tuple(speletaji, speletajs, 0, ipasumi, chance, community_chest);
    }

    // ievāc ziņas par lauciņu, uz kura atrodas
    Property* laucins_info = atrod_pec_nr(speletajs.laucins, ipasumi);
    std::string krasa = laucins_info ? laucins_info->color : "";

    // chance un community chest
    if (krasa == "chance" || krasa == "chest") {
        chance_community(speletaji, speletajs, krasa, chance, community_chest, ipasumi);
        replace_sp(speletaji, speletajs);
        // neļaujam bankrotēt
        if (speletajs.nauda < 0) {
            auto result = nebankrotesana(speletajs, speletaji, ipasumi, chance, community_chest);
            if (std::get<0>(result) == nullptr) {
                return std::make_tuple(speletaji, speletajs, 0, ipasumi, chance, community_chest);
            }
            else {
                speletajs = *std::get<0>(result);
                speletaji = std::get<1>(result);
                ipasumi = std::get<2>(result);
                chance = std::get<3>(result);
                community_chest = std::get<4>(result);
                replace_sp(speletaji, speletajs);
            }
        } // izvērtē lauciņu, uz kura tagad atrodas
        laucins_info = atrod_pec_nr(speletajs.laucins, ipasumi);
        krasa = laucins_info ? laucins_info->color : "";
    }

    // aizsūta uz cietumu
    if (krasa == "go_jail") {
        speletajs.laucins = 0;
        return std::make_tuple(speletaji, speletajs, double_count, ipasumi, chance, community_chest);
    }
    
    // jāmaksā nodokļi nr.1
    else if (krasa == "i_tax") {
        speletajs.nauda -= 2000;
        // neļaujam bankrotēt
        if (speletajs.nauda < 0) {
            replace_sp(speletaji, speletajs);
            auto result = nebankrotesana(speletajs, speletaji, ipasumi, chance, community_chest);
            if (std::get<0>(result) == nullptr) {
                return std::make_tuple(speletaji, speletajs, double_count, ipasumi, chance, community_chest);
            }
            else {
                speletajs = *std::get<0>(result);
                speletaji = std::get<1>(result);
                ipasumi = std::get<2>(result);
                chance = std::get<3>(result);
                community_chest = std::get<4>(result);
            }
            replace_sp(speletaji, speletajs);
        }
    }

    // jāmaksā nodokļi nr.2
    else if (krasa == "s_tax") {
        speletajs.nauda -= 1000;
        // neļaujam bankrotēt
        if (speletajs.nauda < 0) {
            auto result = nebankrotesana(speletajs, speletaji, ipasumi, chance, community_chest);
            replace_sp(speletaji, speletajs);
            if (std::get<0>(result) == nullptr) {
                return std::make_tuple(speletaji, speletajs, double_count, ipasumi, chance, community_chest);
            }
            else {
                speletajs = *std::get<0>(result);
                speletaji = std::get<1>(result);
                ipasumi = std::get<2>(result);
                chance = std::get<3>(result);
                community_chest = std::get<4>(result);
            }
        }replace_sp(speletaji, speletajs);

    }

    // uzkāpts uz īpašuma
    else if (krasa != "free" && krasa != "chance" && krasa != "chest" && krasa != "start" && krasa != "ga_jail" && krasa != "i_tax" && krasa != "s_tax") {
        
        // meklē, vai man pieder
        if (std::find(speletajs.ipasumi.begin(), speletajs.ipasumi.end(), speletajs.laucins) != speletajs.ipasumi.end() ||
            std::find(speletajs.kilas.begin(), speletajs.kilas.end(), speletajs.laucins) != speletajs.kilas.end()) {
            // uzkāpa pats uz sava
        }

        // meklē, vai kādam citam pieder
        else {
            Property* property = atrod_pec_nr(speletajs.laucins, ipasumi);
            int nav = 0;
            int maksa = 0;

            // nosaka, vai ir cita īpašums, ja ir -> samaksā
            for (auto& sp : speletaji) {
                // pārbauda, vai ir kāda cita ķīla
                if (std::find(sp.kilas.begin(), sp.kilas.end(), speletajs.laucins) != sp.kilas.end()) {
                    // par ieķīlātiem īpašumiem nemaksā
                }

                // pārbauda, vai kādam citam pieder
                else if (std::find(sp.ipasumi.begin(), sp.ipasumi.end(), speletajs.laucins) != sp.ipasumi.end()) {
                    // nosaka, kāda ir īres maksa
                    if (property->color == "station") { // stacija
                        maksa = 250 * property->same;
                    } 

                    else if (property->color == "utility") { // utilities
                        int k = (property->same == 1) ? 4 : 10;
                        maksa = k * rez * 10;
                    } 

                    // īpasumi parastie
                    else { 
                        int kop_same = std::count_if(ipasumi.begin(), ipasumi.end(), [&property](const Property& ip) {return ip.color == property->color;  });
                        // ir visi īpašumi
                        if (kop_same == property->same) {
                            switch (property->houses) {
                            case 0:
                                maksa = property->rent * 2;
                                break;
                            case 1:
                                maksa = property->h1;
                                break;
                            case 2:
                                maksa = property->h2;
                                break;
                            case 3:
                                maksa = property->h3;
                                break;
                            case 4:
                                maksa = property->h4;
                                break;
                            case 5:
                                maksa = property->hotel;
                                break;

                            }
                        }

                        // nav visu īpašumu
                        else {
                            maksa = property->rent;
                        }
                    }

                    // samaksā
                    speletajs.nauda -= maksa;
                    sp.nauda += maksa;
                }
                
                // nevienam nepieder
                else {
                    nav += 1;
                }
                replace_sp(speletaji, speletajs);
                replace_sp(speletaji, sp);

            }

            // pārbauda, vai nebankrotēja
            if (speletajs.nauda < 0) {
                auto result = nebankrotesana(speletajs, speletaji, ipasumi, chance, community_chest);
                if (std::get<0>(result) == nullptr) {
                    return std::make_tuple(speletaji, speletajs, double_count, ipasumi, chance, community_chest);
                }
                else {
                    speletajs = *std::get<0>(result);
                    speletaji = std::get<1>(result);
                    ipasumi = std::get<2>(result);
                    chance = std::get<3>(result);
                    community_chest = std::get<4>(result);
                    replace_sp(speletaji, speletajs);
                }
            }

            // ja nevienam nepieder
            if (speletaji.size() == nav && property != nullptr) {

                //neeksistējoši neturpina spēli
                if (&speletajs == nullptr) {
                    return std::make_tuple(speletaji, speletajs, 0, ipasumi, chance, community_chest);
                }

                // pārbauda, vai var atļauties nopirkt -> banka pārdod
                if (speletajs.nauda >= property->price) {
                    Player banka = { 1, property->price, {property->nr}, {}, 0, {} };
                    pardosana(banka, property->nr, speletajs, property->price, ipasumi);
                }

                // pārbauda, vai īpašums ir mērķis
                else if (std::find(speletajs.merkis.begin(), speletajs.merkis.end(), speletajs.laucins) != speletajs.merkis.end()) {
                    // izveido testa spēlētāju, kurš centīsies nabankrotēt, ja tomēr nopirks
                    auto speletajs_tests = speletajs;
                    auto speletaji_tests = speletaji;
                    auto ipasumi_tests = ipasumi;
                    auto community_chest_tests = community_chest;
                    auto chance_tests = chance;

                    auto it = std::find(speletaji_tests.begin(), speletaji_tests.end(), speletajs_tests);
                    if (it != speletaji_tests.end()) {
                        int index = std::distance(speletaji_tests.begin(), it); // nosaka indeksu

                        // izveido banku, kura pārdos
                        Player banka = { 1, property->price, {property->nr}, {}, 0, {} };
                        // banka aizdod naudu pirkšanai, nopērk, atņem naudu ->
                        // -> skatās, vai var nopirkt un nebankrotēt

                        speletajs_tests.nauda += property->price;
                        pardosana(banka, property->nr, speletajs_tests, property->price, ipasumi_tests);
                        speletajs_tests.nauda -= property->price;
                        speletaji_tests[index] = speletajs_tests;

                        // nepārdod tikko iegūto īpašumu vai mērķus
                        std::vector<int> neaizskaramie_i, neaizskaramie_k;
                        for (int nr : speletajs_tests.merkis) {
                            if (std::find(speletajs_tests.ipasumi.begin(), speletajs_tests.ipasumi.end(), nr) != speletajs_tests.ipasumi.end()) {
                                speletajs_tests.ipasumi.erase(std::remove(speletajs_tests.ipasumi.begin(), speletajs_tests.ipasumi.end(), nr), speletajs_tests.ipasumi.end());
                                neaizskaramie_i.push_back(nr);
                            }

                            if (std::find(speletajs_tests.kilas.begin(), speletajs_tests.kilas.end(), nr) != speletajs_tests.kilas.end()) {
                                speletajs_tests.kilas.erase(std::remove(speletajs_tests.kilas.begin(), speletajs_tests.kilas.end(), nr), speletajs_tests.kilas.end());
                                neaizskaramie_k.push_back(nr);
                            }

                        }

                        auto result = nebankrotesana(speletajs, speletaji, ipasumi, chance, community_chest);

                        // ja bankrotēja -> nadabūs
                        if (std::get<0>(result) == nullptr) {
                        }
                        // ja nebankrotēja
                        else {
                            speletajs = *std::get<0>(result);
                            speletaji = std::get<1>(result);
                            ipasumi = std::get<2>(result);
                            chance = std::get<3>(result);
                            community_chest = std::get<4>(result);

                            // atdod īpašumus un ķīlas
                            for (int ip : neaizskaramie_i) {
                                speletajs.ipasumi.push_back(ip);
                            }
                            for (int ip : neaizskaramie_k) {
                                speletajs.kilas.push_back(ip);
                            }
                            replace_sp(speletaji, speletajs);
                        }
                        
                    }
                }
            }

        }
    }

    // kāpšana ir beigusies
    // atsakās no bankrota
    if (speletajs.nauda < 0) {
        replace_sp(speletaji, speletajs);
        auto result = nebankrotesana(speletajs, speletaji, ipasumi, chance, community_chest);
        if (std::get<0>(result) == nullptr) {
            return std::make_tuple(speletaji, speletajs, double_count, ipasumi, chance, community_chest);
        }
        else {
            speletajs = *std::get<0>(result);
            speletaji = std::get<1>(result);
            ipasumi = std::get<2>(result);
            chance = std::get<3>(result);
            community_chest = std::get<4>(result);
            replace_sp(speletaji, speletajs);
        }
    }

    // papilddarbības ----------------------------------------------------------
    if (!speletajs.navvalid() && speletajs.nauda > 0) {
        std::vector<std::string> iespejas = { "atkilat", "majas", "iepirkumi", "nekas"};
        std::string aktivitate = iespejas[rand() % iespejas.size()];

        std::random_device rd;
        std::mt19937 g(rd());

        if (aktivitate == "atkilat") {
            if (!speletajs.kilas.empty()) {
                auto kilas_nr = speletajs.kilas;
                std::shuffle(kilas_nr.begin(), kilas_nr.end(), g);

                for (int kila_nr : kilas_nr) {
                    Property* kila = atrod_pec_nr(kila_nr, ipasumi);
                    if (speletajs.nauda >= kila->mortgage * 1.1 + 100) {
                        speletajs.nauda -= 1.1 * kila->mortgage;
                        speletajs.kilas.erase(std::remove(speletajs.kilas.begin(), speletajs.kilas.end(), kila_nr), speletajs.kilas.end());
                        speletajs.ipasumi.push_back(kila_nr);
                        replace_sp(speletaji, speletajs);
                    }
                }
            }
        }

        else if (aktivitate == "majas") {
            // uzzina, cik viesnīcu un māju ir kopā
            int kop_majas = std::accumulate(ipasumi.begin(), ipasumi.end(), 0, [](int acc, const Property& ip) {return acc + (ip.houses != 5 ? ip.houses : 0);  });
            int kop_viesnicas = std::accumulate(ipasumi.begin(), ipasumi.end(), 0, [](int acc, const Property& ip) {return ip.houses == 5 ? 1:0; });

            std::unordered_map<std::string, int> kop_same_visi;
            for (const auto& ip : ipasumi) {
                kop_same_visi[ip.color]++;
            }

            std::vector<int> apbuvejami;
            for (int ip_nr : speletajs.ipasumi) {
                Property* ip = atrod_pec_nr(ip_nr, ipasumi);
                // atrod apbūvju kandidātus
                if (ip != nullptr && ip->h1 > 0 && ip->same == kop_same_visi[ip->color] && ip->houses != 5) {
                    apbuvejami.push_back(ip_nr);
                }
            }

            if (!apbuvejami.empty()) {
                std::shuffle(apbuvejami.begin(), apbuvejami.end(), g);
                for (int apbuve_nr : apbuvejami) {
                    Property* apbuve = atrod_pec_nr(apbuve_nr, ipasumi);
                    if (apbuve->color != "utility" && apbuve->color != "station") {
                        // viesnīca
                        if (apbuve->houses == 4 && speletajs.nauda >= 1.5 * apbuve->h_cost && kop_viesnicas < 12) {
                            speletajs.nauda -= apbuve->h_cost;
                            apbuve->houses++;
                        }
                        // māja
                        else if (apbuve->houses < 4 && speletajs.nauda >= 1.5 * apbuve->h_cost && kop_majas < 32) {
                            speletajs.nauda -= apbuve->h_cost;
                            apbuve->houses++;
                        }

                    }
                }

            }
            replace_sp(speletaji, speletajs);
        }   

        else if (aktivitate == "iepirkumi") {
            // cenšas iegādāties īpašumu no citiem spēlētājiem
            bool done = false;
            // no mērķa
            for (int sp_nr : speletajs.merkis) {
                for (auto& cits : speletaji) {
                    if (cits != speletajs) {
                        // apskata cita īpašumus
                        for (int cits_nr : cits.ipasumi) {
                            if (cits_nr == sp_nr && done == false) {
                                // noskaidro cenu
                                int k = 1;
                                Property* karotais_ip = atrod_pec_nr(cits_nr, ipasumi);
                                // vai ir citi īpasumi?
                                k *= karotais_ip->same;
                                // vai ir mājas ??
                                k *= karotais_ip->houses + 1;

                                int cena = 2 * k * karotais_ip->price;
                                if (cena <= speletajs.nauda) {
                                    pardosana(cits, cits_nr, speletajs, cena, ipasumi);
                                    replace_sp(speletaji, speletajs);
                                    replace_sp(speletaji, cits);
                                    done = true;
                                    break;
                                }
                            }
                        }

                        // apskata cita ķīlas
                        for (int cits_nr : cits.kilas) {
                            if (cits_nr == sp_nr && done == false) {
                                // noskaidro cenu
                                int k = 1;
                                Property* karotais_ip = atrod_pec_nr(cits_nr, ipasumi);
                                // vai ir citi ipasumi?
                                int cena = 2 * karotais_ip->same * karotais_ip->price;
                                if (cena <= speletajs.nauda) {
                                    pardosana(cits, cits_nr, speletajs, cena, ipasumi);
                                    replace_sp(speletaji, speletajs);
                                    replace_sp(speletaji, cits);
                                    done = true;
                                    break;
                                }
                            }
                        }
                    } if (done) break;
                } if (done) break;
            }

            // no same 
            if (!done) {
                // (ip/ip)
                for (int sp_nr : speletajs.ipasumi) {
                    Property* sp_ip = atrod_pec_nr(sp_nr, ipasumi);
                    for (auto& cits : speletaji) {
                        if (speletajs != cits ) {
                            for (int cits_nr : cits.ipasumi) {
                                Property* cits_ip = atrod_pec_nr(cits_nr, ipasumi);
                                if (cits_ip->color == sp_ip->color) {
                                    int cena = 2 * cits_ip->same * (cits_ip->houses + 1) * cits_ip->price;
                                    if (cena <= speletajs.nauda) {
                                        pardosana(cits, cits_nr, speletajs, cena, ipasumi);
                                        replace_sp(speletaji, speletajs);
                                        replace_sp(speletaji, cits);
                                        done = true;
                                        break;
                                    }
                                }
                            }
                        }

                        if (done) break;
                    } if (done) break;
                }

                // (ip/k)
                for (int sp_nr : speletajs.ipasumi) {
                    Property* sp_ip = atrod_pec_nr(sp_nr, ipasumi);
                    for (auto& cits : speletaji) {
                        if (speletajs != cits) {
                            for (int cits_nr : cits.kilas) {
                                Property* cits_ip = atrod_pec_nr(cits_nr, ipasumi);
                                if (cits_ip->color == sp_ip->color) {
                                    int cena = 2 * cits_ip->same * (cits_ip->houses + 1) * cits_ip->price;
                                    if (cena <= speletajs.nauda) {
                                        pardosana(cits, cits_nr, speletajs, cena, ipasumi);
                                        replace_sp(speletaji, speletajs);
                                        replace_sp(speletaji, cits);
                                        done = true;
                                        break;
                                    }
                                }
                            }
                        }

                        if (done) break;
                    } if (done) break;
                }

                // (k/ip)
                for (int sp_nr : speletajs.kilas) {
                    Property* sp_ip = atrod_pec_nr(sp_nr, ipasumi);
                    for (auto& cits : speletaji) {
                        if (speletajs != cits) {
                            for (int cits_nr : cits.ipasumi) {
                                Property* cits_ip = atrod_pec_nr(cits_nr, ipasumi);
                                if (cits_ip->color == sp_ip->color) {
                                    int cena = 2 * cits_ip->same * (cits_ip->houses + 1) * cits_ip->price;
                                    if (cena <= speletajs.nauda) {
                                        pardosana(cits, cits_nr, speletajs, cena, ipasumi);
                                        replace_sp(speletaji, speletajs);
                                        replace_sp(speletaji, cits);
                                        done = true;
                                        break;
                                    }
                                }
                            }
                        }

                        if (done) break;
                    } if (done) break;
                }

                // (k/k)
                for (int sp_nr : speletajs.kilas) {
                    Property* sp_ip = atrod_pec_nr(sp_nr, ipasumi);
                    for (auto& cits : speletaji) {
                        if (speletajs != cits) {
                            for (int cits_nr : cits.kilas) {
                                Property* cits_ip = atrod_pec_nr(cits_nr, ipasumi);
                                if (cits_ip->color == sp_ip->color) {
                                    int cena = 2 * cits_ip->same * (cits_ip->houses + 1) * cits_ip->price;
                                    if (cena <= speletajs.nauda) {
                                        pardosana(cits, cits_nr, speletajs, cena, ipasumi);
                                        replace_sp(speletaji, speletajs);
                                        replace_sp(speletaji, cits);
                                        done = true;
                                        break;
                                    }
                                }
                            }
                        }

                        if (done) break;
                    } if (done) break;
                }
            }
        }
    }

    // nebeidz ar parādiem, pēdējā pārbaude
    if (speletajs.nauda < 0) {
        auto result = nebankrotesana(speletajs, speletaji, ipasumi, chance, community_chest);
        if (std::get<0>(result) == nullptr) {
            return std::make_tuple(speletaji, speletajs, 0, ipasumi, chance, community_chest);
        }
        else {
            speletajs = *std::get<0>(result);
            speletaji = std::get<1>(result);
            ipasumi = std::get<2>(result);
            chance = std::get<3>(result);
            community_chest = std::get<4>(result);
            replace_sp(speletaji, speletajs);
        }
    }

    // ja ir dubults -> iet vēlreiz
    if (&speletajs!= nullptr && double_count > 0) {
        return gajiens(speletaji, speletajs, double_count, ipasumi, chance, community_chest);
    }
 
    return std::make_tuple(speletaji, speletajs, double_count, ipasumi, chance, community_chest);
}


// spēle --------------------------------------------------------------------------------------------------------------------------------------------------------------------------

std::string spele(const std::vector<std::vector<int>>& merki) {
    // samaisa kārtis
    std::vector<std::string> chance_kartites = { "jf", "-150", "+1500", "+500", "-3s", "l_utility", "l_25", "l_station", "l_0", "l_12", "l_40", "l_6", "l_1", "l_station", "tax_ch", "pl+500" };
    std::vector<std::string> community_chest_kartites = { "pl-100", "+100", "+100", "+200", "+250", "+1000", "+1000", "+500", "+2000", "l_1", "l_0", "jf", "tax_com", "-500", "-500", "-1000" };


    std::random_device rd;
    std::mt19937 g(rd());
    std::vector<std::string> chance = chance_kartites;
    std::vector<std::string> community_chest = community_chest_kartites;

    std::vector<Property> ipasumi = {
    {2, 600, 20, 100, 300, 900, 1600, 2500, 500, 300, "brown", 0, 0},
    {4, 600, 40, 200, 600, 1800, 3200, 4500, 500, 300, "brown", 0, 0},
    {7, 1000, 60, 300, 900, 2700, 4000, 5500, 500, 500, "l_blue", 0, 0},
    {9, 1000, 60, 300, 900, 2700, 4000, 5500, 500, 500, "l_blue", 0, 0},
    {10, 1200, 80, 400, 1000, 3000, 4500, 6000, 500, 600, "l_blue", 0, 0},
    {12, 1400, 100, 500, 1500, 4500, 6250, 7500, 1000, 700, "pink", 0, 0},
    {14, 1400, 100, 500, 1500, 4500, 6250, 7500, 1000, 700, "pink", 0, 0},
    {15, 1600, 120, 600, 1800, 5000, 7000, 9000, 1000, 800, "pink", 0, 0},
    {17, 1800, 140, 700, 2000, 5500, 7500, 9500, 1000, 900, "orange", 0, 0},
    {19, 1800, 140, 700, 2000, 5500, 7500, 9500, 1000, 900, "orange", 0, 0},
    {20, 2000, 160, 800, 2200, 6000, 8000, 10000, 1000, 1000, "orange", 0, 0},
    {22, 2200, 180, 900, 2500, 7000, 8750, 10500, 1500, 1100, "red", 0, 0},
    {24, 2200, 180, 900, 2500, 7000, 8750, 10500, 1500, 1100, "red", 0, 0},
    {25, 2400, 200, 1000, 3000, 7500, 9250, 11000, 1500, 1200, "red", 0, 0},
    {27, 2600, 220, 1100, 3300, 8000, 9750, 11500, 1500, 1300, "yellow", 0, 0},
    {28, 2600, 220, 1100, 3300, 8000, 9750, 11500, 1500, 1300, "yellow", 0, 0},
    {30, 2800, 240, 1200, 3600, 8500, 10250, 12000, 1500, 1400, "yellow", 0, 0},
    {32, 3000, 260, 1300, 3900, 9000, 11000, 12750, 2000, 1500, "green", 0, 0},
    {33, 3000, 260, 1300, 3900, 9000, 11000, 12750, 2000, 1500, "green", 0, 0},
    {35, 3200, 280, 1500, 4500, 10000, 12000, 14000, 2000, 1600, "green", 0, 0},
    {38, 3500, 350, 1750, 5000, 11000, 13000, 15000, 2000, 1750, "d_blue", 0, 0},
    {40, 4000, 500, 2000, 6000, 14000, 17000, 20000, 2000, 2000, "d_blue", 0, 0},
    {13, 1500,   0,    0,    0,     0,     0,     0,    0,  750, "utility", 0, 0},
    {29, 1500,   0,    0,    0,     0,     0,     0,    0,  750, "utility", 0, 0},
    {6, 2000, 0, 0, 0, 0, 0, 0, 1000, 0, "station", 0, 0},
    {16, 2000, 0, 0, 0, 0, 0, 0, 1000, 0, "station", 0, 0},
    {26, 2000, 0, 0, 0, 0, 0, 0, 1000, 0, "station", 0, 0},
    {36, 2000, 0, 0, 0, 0, 0, 0, 1000, 0, "station", 0, 0},
    {8, 0, 0, 0, 0, 0, 0, 0, 0, 0, "chance", 0, 0},
    {23, 0, 0, 0, 0, 0, 0, 0, 0, 0, "chance", 0, 0},
    {37, 0, 0, 0, 0, 0, 0, 0, 0, 0, "chance", 0, 0},
    {3, 0, 0, 0, 0, 0, 0, 0, 0, 0, "chest", 0, 0},
    {18, 0, 0, 0, 0, 0, 0, 0, 0, 0, "chest", 0, 0},
    {34, 0, 0, 0, 0, 0, 0, 0, 0, 0, "chest", 0, 0},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, "start", 0, 0},
    {11, 0, 0, 0, 0, 0, 0, 0, 0, 0, "free", 0, 0},
    {21, 0, 0, 0, 0, 0, 0, 0, 0, 0, "free", 0, 0},
    {5, 0, 0, 0, 0, 0, 0, 0, 0, 0, "i_tax", 0, 0},
    {39, 0, 0, 0, 0, 0, 0, 0, 0, 0, "s_tax", 0, 0},
    {31, 0, 0, 0, 0, 0, 0, 0, 0, 0, "go_jail", 0, 0}
    };


    std::shuffle(chance.begin(), chance.end(), g);
    std::shuffle(community_chest.begin(), community_chest.end(), g);

    // izveido spēlētajus
    std::vector<Player> speletaji;
    std::vector<std::string> vardi = { "es", "Elis", "tetis", "mamma" };

    for (size_t i = 0; i < vardi.size() && i < merki.size(); i++) {
        speletaji.push_back(Player{ 1, 1500, {}, {}, 0, merki[i], vardi[i] });
    }

    // izvēlas secību
    std::shuffle(speletaji.begin(), speletaji.end(), g);

    // spēles gājieni
    while (speletaji.size() > 1) {
        for (size_t i = 0; i < speletaji.size(); ++i) {
            Player& speletajs = speletaji[i];
            int ddd = 0;
            std::tie(speletaji, speletajs, ddd, ipasumi, chance, community_chest) = gajiens(speletaji, speletajs, 0, ipasumi, chance, community_chest);
            replace_sp(speletaji, speletajs);
        }
        std::cout << speletaji.size() << std::endl;
    }
    return speletaji[0].vards;
}


// saraksti citam fun
std::vector<std::string> chance_kartites = { "jf", "-150", "+1500", "+500", "-3s", "l_utility", "l_25", "l_station", "l_0", "l_12", "l_40", "l_6", "l_1", "l_station", "tax_ch", "pl+500" };
std::vector<std::string> community_chest_kartites = { "pl-100", "+100", "+100", "+200", "+250", "+1000", "+1000", "+500", "+2000", "l_1", "l_0", "jf", "tax_com", "-500", "-500", "-1000" };


std::vector<Property> ipasumi = {
{2, 600, 20, 100, 300, 900, 1600, 2500, 500, 300, "brown", 0, 0},
{4, 600, 40, 200, 600, 1800, 3200, 4500, 500, 300, "brown", 0, 0},
{7, 1000, 60, 300, 900, 2700, 4000, 5500, 500, 500, "l_blue", 0, 0},
{9, 1000, 60, 300, 900, 2700, 4000, 5500, 500, 500, "l_blue", 0, 0},
{10, 1200, 80, 400, 1000, 3000, 4500, 6000, 500, 600, "l_blue", 0, 0},
{12, 1400, 100, 500, 1500, 4500, 6250, 7500, 1000, 700, "pink", 0, 0},
{14, 1400, 100, 500, 1500, 4500, 6250, 7500, 1000, 700, "pink", 0, 0},
{15, 1600, 120, 600, 1800, 5000, 7000, 9000, 1000, 800, "pink", 0, 0},
{17, 1800, 140, 700, 2000, 5500, 7500, 9500, 1000, 900, "orange", 0, 0},
{19, 1800, 140, 700, 2000, 5500, 7500, 9500, 1000, 900, "orange", 0, 0},
{20, 2000, 160, 800, 2200, 6000, 8000, 10000, 1000, 1000, "orange", 0, 0},
{22, 2200, 180, 900, 2500, 7000, 8750, 10500, 1500, 1100, "red", 0, 0},
{24, 2200, 180, 900, 2500, 7000, 8750, 10500, 1500, 1100, "red", 0, 0},
{25, 2400, 200, 1000, 3000, 7500, 9250, 11000, 1500, 1200, "red", 0, 0},
{27, 2600, 220, 1100, 3300, 8000, 9750, 11500, 1500, 1300, "yellow", 0, 0},
{28, 2600, 220, 1100, 3300, 8000, 9750, 11500, 1500, 1300, "yellow", 0, 0},
{30, 2800, 240, 1200, 3600, 8500, 10250, 12000, 1500, 1400, "yellow", 0, 0},
{32, 3000, 260, 1300, 3900, 9000, 11000, 12750, 2000, 1500, "green", 0, 0},
{33, 3000, 260, 1300, 3900, 9000, 11000, 12750, 2000, 1500, "green", 0, 0},
{35, 3200, 280, 1500, 4500, 10000, 12000, 14000, 2000, 1600, "green", 0, 0},
{38, 3500, 350, 1750, 5000, 11000, 13000, 15000, 2000, 1750, "d_blue", 0, 0},
{40, 4000, 500, 2000, 6000, 14000, 17000, 20000, 2000, 2000, "d_blue", 0, 0},
{13, 1500,   0,    0,    0,     0,     0,     0,    0,  750, "utility", 0, 0},
{29, 1500,   0,    0,    0,     0,     0,     0,    0,  750, "utility", 0, 0},
{6, 2000, 0, 0, 0, 0, 0, 0, 1000, 0, "station", 0, 0},
{16, 2000, 0, 0, 0, 0, 0, 0, 1000, 0, "station", 0, 0},
{26, 2000, 0, 0, 0, 0, 0, 0, 1000, 0, "station", 0, 0},
{36, 2000, 0, 0, 0, 0, 0, 0, 1000, 0, "station", 0, 0},
{8, 0, 0, 0, 0, 0, 0, 0, 0, 0, "chance", 0, 0},
{23, 0, 0, 0, 0, 0, 0, 0, 0, 0, "chance", 0, 0},
{37, 0, 0, 0, 0, 0, 0, 0, 0, 0, "chance", 0, 0},
{3, 0, 0, 0, 0, 0, 0, 0, 0, 0, "chest", 0, 0},
{18, 0, 0, 0, 0, 0, 0, 0, 0, 0, "chest", 0, 0},
{34, 0, 0, 0, 0, 0, 0, 0, 0, 0, "chest", 0, 0},
{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, "start", 0, 0},
{11, 0, 0, 0, 0, 0, 0, 0, 0, 0, "free", 0, 0},
{21, 0, 0, 0, 0, 0, 0, 0, 0, 0, "free", 0, 0},
{5, 0, 0, 0, 0, 0, 0, 0, 0, 0, "i_tax", 0, 0},
{39, 0, 0, 0, 0, 0, 0, 0, 0, 0, "s_tax", 0, 0},
{31, 0, 0, 0, 0, 0, 0, 0, 0, 0, "go_jail", 0, 0}
};


int main() {
    Player sp1 = { 0, 100000,{}, {}, 0,{}, "es" };
    Player sp2 = { 0, 100000,{13}, {}, 0,{}, "tu" };
    std::vector<Player> spi = { sp1, sp2, {}};
    Player nav = {};
    //chance_community(spi, nav, "chance", chance_kartites, community_chest_kartites, ipasumi);
    //pardosana(sp1, 13, nav, 13, ipasumi);
    //std::pair<Player*, int> dabu = izsole(spi, 13);
    //std::cout << "te" << "\n";
    //Player* playerPtr = dabu.first;
    //int value = dabu.second;
    //if (playerPtr != nullptr) {
        //std::cout << "Player: " << playerPtr->vards << "\n";
    //}
    //else {
        //std::cout << "Player: nullptr" << "\n";
    //}
    //std::cout << "Value: " << value << "\n";
    //std::tuple<vector<Player>, Player, vector<Property>> rez = naudas_ieguve(nav, spi, ipasumi);
    //std::tuple<Player*, std::vector<Player>, std::vector<Property>, std::vector<std::string>, std::vector<std::string>> rez = nebankrotesana(nav, spi, ipasumi, chance_kartites, community_chest_kartites);
    //std::tuple<std::vector<Player>, Player, int, std::vector<Property>, std::vector<std::string>, std::vector<std::string>> rez = gajiens(spi, sp1, 0, ipasumi, chance_kartites, community_chest_kartites);
    std::string rez = spele({ {13, 3}, {2, 4}, {} });
    
    std::cout << "viss iet, spele neiet :(" << "\n";
    std::cout << rez << "\n";



    return 0;
}
