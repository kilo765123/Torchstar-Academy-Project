#include <emscripten.h>
#include <string>
#include <vector>
#include <random>
#include <algorithm>

#define EXPORT EMSCRIPTEN_KEEPALIVE

struct Question { 
    std::string query, options[4], answer, hint; 
    int difficulty; 
    bool hiddenMask[4]; 
};

struct Boss { 
    std::string name, taunt; 
    int maxHP; 
    std::vector<Question> pool; 
    bool isSpecial = false; 
};

struct Player { 
    int level = 1, currentHP = 200, maxHP = 200, baseAttack = 30, atkLevel = 1, sp = 3, money = 100, luckLevel = 1;
    int location = 3; 
};

Player p;
Boss currentBoss;
Question curQ;
int m_currentHP;
std::string state = "menu"; 
int currentDifficulty = 0; 
std::vector<Boss> schoolWings;
std::vector<Boss> specialBosses;

int last_dmg_to_boss = 0;
int last_dmg_to_player = 0;
bool last_hit_crit = false;
int final_score = 0;

std::mt19937 rng;

void load_new_question() {
    if (currentBoss.pool.empty()) return;
    std::vector<Question> validPool;
    for (size_t i = 0; i < currentBoss.pool.size(); i++) {
        if (currentBoss.pool[i].difficulty == currentDifficulty) {
            validPool.push_back(currentBoss.pool[i]);
        }
    }
    if (validPool.empty()) validPool = currentBoss.pool; 
    curQ = validPool[rng() % validPool.size()];
    std::shuffle(curQ.options, curQ.options + 4, rng);
    for(int i=0; i<4; i++) curQ.hiddenMask[i] = false;
}

