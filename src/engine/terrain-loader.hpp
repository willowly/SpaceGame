#include "actor/terrain.hpp"

#include "glm/glm.hpp"

using glm::vec3;

class TerrainLoader {

    std::thread mainThread;
    std::vector<std::shared_ptr<Terrain>> terrains;
    std::mutex terrainMutex;
    std::atomic<bool> stopSignal;

    std::vector<std::thread> workerThreads;

    std::mutex cameraPositionMutex;
    vec3 cameraPosition;

    void mainTask() {
        while(!stopSignal) {
            // if(getTerrainVectorSize() == 0) {
            //     std::this_thread::sleep_for(std::chrono::milliseconds(100));
            // }
            for (int i = 0; i < getTerrainVectorSize(); i++)
            {
                auto terrain = getTerrain(i);
                auto pos = getCameraPosition();
                if(terrain != nullptr) {
                    terrain->loadNextChunk(pos);
                }
            }
        }
    }

    vec3 getCameraPosition() {
        std::scoped_lock lock(cameraPositionMutex);
        return cameraPosition;
    }

    std::shared_ptr<Terrain> getTerrain(int index) {
        std::scoped_lock lock(terrainMutex);
        assert(index >= 0);
        if(static_cast<int>(terrains.size()) <= index) {
            return nullptr;
        }
        return terrains.at(index);
    }

    int getTerrainVectorSize() {
        std::scoped_lock lock(terrainMutex);
        return static_cast<int>(terrains.size());
    }

    public:
        void addTerrain(std::shared_ptr<Terrain> terrain) {
            std::scoped_lock lock(terrainMutex);
            terrains.push_back(terrain);
        }

        void start() {

            if(workerThreads.size() > 0) {
                throw std::runtime_error("tried to start terrain loader when its already going!");
            }

            int allowedThreads = std::thread::hardware_concurrency();
            std::cout << allowedThreads << " threads are allowed" << std::endl;
            int allowedWorkerThreads = allowedThreads - 1;

            if(allowedWorkerThreads <= 0) {
                throw std::runtime_error("not enough threads " + std::to_string(allowedThreads));
            }

            if(allowedWorkerThreads > 8) {
                allowedWorkerThreads = 8;
            }

            stopSignal = false;
            for (size_t i = 0; i < allowedWorkerThreads; i++)
            {
                workerThreads.push_back(std::thread(&TerrainLoader::mainTask,this));
            }
            
            //mainThread = std::thread(&TerrainLoader::mainTask,this);
        }

        void setCameraPosition(vec3 position) {
            std::scoped_lock lock(cameraPositionMutex);
            cameraPosition = position;
        }

        void stop() {
            stopSignal = true;
            
            for (auto& workerThread : workerThreads)
            {
                workerThread.join();
            }
            workerThreads.clear();
        }

    
    
};