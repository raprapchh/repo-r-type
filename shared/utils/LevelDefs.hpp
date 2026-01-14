#pragma once
#include <vector>
#include <string>

namespace rtype::config {

struct EnemySpawn {
    std::string type;
    float x, y;
    float vx, vy;
    float spawnTime;
    float fireRate;
};

struct Wave {
    std::vector<EnemySpawn> enemies;
    float duration;
};

struct Level {
    std::vector<Wave> waves;
};

inline std::vector<Level> getLevels() {
    return {Level{{Wave{{{"Monster_0_Right", 1980.0f, 100.0f, -400.0f, 0.0f, 0.0f, 0.5f},
                         {"Monster_0_Right", 1980.0f, 300.0f, -400.0f, 0.0f, 0.5f, 0.5f},
                         {"Monster_0_Right", 1980.0f, 500.0f, -400.0f, 0.0f, 1.0f, 0.5f},
                         {"Monster_0_Right", 1980.0f, 700.0f, -400.0f, 0.0f, 1.5f, 0.5f},
                         {"Monster_0_Right", 1980.0f, 900.0f, -400.0f, 0.0f, 2.0f, 0.5f}},
                        6.0f},
                   Wave{{{"Monster_0_Top", 400.0f, -100.0f, 0.0f, 300.0f, 0.0f, 0.4f},
                         {"Monster_0_Bot", 400.0f, 1180.0f, 0.0f, -300.0f, 0.0f, 0.4f},
                         {"Monster_0_Top", 800.0f, -100.0f, 0.0f, 300.0f, 1.0f, 0.4f},
                         {"Monster_0_Bot", 800.0f, 1180.0f, 0.0f, -300.0f, 1.0f, 0.4f},
                         {"Monster_0_Top", 1200.0f, -100.0f, 0.0f, 300.0f, 2.0f, 0.4f},
                         {"Monster_0_Bot", 1200.0f, 1180.0f, 0.0f, -300.0f, 2.0f, 0.4f}},
                        6.0f}}},

            Level{{Wave{{{"Monster_0_Left", -50.0f, 100.0f, 300.0f, 0.0f, 0.0f, 0.3f},
                         {"Monster_0_Left", -50.0f, 300.0f, 300.0f, 0.0f, 0.0f, 0.3f},
                         {"Monster_0_Left", -50.0f, 600.0f, 300.0f, 0.0f, 0.0f, 0.3f},
                         {"Monster_0_Left", -50.0f, 800.0f, 300.0f, 0.0f, 0.0f, 0.3f},
                         {"Monster_0_Left", -50.0f, 900.0f, 300.0f, 0.0f, 0.0f, 0.3f},
                         {"Monster_0_Left", -50.0f, 1000.0f, 300.0f, 0.0f, 0.0f, 0.3f}},
                        8.0f},
                   Wave{{{"Monster_0_Top", 500.0f, -100.0f, 0.0f, 400.0f, 0.0f, 0.2f},
                         {"Monster_0_Bot", 700.0f, 1180.0f, 0.0f, -400.0f, 0.5f, 0.2f},
                         {"Monster_0_Left", -100.0f, 500.0f, 400.0f, 0.0f, 1.0f, 0.2f},
                         {"Monster_0_Right", 2020.0f, 300.0f, -400.0f, 0.0f, 1.5f, 0.2f},
                         {"Monster_0_Top", 1000.0f, -100.0f, 0.0f, 400.0f, 2.0f, 0.2f},
                         {"Monster_0_Bot", 1200.0f, 1180.0f, 0.0f, -400.0f, 2.5f, 0.2f}},
                        8.0f}}},

            // Level{{Wave{{{"Monster_0_Right", 1980.0f, 100.0f, -500.0f, 100.0f, 0.0f, 0.1f},
            //              {"Monster_0_Right", 1980.0f, 200.0f, -500.0f, -100.0f, 0.2f, 0.1f},
            //              {"Monster_0_Right", 1980.0f, 300.0f, -500.0f, 100.0f, 0.4f, 0.1f},
            //              {"Monster_0_Right", 1980.0f, 400.0f, -500.0f, -100.0f, 0.6f, 0.1f},
            //              {"Monster_0_Right", 1980.0f, 500.0f, -500.0f, 100.0f, 0.8f, 0.1f},
            //              {"Monster_0_Right", 1980.0f, 600.0f, -500.0f, -100.0f, 1.0f, 0.1f},
            //              {"Monster_0_Right", 1980.0f, 700.0f, -500.0f, 100.0f, 1.2f, 0.1f},
            //              {"Monster_0_Right", 1980.0f, 800.0f, -500.0f, -100.0f, 1.4f, 0.1f},
            //              {"Monster_0_Right", 1980.0f, 900.0f, -500.0f, 100.0f, 1.6f, 0.1f}},
            //             5.0f},
            //        Wave{{{"Monster_0_Left", -100.0f, 100.0f, 600.0f, 0.0f, 0.0f, 0.05f},
            //              {"Monster_0_Left", -100.0f, 300.0f, 600.0f, 0.0f, 0.2f, 0.05f},
            //              {"Monster_0_Left", -100.0f, 500.0f, 600.0f, 0.0f, 0.4f, 0.05f},
            //              {"Monster_0_Left", -100.0f, 800.0f, 600.0f, 0.0f, 0.6f, 0.05f},
            //              {"Monster_0_Left", -100.0f, 900.0f, 600.0f, 0.0f, 0.8f, 0.05f},
            //              {"Monster_0_Top", 500.0f, -100.0f, 0.0f, 600.0f, 1.0f, 0.05f},
            //              {"Monster_0_Bot", 1000.0f, 1180.0f, 0.0f, -600.0f, 1.2f, 0.05f},
            //              {"Monster_0_Right", 2020.0f, 540.0f, -600.0f, 0.0f, 1.5f, 0.05f}},
            //             5.0f}}},

            Level{{Wave{{{"Boss_1", 1500.0f, 540.0f, 0.0f, 0.0f, 0.0f, 0.6f},
                         {"Monster_0_Top", 1800.0f, 200.0f, -300.0f, 100.0f, 2.0f, 1.0f},
                         {"Monster_0_Bot", 1800.0f, 880.0f, -300.0f, -100.0f, 2.0f, 1.0f},
                         {"Monster_0_Top", 1800.0f, 200.0f, -300.0f, 100.0f, 5.0f, 1.0f},
                         {"Monster_0_Bot", 1800.0f, 880.0f, -300.0f, -100.0f, 5.0f, 1.0f},
                         {"Monster_0_Left", -100.0f, 300.0f, 400.0f, 0.0f, 8.0f, 1.0f},
                         {"Monster_0_Left", -100.0f, 700.0f, 400.0f, 0.0f, 8.0f, 1.0f},
                         {"Monster_0_Top", 1800.0f, 200.0f, -300.0f, 100.0f, 12.0f, 1.0f},
                         {"Monster_0_Bot", 1800.0f, 880.0f, -300.0f, -100.0f, 12.0f, 1.0f},
                         {"Monster_0_Left", -100.0f, 540.0f, 400.0f, 0.0f, 15.0f, 1.0f}},
                        999.0f}}},

            Level{{Wave{{{"Monster_Wave_2_Left", -100.0f, 500.0f, 200.0f, 0.0f, 0.0f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 300.0f, 200.0f, 0.0f, 0.5f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 700.0f, 200.0f, 0.0f, 1.0f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 400.0f, 200.0f, 0.0f, 1.5f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 600.0f, 200.0f, 0.0f, 2.0f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 200.0f, 200.0f, 0.0f, 2.5f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 800.0f, 200.0f, 0.0f, 3.0f, 0.5f},
                         {"Monster_Wave_2_Right", 2000.0f, 200.0f, -200.0f, 0.0f, 3.0f, 0.5f},
                         {"Monster_Wave_2_Right", 2000.0f, 400.0f, -200.0f, 0.0f, 3.5f, 0.5f},
                         {"Monster_Wave_2_Right", 2000.0f, 600.0f, -200.0f, 0.0f, 4.0f, 0.5f},
                         {"Monster_Wave_2_Right", 2000.0f, 800.0f, -200.0f, 0.0f, 4.5f, 0.5f}},
                        8.0f},
                   Wave{{{"Monster_Wave_2_Left", -100.0f, 300.0f, 200.0f, 0.0f, 0.0f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 600.0f, 200.0f, 0.0f, 0.5f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 400.0f, 200.0f, 0.0f, 1.0f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 700.0f, 200.0f, 0.0f, 1.5f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 500.0f, 200.0f, 0.0f, 2.0f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 200.0f, 200.0f, 0.0f, 2.5f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 800.0f, 200.0f, 0.0f, 3.0f, 0.5f},
                         {"Monster_Wave_2_Right", 2000.0f, 300.0f, -200.0f, 0.0f, 0.0f, 0.5f},
                         {"Monster_Wave_2_Right", 2000.0f, 700.0f, -200.0f, 0.0f, 0.0f, 0.5f},
                         {"Monster_Wave_2_Right", 2000.0f, 500.0f, -200.0f, 0.0f, 2.0f, 0.5f}},
                        6.0f},
                   Wave{{{"Monster_Wave_2_Left", -100.0f, 200.0f, 200.0f, 0.0f, 0.0f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 800.0f, 200.0f, 0.0f, 0.5f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 400.0f, 200.0f, 0.0f, 1.0f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 600.0f, 200.0f, 0.0f, 1.5f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 300.0f, 200.0f, 0.0f, 2.0f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 700.0f, 200.0f, 0.0f, 2.5f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 500.0f, 200.0f, 0.0f, 3.0f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 200.0f, 200.0f, 0.0f, 3.5f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 800.0f, 200.0f, 0.0f, 4.0f, 0.5f},
                         {"Monster_Wave_2_Right", 2000.0f, 100.0f, -200.0f, 0.0f, 0.0f, 0.5f},
                         {"Monster_Wave_2_Right", 2000.0f, 200.0f, -200.0f, 0.0f, 0.5f, 0.5f},
                         {"Monster_Wave_2_Right", 2000.0f, 300.0f, -200.0f, 0.0f, 1.0f, 0.5f},
                         {"Monster_Wave_2_Right", 2000.0f, 400.0f, -200.0f, 0.0f, 1.5f, 0.5f},
                         {"Monster_Wave_2_Right", 2000.0f, 500.0f, -200.0f, 0.0f, 2.0f, 0.5f},
                         {"Monster_Wave_2_Right", 2000.0f, 600.0f, -200.0f, 0.0f, 2.5f, 0.5f},
                         {"Monster_Wave_2_Right", 2000.0f, 700.0f, -200.0f, 0.0f, 3.0f, 0.5f},
                         {"Monster_Wave_2_Right", 2000.0f, 800.0f, -200.0f, 0.0f, 3.5f, 0.5f}},
                        10.0f},
                   Wave{{{"Monster_Wave_2_Left", -100.0f, 400.0f, 200.0f, 0.0f, 0.0f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 600.0f, 200.0f, 0.0f, 0.5f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 200.0f, 200.0f, 0.0f, 1.0f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 800.0f, 200.0f, 0.0f, 1.5f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 300.0f, 200.0f, 0.0f, 2.0f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 700.0f, 200.0f, 0.0f, 2.5f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 500.0f, 200.0f, 0.0f, 3.0f, 0.5f},
                         {"Monster_Wave_2_Right", 2000.0f, 400.0f, -200.0f, 0.0f, 1.0f, 0.5f},
                         {"Monster_Wave_2_Right", 2000.0f, 600.0f, -200.0f, 0.0f, 1.5f, 0.5f}},
                        5.0f}}},

            Level{{Wave{{{"Boss_2", 1700.0f, 300.0f, 0.0f, 0.0f, 0.0f, 0.05f},
                         {"Monster_Wave_2_Left", -100.0f, 200.0f, 200.0f, 0.0f, 5.0f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 800.0f, 200.0f, 0.0f, 10.0f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 400.0f, 200.0f, 0.0f, 15.0f, 0.5f},
                         {"Monster_Wave_2_Left", -100.0f, 600.0f, 200.0f, 0.0f, 20.0f, 0.5f}},
                        999.0f}}}};
}

} // namespace rtype::config