void init_data() {
    schoolWings.clear(); specialBosses.clear();

    // Added proper Titles and Names
    schoolWings.push_back({"Dr. Herodotus (Dekan Sosial)", "Sejarah mencatat pemenang!", 400, {
        {"Ibukota Australia?", {"Canberra", "Sydney", "Melbourne", "Perth"}, "Canberra", "Bukan Sydney.", 0, {false,false,false,false}},
        {"Benua Terbesar?", {"Asia", "Afrika", "Eropa", "Amerika"}, "Asia", "Kita disini.", 0, {false,false,false,false}},
        {"Mata uang Jepang?", {"Yen", "Won", "Baht", "Dollar"}, "Yen", "Matahari Terbit.", 0, {false,false,false,false}},
        {"Perang Dunia II berakhir?", {"1945", "1944", "1939", "1950"}, "1945", "Tahun Merdeka RI.", 0, {false,false,false,false}},
        {"Penulis 'Habis Gelap Terbitlah Terang'?", {"Kartini", "Cut Nyak Dien", "Dewi Sartika", "Fatmawati"}, "Kartini", "Pahlawan Wanita.", 0, {false,false,false,false}},
        {"Negara Tirai Bambu?", {"China", "Jepang", "Korea", "Vietnam"}, "China", "Panda.", 0, {false,false,false,false}},
        {"Kerajaan Hindu tertua di Indonesia?", {"Kutai", "Tarumanegara", "Majapahit", "Sriwijaya"}, "Kutai", "Kalimantan Timur.", 0, {false,false,false,false}},
        {"Sumpah Pemuda tahun?", {"1928", "1908", "1945", "1930"}, "1928", "28 Oktober.", 0, {false,false,false,false}},
        {"Presiden RI ke-3?", {"Habibie", "Gus Dur", "Megawati", "Soeharto"}, "Habibie", "Pesawat Terbang.", 0, {false,false,false,false}},
        {"Laut terluas di dunia?", {"Pasifik", "Atlantik", "Hindia", "Arktik"}, "Pasifik", "Tenang.", 0, {false,false,false,false}},
        
        {"Siapa ekonom yang mencetuskan teori keunggulan komparatif?", {"David Ricardo", "Adam Smith", "Karl Marx", "J.M. Keynes"}, "David Ricardo", "Buku On the Principles of Political Economy and Taxation (1817).", 1, {false,false,false,false}},
        {"Peradaban kuno di antara sungai Eufrat dan Tigris?", {"Mesopotamia", "Mesir Kuno", "Lembah Indus", "Inca"}, "Mesopotamia", "Berasal dari bahasa Yunani yang berarti 'di antara sungai-sungai'.", 1, {false,false,false,false}},
        {"Kurva Phillips menggambarkan hubungan terbalik antara tingkat pengangguran dan?", {"Inflasi", "Suku Bunga", "Pertumbuhan Ekonomi", "Pajak"}, "Inflasi", "Saat pengangguran rendah, harga-harga cenderung naik cepat.", 1, {false,false,false,false}},
        {"Organisasi perdagangan bebas di kawasan Amerika Utara?", {"NAFTA", "AFTA", "EU", "OPEC"}, "NAFTA", "Beranggotakan Amerika Serikat, Kanada, dan Meksiko.", 1, {false,false,false,false}},
        {"Sistem tanam paksa (Cultuurstelsel) di Indonesia dicetuskan oleh?", {"Van den Bosch", "Daendels", "Raffles", "Cornelis de Houtman"}, "Van den Bosch", "Diterapkan pada tahun 1830 untuk mengisi kas Belanda yang kosong.", 1, {false,false,false,false}},
        {"Pemberontakan Taiping yang memakan jutaan korban jiwa terjadi di negara?", {"Tiongkok", "Jepang", "Rusia", "Vietnam"}, "Tiongkok", "Dipimpin oleh Hong Xiuquan pada masa Dinasti Qing.", 1, {false,false,false,false}},
        {"Fenomena naiknya air dingin dan kaya nutrisi dari dasar laut ke permukaan disebut?", {"Upwelling", "Downwelling", "El Nino", "La Nina"}, "Upwelling", "Sangat menguntungkan bagi tangkapan nelayan.", 1, {false,false,false,false}},
        {"Bentuk muka bumi hasil deposisi material oleh angin di daerah gurun disebut?", {"Sand dune", "Fjord", "Moraine", "Stalaktit"}, "Sand dune", "Biasa disebut juga gumuk pasir.", 1, {false,false,false,false}},
        {"Jenis inflasi parah yang membuat masyarakat kehilangan kepercayaan pada uang disebut?", {"Hiperinflasi", "Deflasi", "Stagflasi", "Devaluasi"}, "Hiperinflasi", "Pernah terjadi di Zimbabwe dan Jerman pasca PD 1.", 1, {false,false,false,false}},
        {"Kerajaan bercorak Buddha terbesar di Nusantara yang berpusat di Sumatera?", {"Sriwijaya", "Majapahit", "Mataram Kuno", "Tarumanegara"}, "Sriwijaya", "Terkenal sebagai pusat pembelajaran agama Buddha dan maritim.", 1, {false,false,false,false}}
    }});

    schoolWings.push_back({"Prof. Al-Khawarizmi (Master Kalkulus)", "Logika adalah kunci!", 500, {
        {"12 x 12?", {"144", "124", "100", "122"}, "144", "Gros.", 0, {false,false,false,false}},
        {"Akar 81?", {"9", "8", "7", "6"}, "9", "Kuadrat 9.", 0, {false,false,false,false}},
        {"Sudut Siku-siku?", {"90", "180", "45", "60"}, "90", "Tegak lurus.", 0, {false,false,false,false}},
        {"2 + 2 x 5?", {"12", "20", "10", "14"}, "12", "Perkalian dulu.", 0, {false,false,false,false}},
        {"Bilangan Prima terkecil?", {"2", "1", "3", "0"}, "2", "Satu-satunya genap.", 0, {false,false,false,false}},
        {"Rumus Luas Lingkaran?", {"Pi x r x r", "p x l", "s x s", "2 x Pi x r"}, "Pi x r x r", "Ada Pi-nya.", 0, {false,false,false,false}},
        {"Hasil -5 + 8?", {"3", "-3", "13", "-13"}, "3", "Hutang 5 bayar 8.", 0, {false,false,false,false}},
        {"Biner dari 5?", {"101", "110", "100", "111"}, "101", "4+1.", 0, {false,false,false,false}},
        {"Log 100 basis 10?", {"2", "10", "100", "0"}, "2", "10 pangkat berapa?", 0, {false,false,false,false}},
        {"Deret: 2, 4, 8, ...?", {"16", "12", "10", "14"}, "16", "Dikali 2.", 0, {false,false,false,false}},

        {"Turunan pertama dari f(x) = sin(2x) adalah?", {"2 cos(2x)", "cos(2x)", "-2 cos(2x)", "sin(2x)"}, "2 cos(2x)", "Gunakan aturan rantai (chain rule). Turunan sin(u) adalah cos(u) * u'.", 1, {false,false,false,false}},
        {"Nilai lim (x->0) (sin 3x) / x adalah?", {"3", "1/3", "1", "0"}, "3", "Gunakan sifat limit trigonometri dasar: lim(x->0) sin(ax)/bx = a/b.", 1, {false,false,false,false}},
        {"Jika matriks A singular, maka nilai determinannya adalah?", {"0", "1", "-1", "Tak Terhingga"}, "0", "Matriks singular tidak memiliki invers karena determinannya nol.", 1, {false,false,false,false}},
        {"Berapa banyak susunan huruf dari kata 'MATI'?", {"24", "12", "6", "120"}, "24", "Permutasi dari 4 huruf berbeda: 4! = 4 x 3 x 2 x 1.", 1, {false,false,false,false}},
        {"Jika log 2 = 0.3 dan log 3 = 0.47, berapakah log 6?", {"0.77", "0.14", "1.41", "0.60"}, "0.77", "Sifat logaritma: log(a*b) = log a + log b.", 1, {false,false,false,false}},
        {"Integral tak tentu dari 2x dx adalah?", {"x^2 + C", "2x^2 + C", "x + C", "0"}, "x^2 + C", "Gunakan aturan pangkat integral: (a / (n+1)) x^(n+1).", 1, {false,false,false,false}},
        {"Pusat lingkaran dengan persamaan x^2 + y^2 - 4x + 6y - 12 = 0 adalah?", {"(2, -3)", "(-2, 3)", "(4, -6)", "(-4, 6)"}, "(2, -3)", "Bentuk umum: x^2 + y^2 + Ax + By + C = 0. Pusatnya adalah (-A/2, -B/2).", 1, {false,false,false,false}},
        {"Peluang munculnya jumlah mata dadu 7 dari pelemparan dua dadu adalah?", {"1/6", "1/12", "1/36", "7/36"}, "1/6", "Ada 6 kemungkinan: (1,6), (2,5), (3,4), (4,3), (5,2), (6,1) dari total 36.", 1, {false,false,false,false}},
        {"Suku ke-n barisan geometri dengan U1=2 dan rasio=3 adalah?", {"2 x 3^(n-1)", "2 x 3^n", "3 x 2^(n-1)", "2 + 3n"}, "2 x 3^(n-1)", "Rumus suku ke-n barisan geometri: Un = a * r^(n-1).", 1, {false,false,false,false}},
        {"Akar persamaan kuadrat x^2 - 5x + 6 = 0 adalah?", {"2 dan 3", "-2 dan -3", "1 dan 6", "-1 dan -6"}, "2 dan 3", "Cari dua bilangan yang jika ditambahkan hasilnya -5 dan dikalikan hasilnya 6.", 1, {false,false,false,false}}
    }});

    schoolWings.push_back({"Ir. Mendeleev (Kepala Lab Sains)", "Awas meledak!", 550, {
        {"Rumus Air?", {"H2O", "CO2", "O2", "H2SO4"}, "H2O", "Water.", 0, {false,false,false,false}},
        {"Planet Merah?", {"Mars", "Venus", "Jupiter", "Saturnus"}, "Mars", "Dewa Perang.", 0, {false,false,false,false}},
        {"Satuan Daya?", {"Watt", "Joule", "Volt", "Ampere"}, "Watt", "Lampu.", 0, {false,false,false,false}},
        {"Lambang Emas?", {"Au", "Ag", "Fe", "Cu"}, "Au", "Aurum.", 0, {false,false,false,false}},
        {"Pusat Tata Surya?", {"Matahari", "Bumi", "Bulan", "Mars"}, "Matahari", "Bintang terdekat.", 0, {false,false,false,false}},
        {"Hukum Newton I tentang?", {"Kelembaman", "Aksi-Reaksi", "Percepatan", "Gravitasi"}, "Kelembaman", "Malas gerak.", 0, {false,false,false,false}},
        {"pH Air murni?", {"7", "1", "14", "5"}, "7", "Netral.", 0, {false,false,false,false}},
        {"Satuan Tegangan?", {"Volt", "Ampere", "Ohm", "Watt"}, "Volt", "Baterai.", 0, {false,false,false,false}},
        {"Kecepatan cahaya?", {"300.000 km/s", "340 m/s", "100 km/j", "Infinity"}, "300.000 km/s", "Sangat cepat.", 0, {false,false,false,false}},
        {"Hewan menyusui?", {"Mamalia", "Aves", "Reptil", "Amfibi"}, "Mamalia", "Punya kelenjar.", 0, {false,false,false,false}},

        {"Senyawa yang bertindak sebagai donor pasangan elektron bebas menurut Lewis disebut?", {"Basa Lewis", "Asam Lewis", "Asam Arrhenius", "Basa Bronsted"}, "Basa Lewis", "Teori asam-basa Lewis: Asam menerima (akseptor), Basa memberi (donor).", 1, {false,false,false,false}},
        {"Hukum Termodinamika II menyatakan bahwa entropi semesta selalu?", {"Bertambah", "Berkurang", "Konstan", "Nol"}, "Bertambah", "Entropi adalah ukuran ketidakteraturan sistem.", 1, {false,false,false,false}},
        {"Proses pembentukan ATP paling banyak melalui transfer elektron di mitokondria disebut?", {"Fosforilasi Oksidatif", "Glikolisis", "Siklus Krebs", "Fermentasi"}, "Fosforilasi Oksidatif", "Terjadi di membran dalam (krista) mitokondria.", 1, {false,false,false,false}},
        {"Konstanta Planck bernilai sekitar?", {"6.626 x 10^-34 J.s", "3.0 x 10^8 m/s", "9.8 m/s^2", "1.6 x 10^-19 C"}, "6.626 x 10^-34 J.s", "Merupakan konstanta penting dalam teori kuantum cahaya dan rumus E = h.f", 1, {false,false,false,false}},
        {"Organel sel yang berfungsi untuk detoksifikasi racun dan sintesis lipid adalah?", {"Retikulum Endoplasma Halus", "Lisosom", "Badan Golgi", "Mitokondria"}, "Retikulum Endoplasma Halus", "Berbeda dengan RE Kasar, RE Halus tidak dihinggapi ribosom.", 1, {false,false,false,false}},
        {"Hukum yang menyatakan Volume gas sebanding dengan Suhu mutlak pada Tekanan tetap adalah?", {"Hukum Charles", "Hukum Boyle", "Hukum Gay-Lussac", "Hukum Avogadro"}, "Hukum Charles", "Rumusnya V/T = konstan.", 1, {false,false,false,false}},
        {"Hormon tumbuhan yang berbentuk gas dan berfungsi memicu pematangan buah adalah?", {"Gas Etilen", "Auksin", "Giberelin", "Sitokinin"}, "Gas Etilen", "Sering memicu proses klimakterik pada buah seperti pisang dan apel.", 1, {false,false,false,false}},
        {"Peristiwa pelenturan atau pembelokan gelombang cahaya saat melewati celah sempit disebut?", {"Difraksi", "Interferensi", "Polarisasi", "Refraksi"}, "Difraksi", "Dapat dijelaskan menggunakan prinsip Huygens.", 1, {false,false,false,false}},
        {"Partikel subatomik tak bermassa yang menjadi pembawa gaya elektromagnetik adalah?", {"Foton", "Boson W", "Gluon", "Graviton"}, "Foton", "Merupakan kuantum (paket energi) dari radiasi elektromagnetik.", 1, {false,false,false,false}},
        {"Ikatan kimia yang terbentuk akibat pemakaian bersama pasangan elektron oleh dua atom disebut?", {"Ikatan Kovalen", "Ikatan Ionik", "Ikatan Logam", "Ikatan Hidrogen"}, "Ikatan Kovalen", "Umumnya terjadi antara atom-atom non-logam.", 1, {false,false,false,false}}
    }});

    Boss wakasek = {"Drs. Budi (Wakil Kepala Sekolah)", "Tunjukkan prestasimu!", 2500, {}};
    for(auto& b : schoolWings) wakasek.pool.insert(wakasek.pool.end(), b.pool.begin(), b.pool.end());
    specialBosses.push_back(wakasek);

    Boss kasek = {"Dr. Haryanto (Bapak Kepala Sekolah)", "Integritas nomor satu!", 4000, {}};
    for(auto& b : schoolWings) kasek.pool.insert(kasek.pool.end(), b.pool.begin(), b.pool.end());
    specialBosses.push_back(kasek);

    Boss menteri = {"Prof. Nadiem (Menteri Pendidikan)", "Kurikulum Merdeka!", 10000, {}};
    for(auto& b : schoolWings) menteri.pool.insert(menteri.pool.end(), b.pool.begin(), b.pool.end());
    specialBosses.push_back(menteri);
}

extern "C" {
    EXPORT void init_game(unsigned int seed) {
        rng.seed(seed);
        init_data();
        p = Player();
        state = "menu";
    }

    EXPORT void load_save_data(int lvl, int hp, int sp, int atkLvl, int mny, int diff, int luck) {
        p.level = lvl; p.currentHP = hp; p.sp = sp; p.atkLevel = atkLvl; p.money = mny; p.luckLevel = luck;
        currentDifficulty = diff;
        p.baseAttack = (diff == 0 ? 40 : 25) + ((atkLvl - 1) * 15);
        state = "explore";
    }

    EXPORT void go_to_difficulty() { state = "difficulty"; }
    
    EXPORT void set_difficulty(int mode) {
        currentDifficulty = mode;
        if(mode == 0) { 
            p.currentHP = 250; p.maxHP = 250; p.baseAttack = 40; p.money = 300;
        } else { 
            p.currentHP = 150; p.maxHP = 150; p.baseAttack = 25; p.money = 50;
        }
        state = "explore";
    }

    EXPORT int get_difficulty() { return currentDifficulty; }

    EXPORT void travel_to(int id) {
        if (state != "explore" && state != "shop") return;
        p.location = id;

        if (id == 3) {
            state = "shop"; 
        } else {
            state = "explore"; 
            if ((rng() % 100) < 70) { 
                state = "combat"; 
                currentBoss = schoolWings[rng() % schoolWings.size()]; 
                m_currentHP = currentBoss.maxHP;
                load_new_question();
            }
        }
    }

    EXPORT void exit_shop() { state = "explore"; p.location = 3; }

    EXPORT void buy_item(int id) {
        if (state != "shop") return;
        if (id == 0 && p.money >= 50) { p.money -= 50; p.currentHP += 50; if(p.currentHP > p.maxHP) p.currentHP = p.maxHP; } 
        else if (id == 1 && p.money >= 100) { p.money -= 100; p.sp += 1; } 
        else if (id == 2 && p.money >= 200) { p.money -= 200; p.baseAttack += 5; p.atkLevel++; }
        else if (id == 3 && p.money >= 300) { p.money -= 300; p.luckLevel++; }
    }

    EXPORT void use_skill(int id) {
        last_dmg_to_boss = 0; last_dmg_to_player = 0; last_hit_crit = false;
        if (state != "combat") return;
        if (id == 0 && p.sp >= 2) { p.sp -= 2; p.currentHP += (p.maxHP/2); if(p.currentHP > p.maxHP) p.currentHP = p.maxHP; }
        else if (id == 1 && p.sp >= 3) {
            p.sp -= 3; int h=0;
            for(int i=0; i<4; i++) { if(curQ.options[i] != curQ.answer && !curQ.hiddenMask[i]) { curQ.hiddenMask[i]=true; h++; if(h>=2) break; } }
        }
        else if (id == 2 && p.sp >= 4) { 
            p.sp -= 4; 
            int dmg = (int)(currentBoss.maxHP * 0.2);
            m_currentHP -= dmg; 
            last_dmg_to_boss = dmg;
            load_new_question(); 
            if(m_currentHP<=0) m_currentHP=1; 
        }
    }

    EXPORT void check_answer(int idx) {
        last_dmg_to_boss = 0; last_dmg_to_player = 0; last_hit_crit = false;
        if (state != "combat") return;
        
        if (curQ.options[idx] == curQ.answer) { 
            int dmg = p.baseAttack;
            int critChance = 5 + (p.luckLevel * 5); // Base 10%
            if ((rng() % 100) < critChance) { dmg *= 2; last_hit_crit = true; }
            m_currentHP -= dmg; 
            p.money += 15; 
            last_dmg_to_boss = dmg;
        } 
        else { 
            p.currentHP -= 20; 
            last_dmg_to_player = 20;
        }
        
        if (m_currentHP <= 0) { 
            p.level++; p.money += 150;
            if (p.level == 15) { state = "combat"; currentBoss = specialBosses[0]; m_currentHP = currentBoss.maxHP; load_new_question(); return; } 
            else if (p.level == 20) { state = "combat"; currentBoss = specialBosses[1]; m_currentHP = currentBoss.maxHP; load_new_question(); return; }
            else if (p.level == 25) { state = "combat"; currentBoss = specialBosses[2]; m_currentHP = currentBoss.maxHP; load_new_question(); return; }
            else if (p.level > 25) { 
                state = "victory"; 
                final_score = (p.currentHP * 10) + (p.money * 5) + (p.level * 100);
                if(currentDifficulty == 1) final_score *= 2;
                return; 
            }
            state = "explore"; 
        } else if (p.currentHP <= 0) {
            state = "dead";
        } else if (last_dmg_to_boss > 0) { 
            load_new_question();
        }
    }

    EXPORT void roll_gacha() { if(p.sp > 0) { p.sp--; p.atkLevel++; p.baseAttack += 10; p.currentHP = p.maxHP; } }

    EXPORT const char* get_state() { return state.c_str(); }
    EXPORT const char* get_q_text() { return curQ.query.c_str(); }
    EXPORT const char* get_q_opt(int i) { return curQ.options[i].c_str(); }
    EXPORT int is_opt_hidden(int i) { return curQ.hiddenMask[i] ? 1 : 0; }
    EXPORT const char* get_boss_name() { return currentBoss.name.c_str(); }
    EXPORT const char* get_q_hint() { return curQ.hint.c_str(); }
    EXPORT int get_p_hp() { return p.currentHP; }
    EXPORT int get_p_lvl() { return p.level; }
    EXPORT int get_p_atk() { return p.atkLevel; }
    EXPORT int get_p_luck() { return p.luckLevel; }
    EXPORT int get_p_sp() { return p.sp; }
    EXPORT int get_p_money() { return p.money; }
    EXPORT int get_m_hp() { return m_currentHP; }
    EXPORT int get_m_max_hp() { return currentBoss.maxHP; }
    EXPORT int get_last_dmg_boss() { return last_dmg_to_boss; }
    EXPORT int get_last_dmg_player() { return last_dmg_to_player; }
    EXPORT int is_last_crit() { return last_hit_crit ? 1 : 0; }
    EXPORT int get_final_score() { return final_score; }
}

